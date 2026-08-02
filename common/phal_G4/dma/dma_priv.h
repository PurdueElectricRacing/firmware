/**
 * @file dma_priv.h
 * @brief G4 DMA Peripheral private/register level implementation 
 * @author Shriya Balu (balu@purdue.edu) 
 * 
 */

 #include "common/phal_G4/dma/dma.h"

void PHAL_DMA_priv_enableClock(dma_init_t *dma);
void PHAL_DMA_priv_disableStream(DMA_Channel_TypeDef *channel);
void PHAL_DMA_priv_clearFlags(DMA_TypeDef *dma_periph, uint8_t channel_idx);
void PHAL_DMA_priv_configParams(dma_init_t *dma);
void PHAL_DMA_priv_setChannel(DMA_TypeDef *periph, DMA_Channel_TypeDef **channel, uint8_t channel_idx);
void PHAL_DMA_priv_enableStream(DMA_Channel_TypeDef *channel);
void PHAL_DMA_priv_configMUX(dma_init_t *dma);
void PHAL_DMA_priv_setPeriphAddress(dma_init_t *dma);
uint32_t PHAL_DMA_priv_modeBits(dma_mode_t mode);
void PHAL_DMA_priv_writeTxferLength(dma_init_t *dma, const uint32_t length);
void PHAL_DMA_priv_writeMemAddress(dma_init_t *dma, const uint32_t address);