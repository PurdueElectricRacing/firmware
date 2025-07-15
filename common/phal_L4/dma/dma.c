/**
 * @file dma.c
 * @author Dawson Moore (moore800@purdue.edu)
 * @brief Basic DMA Peripheral HAL library for setting up DMA transfers
 * @version 0.1
 * @date 2022-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "common/phal_L4/dma/dma.h"

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

    // Channel configuration set
    init->channel->CCR &= ~(DMA_CCR_EN_Msk);
    init->channel->CCR &= ~(0x3FFF);

    init->channel->CCR |= (init->mem_to_mem << 14) | (init->priority << 12) | (init->mem_size << 10);
    init->channel->CCR |= (init->periph_size << 8) | (init->mem_inc << 7) | (init->periph_inc << 6);
    init->channel->CCR |= (init->circular << 5) | (init->dir << 4) | (1U << 3) | (init->tx_isr_en << 1);

    // Channel memory configuration set
    PHAL_DMA_setTxferLength(init, init->tx_size);

    init->channel->CPAR = init->periph_addr;
    init->channel->CMAR = init->mem_addr;

    init->request->CSELR &= ~((0xf) << ((init->channel_idx - 1) * 4));
    init->request->CSELR |= (init->dma_chan_request << ((init->channel_idx - 1) * 4));

    return true;
}

void PHAL_startTxfer(dma_init_t* init) {
    // Channel enable starts txfer
    init->channel->CCR |= 1U;
}

void PHAL_stopTxfer(dma_init_t* init) {
    // Channel disable stops txfer
    init->channel->CCR &= ~1U;
}

void PHAL_reEnable(dma_init_t* init) {
    init->periph->IFCR |= (0xf << (init->channel_idx - 1) * 4);

    init->channel->CCR |= 1U;
}

void PHAL_DMA_setMemAddress(dma_init_t* init, const uint32_t address) {
    init->channel->CMAR = address;
}

void PHAL_DMA_setTxferLength(dma_init_t* init, const uint32_t length) {
    init->channel->CNDTR &= ~(DMA_CNDTR_NDT_Msk);
    init->channel->CNDTR |= length;
}