/**
 * @file spi.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief PER Serial Peripheral Interface Device driver for STM32L4
 * @version 0.1
 * @date 2022-01-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "common/phal_L4/spi/spi.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/dma/dma.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common_defs.h"
#include <stddef.h>

extern uint32_t APB2ClockRateHz;
static SPI_InitConfig_t* active_transfer = NULL;

bool PHAL_SPI_init(const SPI_InitConfig_t* cfg)
{
    // Enable RCC Clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Setup for Master, Bidirectional, positive polarity
    SPI1->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
    SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    SPI1->CR2 |= SPI_CR2_NSSP | SPI_CR2_SSOE;

    // Data Size
    SPI1->CR2 &= ~(SPI_CR2_DS_Msk);
    SPI1->CR2 |= (CLAMP(cfg->data_len, 4, 16) - 1) << SPI_CR2_DS_Pos;

    // Data Rate
    // Divisor is a power of 2, find the closest power of 2 limited to log2(256)
    uint32_t f_div = LOG2_DOWN(APB2ClockRateHz / cfg->data_rate);
    f_div = CLAMP(f_div, 0, 0b111);
    SPI1->CR1 &= ~SPI_CR1_BR_Msk;
    SPI1->CR1 |= f_div << SPI_CR1_BR_Pos;

    // NSS control
    if (cfg->nss_sw)
        SPI1->CR1 |= SPI_CR1_SSM;

    // Setup DMA channels
    if (!PHAL_initDMA(cfg->rx_dma_cfg))
        return false;


    if (!PHAL_initDMA(cfg->tx_dma_cfg))
        return false;
}

bool PHAL_SPI_transfer(SPI_InitConfig_t* spi, const uint8_t* out_data, const uint32_t data_len, const uint8_t* in_data)
{
    /*
    Each DMA channel is enabled if a data buffer is provided.

    Only the Tx DMA channel has transfer complete interrupts enabled, as the same data length is used for both channels
    */

    if (spi->_busy)
        return false;

    spi->_busy = true;
    active_transfer = spi;

    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    PHAL_DMA_setTxferLength(spi->tx_dma_cfg, data_len);
    PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t) out_data);

    SPI1->CR2 |= SPI_CR2_RXDMAEN;
    PHAL_DMA_setTxferLength(spi->rx_dma_cfg, data_len);
    PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t) in_data);

    // Enable the DMA IRQ
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    PHAL_startTxfer(active_transfer->rx_dma_cfg);
    PHAL_startTxfer(active_transfer->tx_dma_cfg);

    // Start transaction
    if(active_transfer->nss_sw)
        PHAL_writeGPIO(active_transfer->nss_gpio_bank, active_transfer->nss_gpio_pin, 0);
    SPI1->CR1 |= SPI_CR1_SPE;

    return true;
}

bool PHAL_SPI_busy()
{
    if (active_transfer)
        return active_transfer->_busy;
    return false;
}


/**
 * @brief SPI1 Tx DMA Transfer Complete interrupt
 * 
 */
void DMA1_Channel3_IRQHandler()
{
    bool is_tc_interrupt = DMA1->ISR & DMA_ISR_TCIF3;
    // Ack interrupt
    DMA1->IFCR |= DMA_IFCR_CTCIF3_Msk;

    if (active_transfer->nss_sw)
        PHAL_writeGPIO(active_transfer->nss_gpio_bank, active_transfer->nss_gpio_pin, 1);

    // Disable DMA channels
    PHAL_stopTxfer(active_transfer->rx_dma_cfg);
    PHAL_stopTxfer(active_transfer->tx_dma_cfg);

    // Disable SPI peripheral and DMA requests
    SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    SPI1->CR1 &= ~SPI_CR1_SPE;

    active_transfer->_busy = false;
    active_transfer = NULL;
}
