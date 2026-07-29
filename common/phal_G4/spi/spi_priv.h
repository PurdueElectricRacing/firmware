/**
 * @file spi_priv.h
 * @author Shriya Balu (balu@purdue.edu)
 * @author Ronak Jain (jain717@purdue.edu)
 * @brief Internal G4 SPI helpers and DMA configuration macros.
 */

#ifndef _PHAL_G4_SPI_PRIV_H
#define _PHAL_G4_SPI_PRIV_H

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/spi/spi.h"
#include "common/phal_G4/gpio/gpio.h"


#define SPI1_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI1->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, /* P2M */ \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 2, /* DMA1 Channel2 typical for SPI1_RX */ \
     .mux_request      = DMA_REQUEST_SPI1_RX, \
     .periph           = DMA1}

#define SPI1_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI1->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, /* M2P */ \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 3, /* DMA1 Channel3 typical for SPI1_TX */ \
     .mux_request      = DMA_REQUEST_SPI1_TX, \
     .periph           = DMA1}

#define SPI2_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI2->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 4, /* DMA1 Channel4 typical for SPI2_RX */ \
     .mux_request      = DMA_REQUEST_SPI2_RX, \
     .periph           = DMA1}

#define SPI2_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI2->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 5, /* DMA1 Channel5 typical for SPI2_TX */ \
     .mux_request      = DMA_REQUEST_SPI2_TX, \
     .periph           = DMA1}

#define SPI3_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI3->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 2, /* DMA2 Channel2 example */ \
     .mux_request      = DMA_REQUEST_SPI3_RX, \
     .periph           = DMA2}

#define SPI3_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI3->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 3, /* DMA2 Channel3 example */ \
     .mux_request      = DMA_REQUEST_SPI3_TX, \
     .periph           = DMA2}

/* Calculates the shift for a given DMA flag and channel (1-7) */
#define DMA_FLAG_MASK(flag_base, channel) (((uint32_t)(flag_base)) << (4 * ((uint32_t)(channel) - 1)))

/* DMA flag masks */
#define DMA_TCIF_MASK(channel) DMA_FLAG_MASK(DMA_ISR_TCIF1, (channel))
#define DMA_TEIF_MASK(channel) DMA_FLAG_MASK(DMA_ISR_TEIF1, (channel))
#define DMA_HTIF_MASK(channel) DMA_FLAG_MASK(DMA_ISR_HTIF1, (channel))
#define DMA_GIF_MASK(channel)  DMA_FLAG_MASK(DMA_ISR_GIF1,  (channel))

bool PHAL_SPI_priv_enableClock(SPI_TypeDef *periph);
void PHAL_SPI_priv_configCR1(SPI_InitConfig_t *cfg, uint32_t f_div);
uint32_t PHAL_SPI_priv_calcBaudRatePrescaler(uint32_t data_rate, SPI_TypeDef *periph);
void PHAL_SPI_priv_configCR2(SPI_InitConfig_t *cfg);
void PHAL_SPI_priv_enableDMA_TX(SPI_InitConfig_t *cfg);
void PHAL_SPI_priv_enableDMA_RX(SPI_InitConfig_t *cfg);
void PHAL_SPI_priv_handleTxComplete(DMA_TypeDef *dma_periph, uint8_t channel);
void PHAL_SPI_priv_resetTransferState(SPI_InitConfig_t *cfg);
void PHAL_SPI_priv_registerActiveTx(SPI_InitConfig_t *spi);
void PHAL_SPI_priv_Enable(SPI_InitConfig_t *spi);
void PHAL_SPI_priv_Disable(SPI_InitConfig_t *spi);
#endif /* _PHAL_G4_SPI_PRIV_H */
