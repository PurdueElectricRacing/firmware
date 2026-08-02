/**
 * @file dma_priv.c
 * @brief G4 DMA Peripheral private/register level implementation 
 * @author Shriya Balu (balu@purdue.edu) 
 * 
 */

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/dma/dma_priv.h"

void PHAL_DMA_priv_enableClock(dma_init_t *dma) {
    // DMAMUX1 clock is required to route peripheral requests to DMA channels
    RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;

    if (dma->periph == DMA1) {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    } else if (dma->periph == DMA2) {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    }
}

void PHAL_DMA_priv_disableStream(DMA_Channel_TypeDef *channel) {
    channel->CCR &= ~(DMA_CCR_EN);
    while (channel->CCR & DMA_CCR_EN) {
    __asm__("nop");
    }
}

void PHAL_DMA_priv_setChannel(DMA_TypeDef *periph, DMA_Channel_TypeDef **channel, uint8_t channel_idx) {
    // TODO less magic numbers, read why offsetes were like this
    if (periph == DMA1) {
        *channel = (DMA_Channel_TypeDef *)((uint32_t)DMA1_Channel1 + 0x14U * (channel_idx - 1U));
    } else if (periph == DMA2) {
        *channel = (DMA_Channel_TypeDef *)((uint32_t)DMA2_Channel1 + 0x14U * (channel_idx - 1U));
    }
}


void PHAL_DMA_priv_clearFlags(DMA_TypeDef *dma_periph, uint8_t channel_idx) {
    // Writing 1 to the bits in the IFCR register clears the corresponding bits in the ISR register
    dma_periph->IFCR = (DMA_ISR_GIF1 | DMA_ISR_TCIF1 | DMA_ISR_HTIF1 | DMA_ISR_TEIF1)
        << (4 * (channel_idx - 1));
}

void PHAL_DMA_priv_configParams(dma_init_t *dma) {
    dma->channel->CCR = 
          ((dma->mem_size << DMA_CCR_MSIZE_Pos) & DMA_CCR_MSIZE_Msk)        // memory data size
        | ((dma->periph_size << DMA_CCR_PSIZE_Pos) & DMA_CCR_PSIZE_Msk)     // peripheral data size
        | ((dma->priority << DMA_CCR_PL_Pos) & DMA_CCR_PL_Msk)              // channel priority
        | ((dma->mem_inc << DMA_CCR_MINC_Pos) & DMA_CCR_MINC_Msk)           // memory increment mode
        | ((dma->periph_inc << DMA_CCR_PINC_Pos) & DMA_CCR_PINC_Msk)        // peripheral increment mode
        | ((dma->dir << DMA_CCR_DIR_Pos) & DMA_CCR_DIR_Msk)                 // data transfer direction
        | ((dma->tx_isr_en << DMA_CCR_TEIE_Pos) & DMA_CCR_TEIE_Msk)         // transfer error interrupt enable
        | ((dma->tx_isr_en << DMA_CCR_TCIE_Pos) & DMA_CCR_TCIE_Msk)         // transfer complete interrupt enable
        | PHAL_DMA_priv_modeBits(dma->mode); // normal, circular or memory-to-memory mode

}

void PHAL_DMA_priv_enableStream(DMA_Channel_TypeDef *channel) {
    // Enable the DMA channel by setting the EN bit in the DMA_CCRx register
    channel->CCR |= DMA_CCR_EN;
}

void PHAL_DMA_priv_configMUX(dma_init_t *dma) {
    // DMAMUX mapping (See RM0440 13.3.2 DMAMUX mapping section)
    
    // Calculate mux index
    uint8_t mux_idx = (dma->periph == DMA2) ? (dma->channel_idx + 7) : (dma->channel_idx - 1);

    // Determine corresponding DMAMUX channel
    DMAMUX_Channel_TypeDef* mux = (DMAMUX1_Channel0 + mux_idx);

    // Set the DMA request ID for the DMAMUX channel
    mux->CCR = (mux->CCR & ~DMAMUX_CxCR_DMAREQ_ID_Msk) | dma->mux_request;   
}

void PHAL_DMA_priv_setPeriphAddress(dma_init_t *dma) {
    dma->channel->CPAR = dma->periph_addr;
}

uint32_t PHAL_DMA_priv_modeBits(dma_mode_t mode)
{
    switch (mode) {
        case DMA_MODE_NORMAL:   return 0;
        case DMA_MODE_CIRCULAR: return DMA_CCR_CIRC;
        case DMA_MODE_MEM2MEM:  return DMA_CCR_MEM2MEM;
        default:
            __builtin_trap();
    }
}

void PHAL_DMA_priv_writeTxferLength(dma_init_t *dma, const uint32_t length) {
    dma->channel->CNDTR = length;
}

void PHAL_DMA_priv_writeMemAddress(dma_init_t *dma, const uint32_t address) {
    dma->channel->CMAR = address;
}