/**
 * @file dma_priv.h
 * @brief G4 DMA Peripheral private/register level implementation 
 * @author Shriya Balu (balu@purdue.edu) 
 */

#ifndef PHAL_G4_DMA_PRIV_H
#define PHAL_G4_DMA_PRIV_H

#include "common/phal_G4/dma/dma.h"

/// Enable the clock for the selected DMA peripheral
void PHAL_DMA_priv_enableClock(dma_init_t *dma);

/// Disable a DMA channel and wait until it is fully disabled
void PHAL_DMA_priv_disableStream(DMA_Channel_TypeDef *channel);

/// Clear all pending status flags for a DMA channel
void PHAL_DMA_priv_clearFlags(DMA_TypeDef *dma_periph, uint8_t channel_idx);

/// Configure the DMA channel control register from the initialization structure
void PHAL_DMA_priv_configParams(dma_init_t *dma);

/// Get the DMA channel instance corresponding to a DMA peripheral and channel index
void PHAL_DMA_priv_setChannel(DMA_TypeDef *periph, DMA_Channel_TypeDef **channel, uint8_t channel_idx);

/// Enable a DMA channel
void PHAL_DMA_priv_enableStream(DMA_Channel_TypeDef *channel);

/// Configure the DMAMUX request for a DMA channel
void PHAL_DMA_priv_configMUX(dma_init_t *dma);

/// Write the peripheral address to the DMA channel
void PHAL_DMA_priv_setPeriphAddress(dma_init_t *dma);

/// Convert a DMA mode into the corresponding CCR register bits
uint32_t PHAL_DMA_priv_modeBits(dma_mode_t mode);

/// Write the DMA transfer length to the channel
void PHAL_DMA_priv_writeTxferLength(dma_init_t *dma, const uint32_t length);

/// Write the memory address to the DMA channel
void PHAL_DMA_priv_writeMemAddress(dma_init_t *dma, const uint32_t address);

#endif // PHAL_G4_DMA_PRIV_H