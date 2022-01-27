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
#include "common_defs.h"

bool PHAL_SPI_init(const SPI_Handle_t* handle)
{
    // Enable RCC Clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Setup for Master, Bidirectional, positive polarity
    SPI1->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
    SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    SPI1->CR2 |= SPI_CR2_NSSP | SPI_CR2_SSOE;

    SPI1->CR2 &= ~(SPI_CR2_DS_Msk);
    SPI1->CR2 |= CLAMP(handle->data_len, 4, 12) - 4;
    
    // Enable SPI peripheral
    SPI1->CR1 |= SPI_CR1_SPE;

}

bool PHAL_SPI_read()
{

}
