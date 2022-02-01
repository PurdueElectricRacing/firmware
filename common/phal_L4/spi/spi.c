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
#include "common_defs.h"

extern uint32_t APB2ClockRateHz;

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
    SPI1->CR2 |= CLAMP(cfg->data_len, 4, 12) - 4;

    // Data Rate
    // Divisor is a power of 2, find the closest power of 2 limited to log2(256)
    uint32_t f_div = LOG2_DOWN(APB2ClockRateHz / cfg->data_rate);
    f_div = CLAMP(f_div, 0, 0b111);
    SPI1->CR1 &= ~SPI_CR1_BR_Msk;
    SPI1->CR1 |= f_div << SPI_CR1_BR_Pos;

    // NSS control
    if (cfg->nss_sw)
        SPI1->CR1 |= SPI_CR1_SSM;
    
    // Enable DMA
    SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;
    
    // Enable SPI peripheral
    SPI1->CR1 |= SPI_CR1_SPE;
}

bool PHAL_SPI_transmit(const uint32_t* data, const uint32_t length)
{
    for(uint32_t i = 0; i < length; i++)
    {
        while ((SPI1->SR & SPI_SR_TXE) == 0)
		    ;
        SPI1->DR = data[i];
    }
}

bool PHAL_SPI_recieve(const uint32_t* data, const uint32_t length)
{
    
}
