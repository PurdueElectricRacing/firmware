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

extern uint32_t APB2ClockRateHz;
extern uint32_t APB1ClockRateHz;
static volatile SPI_InitConfig_t *active_transfer = NULL;

static uint16_t trash_can; // Used as an address for DMA to dump data into
static uint16_t zero;      // Used as a constant zero during transmissions

bool PHAL_SPI_init(SPI_InitConfig_t *cfg)
{
    zero = 0;
    if (cfg->periph == SPI1)
    {
        // Enable RCC Clock
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    }
#ifdef SPI2
    else if (cfg->periph == SPI2)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
    }
#endif
    else if (cfg->periph == SPI3)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;
    }
    else
    {
        return false;
    }

    // Setup for Master, positive polarity
    cfg->periph->CR1 |= SPI_CR1_MSTR | SPI_CR1_SPE | SPI_CR1_SSM | SPI_CR1_SSI;
    cfg->periph->CR1 &= ~(SPI_CR1_CPOL);
    cfg->periph->CR2 &= ~(SPI_CR2_NSSP | SPI_CR2_SSOE);

    // Data Size
    cfg->periph->CR2 &= ~(SPI_CR2_DS_Msk);
    cfg->periph->CR2 |= (CLAMP(cfg->data_len, 4, 16) - 1) << SPI_CR2_DS_Pos;

    // RX Fifo full on 8 bits
    cfg->periph->CR2 |= SPI_CR2_FRXTH;
    // SPI1->CR1 |= SPI_CR1_LSBFIRST;

    // Data Rate
    // Divisor is a power of 2, find the closest power of 2 limited to log2(256)
    uint32_t f_div = 0;
    if (cfg->periph == SPI1)
        f_div = LOG2_DOWN(APB2ClockRateHz / cfg->data_rate) - 1;
    // Both SPI1 and SPI2 are on APB1
    else
        f_div = LOG2_DOWN(APB1ClockRateHz / cfg->data_rate) - 1;
    f_div = CLAMP(f_div, 0, 0b111);
    cfg->periph->CR1 &= ~SPI_CR1_BR_Msk;
    cfg->periph->CR1 |= f_div << SPI_CR1_BR_Pos;

    // Setup DMA channels
    if (cfg->rx_dma_cfg && !PHAL_initDMA(cfg->rx_dma_cfg))
        return false;

    if (cfg->tx_dma_cfg && !PHAL_initDMA(cfg->tx_dma_cfg))
        return false;

    PHAL_writeGPIO(cfg->nss_gpio_port, cfg->nss_gpio_pin, 1);

    cfg->_busy = false;
    cfg->_error = false;
}

bool PHAL_SPI_transfer_noDMA(SPI_InitConfig_t *spi, const uint8_t *out_data, uint32_t txlen, uint32_t rxlen, uint8_t *in_data)
{
    // SPI1->CR1 &= ~(1 << 11);
    in_data += txlen;
    if (PHAL_SPI_busy(spi))
        return false;
    active_transfer = spi;
    spi->_busy = true;
    // Enable SPI
    spi->periph->CR1 |= SPI_CR1_SPE;
    // Select peripheral
    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);
    // Add messages to TX FIFO
    for (uint8_t i = 0; i < txlen; i++)
    {
        while (!(spi->periph->SR & SPI_SR_TXE))
            ;
        if (i + 1 < txlen)
        {
            uint16_t data = out_data[i + 1] << 8 | (uint16_t)out_data[i];
            spi->periph->DR = data;
            i++;
        }
        else
        {
            spi->periph->DR = out_data[i];
        }
    }
    // Wait till TX FIFO is empty and spi is not active
    while (!(spi->periph->SR & SPI_SR_TXE) || (spi->periph->SR & SPI_SR_BSY))
        ;
    // Clear overrun
    uint8_t trash = spi->periph->DR;
    trash = spi->periph->SR;

    // RX
    for (uint8_t i = 0; i < rxlen; i++)
    {
        // Wait till SPI is not in active transaction, send dummy
        while ((spi->periph->SR & SPI_SR_BSY))
            ;
        spi->periph->DR = 0;
        // With for RX FIFO to recieve a message, read it in
        while (!(spi->periph->SR & SPI_SR_RXNE))
            ;
        in_data[i] = (uint8_t)(spi->periph->DR);
    }
    // Stop message
    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 1);
    // Wait till transaction is finished, disable spi, and clear the queue
    while ((spi->periph->SR & SPI_SR_BSY))
        ;
    spi->periph->CR1 &= ~SPI_CR1_SPE;
    while ((spi->periph->SR & SPI_SR_FRLVL))
    {
        trash = spi->periph->DR;
    }
    spi->_busy = false;
    return true;
}

bool PHAL_SPI_transfer(SPI_InitConfig_t *spi, const uint8_t *out_data, const uint32_t data_len, const uint8_t *in_data)
{
    /*
    Each DMA channel is enabled if a data buffer is provided.
    Only the Tx DMA channel has transfer complete interrupts enabled, as the same data length is used for both channels
    */

    if (PHAL_SPI_busy(spi))
        return false;

    active_transfer = spi;

    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);

    spi->_busy = true;

    spi->periph->CR2 |= SPI_CR2_TXDMAEN;
    if (!out_data)
    {
        spi->tx_dma_cfg->channel->CCR &= ~DMA_CCR_MINC;
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)&zero);
    }
    else
    {
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)out_data);
    }
    PHAL_DMA_setTxferLength(spi->tx_dma_cfg, data_len);

    spi->periph->CR2 |= SPI_CR2_RXDMAEN;
    if (!in_data)
    {
        spi->rx_dma_cfg->channel->CCR &= ~DMA_CCR_MINC;
        PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)&trash_can);
    }
    else
    {
        PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)in_data);
    }
    PHAL_DMA_setTxferLength(spi->rx_dma_cfg, data_len);

    PHAL_startTxfer(spi->rx_dma_cfg);

    // Enable the DMA IRQ
    if (spi->periph == SPI1)
    {
        NVIC_EnableIRQ(DMA1_Channel2_IRQn);
        NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    }
#ifdef SPI2
    else if (spi->periph == SPI2)
    {
        NVIC_EnableIRQ(DMA1_Channel4_IRQn);
        NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    }
#endif
    else if (spi->periph == SPI3)
    {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn);
        NVIC_EnableIRQ(DMA2_Channel2_IRQn);
    }

    // Start transaction
    spi->periph->CR1 |= SPI_CR1_SPE;

    // STM32 HAL Libraries start TX Dma transaction last
    PHAL_startTxfer(spi->tx_dma_cfg);

    return true;
}

bool PHAL_SPI_busy(SPI_InitConfig_t *cfg)
{
    // Latch in case active_transfer cleared during interrupt
    volatile SPI_InitConfig_t *act = active_transfer;
    if (act && cfg->periph == act->periph)
        return act->_busy;

    return false;
}

void DMA1_Channel3_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF3)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF3;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA1->ISR & DMA_ISR_TCIF3)
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF3;
    }
    if (DMA1->ISR & DMA_ISR_GIF3)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF3;
    }
}

void DMA2_Channel2_IRQHandler()
{
    if (DMA2->ISR & DMA_ISR_TEIF2)
    {
        DMA2->IFCR |= DMA_IFCR_CTEIF2;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA2->ISR & DMA_ISR_TCIF2)
    {
        DMA2->IFCR |= DMA_IFCR_CTCIF2;
    }
    if (DMA2->ISR & DMA_ISR_GIF2)
    {
        DMA2->IFCR |= DMA_IFCR_CGIF2;
    }
}

#ifdef SPI2
void DMA1_Channel5_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF5)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF5;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA1->ISR & DMA_ISR_TCIF5)
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF5;
    }
    if (DMA1->ISR & DMA_ISR_GIF5)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF5;
    }
}
#endif

/**
 * @brief SPI1 Rx DMA Transfer Complete interrupt
 *
 */
void DMA1_Channel2_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF2)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF2;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA1->ISR & DMA_ISR_TCIF2)
    {
        if (active_transfer->nss_sw)
            PHAL_writeGPIO(active_transfer->nss_gpio_port, active_transfer->nss_gpio_pin, 1);

        // Disable DMA channels
        PHAL_stopTxfer(active_transfer->rx_dma_cfg);
        PHAL_stopTxfer(active_transfer->tx_dma_cfg);

        // Revert to mem_inc if trash_can used
        if (active_transfer->rx_dma_cfg->mem_inc)
            active_transfer->rx_dma_cfg->channel->CCR |= DMA_CCR_MINC;
        if (active_transfer->tx_dma_cfg->mem_inc)
            active_transfer->tx_dma_cfg->channel->CCR |= DMA_CCR_MINC;

        // Disable SPI peripheral and DMA requests
        SPI1->CR1 &= ~SPI_CR1_SPE;
        SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        active_transfer->_busy = false;
        active_transfer->_error = false;
        active_transfer = NULL;
    }
    if (DMA1->ISR & DMA_ISR_GIF2)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF2;
    }
}
void DMA2_Channel1_IRQHandler()
{
    if (DMA2->ISR & DMA_ISR_TEIF1)
    {
        DMA2->IFCR |= DMA_IFCR_CTEIF1;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA2->ISR & DMA_ISR_TCIF1)
    {
        if (active_transfer->nss_sw)
            PHAL_writeGPIO(active_transfer->nss_gpio_port, active_transfer->nss_gpio_pin, 1);

        // Disable DMA channels
        PHAL_stopTxfer(active_transfer->rx_dma_cfg);
        PHAL_stopTxfer(active_transfer->tx_dma_cfg);

        // Revert to mem_inc if trash_can used
        if (active_transfer->rx_dma_cfg->mem_inc)
            active_transfer->rx_dma_cfg->channel->CCR |= DMA_CCR_MINC;
        if (active_transfer->tx_dma_cfg->mem_inc)
            active_transfer->tx_dma_cfg->channel->CCR |= DMA_CCR_MINC;

        // Disable SPI peripheral and DMA requests
        SPI3->CR1 &= ~SPI_CR1_SPE;
        SPI3->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        active_transfer->_busy = false;
        active_transfer->_error = false;
        active_transfer = NULL;
    }
    if (DMA2->ISR & DMA_ISR_GIF1)
    {
        DMA2->IFCR |= DMA_IFCR_CGIF1;
    }
}
/**
 * @brief SPI1 Rx DMA Transfer Complete interrupt
 *
 */
#ifdef SPI2
void DMA1_Channel4_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF4)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF4;
        if (active_transfer)
            active_transfer->_error = true;
    }
    if (DMA1->ISR & DMA_ISR_TCIF4)
    {
        if (active_transfer->nss_sw)
            PHAL_writeGPIO(active_transfer->nss_gpio_port, active_transfer->nss_gpio_pin, 1);

        // Disable DMA channels
        PHAL_stopTxfer(active_transfer->rx_dma_cfg);
        PHAL_stopTxfer(active_transfer->tx_dma_cfg);

        // Revert to mem_inc if trash_can used
        if (active_transfer->rx_dma_cfg->mem_inc)
            active_transfer->rx_dma_cfg->channel->CCR |= DMA_CCR_MINC;
        if (active_transfer->tx_dma_cfg->mem_inc)
            active_transfer->tx_dma_cfg->channel->CCR |= DMA_CCR_MINC;

        // Disable SPI peripheral and DMA requests
        SPI2->CR1 &= ~SPI_CR1_SPE;
        SPI2->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        active_transfer->_busy = false;
        active_transfer->_error = false;
        active_transfer = NULL;
    }
    if (DMA1->ISR & DMA_ISR_GIF4)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF4;
    }
}
#endif

uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy)
{
    static uint8_t tx_cmd[4] = {(1 << 7), 0, 0};
    static uint8_t rx_dat[4] = {1, 1, 1, 1};
    tx_cmd[0] |= (address & 0x7F);

    while (PHAL_SPI_busy(spi))
        ;
    if (spi->rx_dma_cfg != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, skipDummy ? 2 : 3, rx_dat);
    else
        PHAL_SPI_transfer_noDMA(spi, tx_cmd, 1, skipDummy ? 1 : 2, rx_dat);
    while (PHAL_SPI_busy(spi))
        ;

    return skipDummy ? rx_dat[1] : rx_dat[2];
}

uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat)
{
    uint8_t tx_cmd[3] = {0};
    uint8_t rx_dat[3] = {0};
    tx_cmd[0] |= (address & 0x7F);
    tx_cmd[1] |= (writeDat);

    while (PHAL_SPI_busy(spi))
        ;
    if (spi->tx_dma_cfg != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, 2, rx_dat);
    else
        PHAL_SPI_transfer_noDMA(spi, tx_cmd, 2, 0, rx_dat);
    while (PHAL_SPI_busy(spi))
        ;

    return rx_dat[1];
}