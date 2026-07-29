/**
 * @file dma_priv.h
 * @brief G4 DMA Peripheral private/register level implementation 
 * @author Shriya Balu (balu@purdue.edu) 
 * 
 */

 #include "common/phal_G4/dma/dma.h"

 bool PHAL_DMA_priv_validateConfig(dma_init_t *dma);
 void PHAL_DMA_priv_enableClock(dma_init_t *dma);
 void PHAL_DMA_priv_disableStream(DMA_Channel_TypeDef *channel);
 void PHAL_DMA_priv_clearFlags(DMA_TypeDef *dma_periph, uint8_t channel_idx);
 void PHAL_DMA_priv_configParams(dma_init_t *dma);
 void PHAL_DMA_priv_setChannel(DMA_TypeDef *periph, DMA_Channel_TypeDef **channel, uint8_t channel_idx);
 void PHAL_DMA_priv_enableStream(DMA_Channel_TypeDef *channel);
 void PHAL_DMA_priv_configMUX(dma_init_t *dma);
 void PHAL_DMA_priv_setPeriphAddress(dma_init_t *dma);