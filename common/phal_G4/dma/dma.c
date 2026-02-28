/**
 * @file dma.c
 * @author Eileen Yoon
 * @brief Basic DMA Peripheral HAL library
 * @version 0.1
 * @date 2023-08-19
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "common/phal_G4/dma/dma.h"

bool PHAL_initDMA(dma_init_t* dma) {
    // Check we aren't going to break the peripheral
    if (dma->mem_to_mem && dma->circular) {
        return false;
    } else if (dma->dir > 1) {
        return false;
    } else if (dma->priority > 3) {
        return false;
    } else if (dma->mem_size > 2 || dma->periph_size > 2) {
        return false;
    }

    // Enable clock in RCC
    if (dma->periph == DMA1) {
        dma->channel = (DMA_Channel_TypeDef *)((uint32_t)DMA1_Channel1 + 0x14U * (dma->channel_idx - 1U));
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;
    } else if (dma->periph == DMA2) {
        dma->channel = (DMA_Channel_TypeDef *)((uint32_t)DMA2_Channel1 + 0x14U * (dma->channel_idx - 1U));
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN | RCC_AHB1ENR_DMAMUX1EN;
    } else {
        return false;
    }

    // Ensure the stream is disabled, must be in order to configure the DMA control registers
    dma->channel->CCR &= ~(DMA_CCR_EN);
    while (dma->channel->CCR & DMA_CCR_EN)
        ;

    // Clear any stream dedicated status flags that may have been set previously
    dma->periph->IFCR = (DMA_ISR_GIF1 | DMA_ISR_TCIF1 | DMA_ISR_HTIF1 | DMA_ISR_TEIF1)
        << (4 * (dma->channel_idx - 1));

    // Set peripheral address (src)
    dma->channel->CPAR = dma->periph_addr;
    // Set memory address (dst)
    dma->channel->CMAR  = dma->mem_addr;
    dma->channel->CNDTR = dma->tx_size;

    // Reset preconfigured CR values
    dma->channel->CCR = 0;
    // Set channel, priority, memory data size
    dma->channel->CCR |= ((dma->mem_size << DMA_CCR_MSIZE_Pos) & DMA_CCR_MSIZE_Msk)
        | ((dma->periph_size << DMA_CCR_PSIZE_Pos) & DMA_CCR_PSIZE_Msk)
        | ((dma->priority << DMA_CCR_PL_Pos) & DMA_CCR_PL_Msk)
        | ((dma->mem_inc << DMA_CCR_MINC_Pos) & DMA_CCR_MINC_Msk)
        | ((dma->periph_inc << DMA_CCR_PINC_Pos) & DMA_CCR_PINC_Msk)
        | ((dma->circular << DMA_CCR_CIRC_Pos) & DMA_CCR_CIRC_Msk)
        | ((dma->dir << DMA_CCR_DIR_Pos) & DMA_CCR_DIR_Msk)
        | ((dma->tx_isr_en << DMA_CCR_TEIE_Pos) & DMA_CCR_TEIE_Msk)
        | ((dma->tx_isr_en << DMA_CCR_TCIE_Pos) & DMA_CCR_TCIE_Msk)
        | ((dma->mem_to_mem << DMA_CCR_MEM2MEM_Pos) & DMA_CCR_MEM2MEM_Msk);

    /* DMA Mux */
    // For category 3 and category 4 devices:
    // DMAMUX channels 0 to 7 are connected to DMA1 channels 1 to 8
    // DMAMUX channels 8 to 15 are connected to DMA2 channels 1 to 8
    // DMAMUX Channel (usually equal to DMA channel number - 1)
    DMAMUX_Channel_TypeDef* mux;
    mux      = (DMAMUX1_Channel0 + dma->channel_idx - 1);
    mux->CCR = (mux->CCR & ~0x7F) | dma->mux_request;

    return true;
}

void PHAL_startTxfer(dma_init_t* dma) {
    // Stream enable starts txfer
    dma->channel->CCR |= DMA_CCR_EN;
}

void PHAL_stopTxfer(dma_init_t* dma) {
    // Stream disable stops txfer
    dma->channel->CCR &= ~DMA_CCR_EN;
}

void PHAL_reEnable(dma_init_t* dma) {
    // Clear any stream dedicated status flags that may have been set previously
    dma->periph->IFCR = (DMA_ISR_GIF1 | DMA_ISR_TCIF1 | DMA_ISR_HTIF1 | DMA_ISR_TEIF1)
        << (4 * (dma->channel_idx - 1));
    dma->channel->CCR |= DMA_CCR_EN;
}

void PHAL_DMA_setMemAddress(dma_init_t* dma, const uint32_t address) {
    dma->channel->CMAR = address;
}

void PHAL_DMA_setTxferLength(dma_init_t* dma, const uint32_t length) {
    dma->channel->CNDTR = length; // Set number of data to transfer
}
