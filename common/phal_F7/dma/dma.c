/**
 * @file dma.c
 * @author Chris McGalliard - Port of L4 HAL by Dawson Moore (moore800@purdue.edu)
 * @brief Basic DMA Peripheral HAL library for setting up DMA transfers
 * @version 0.1
 * @date 2023-08-19
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "common/phal_F7/dma/dma.h"

bool PHAL_initDMA(dma_init_t* init) {
    // Check we aren't going to break the peripheral
    if (init->mem_to_mem && init->circular) {
        return false;
    } else if (init->dir > 1) {
        return false;
    } else if (init->priority > 3) {
        return false;
    } else if (init->mem_size > 2 || init->periph_size > 2) {
        return false;
    }

    // Enable clock in RCC
    if (init->periph == DMA1) {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    } else if (init->periph == DMA2) {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    } else {
        return false;
    }

    // Ensure the stream is disabled, must be in order to configure the DMA control registers
    init->stream->CR &= ~(DMA_SxCR_EN);
    while(init->stream->CR &= DMA_SxCR_EN)
        ;

    // Set channel, priority, memory data size
    init->stream->CR |= (init->dma_chan_request << 25) | (init->priority << 16) | (init->mem_size << 13);

    // Set peripheral data size, memory increment, peripheral increment
    init->stream->CR |= (init->periph_size << 11) | (init->mem_inc << 10) | (init->periph_inc << 9);

    // Set circular mode, direction, transfer error interrupt enable, and transfer complete interrupt enable
    init->stream->CR |= (init->circular << 8) | (init->dir << 6) | (init->tx_isr_en << 4) | (1U << 2);

    // Set peripheral port register address
    init->stream->PAR = init->periph_addr;

    // Set memory address
    init->stream->M0AR = init->mem_addr;

    // Ensure status bits are cleared
    init->periph->LISR = 0;
    init->periph->HISR = 0;

    // Set stream memory configuration
    PHAL_DMA_setTxferLength(init, init->tx_size);

    // Active the stream
    init->stream->CR &= (DMA_SxCR_EN);
    return true;
}


void PHAL_DMA_setTxferLength(dma_init_t* init, const uint32_t length)
{
    init->stream->NDTR &= ~(DMA_SxNDT);                            // Clear current tx length
    init->stream->NDTR |= length;                                  // Set number of data to transfer
}