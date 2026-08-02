/**
 * @file dma.h
 * @brief G4 DMA Peripheral public API implementation
 * @author Shriya Balu (balu@purdue.edu)
 * @author Chris McGalliard 
 * @author Dawson Moore (moore800@purdue.edu)
 * 
 */

#ifndef __PHAL_G4_DMA_H__
#define __PHAL_G4_DMA_H__

#include "common/phal_G4/phal_G4.h"

typedef enum {
    DMA_SIZE_8BIT  = 0,
    DMA_SIZE_16BIT = 1,
    DMA_SIZE_32BIT = 2
} dma_size_t;

typedef enum {
    DMA_PERIPH_TO_MEMORY = 0,
    DMA_MEMORY_TO_PERIPH = 1
} dma_dir_t;

typedef enum {
    DMA_PRIORITY_LOW       = 0,
    DMA_PRIORITY_MEDIUM    = 1,
    DMA_PRIORITY_HIGH      = 2,
    DMA_PRIORITY_VERY_HIGH = 3
} dma_priority_t;

typedef enum {
    DMA_MODE_NORMAL   = 0,
    DMA_MODE_CIRCULAR = 1,
    DMA_MODE_MEM2MEM = 2
} dma_mode_t;

// Mux requests (TODO support all)
#define DMA_REQUEST_ADC1 5U
#define DMA_REQUEST_ADC2 36U
#define DMA_REQUEST_ADC3 37U
#define DMA_REQUEST_ADC4 38U

#define DMA_REQUEST_SPI1_RX 10U
#define DMA_REQUEST_SPI1_TX 11U
#define DMA_REQUEST_SPI2_RX 12U
#define DMA_REQUEST_SPI2_TX 13U
#define DMA_REQUEST_SPI3_RX 14U
#define DMA_REQUEST_SPI3_TX 15U

#define DMA_REQUEST_USART1_RX 24U
#define DMA_REQUEST_USART1_TX 25U
#define DMA_REQUEST_USART2_RX 26U
#define DMA_REQUEST_USART2_TX 27U
#define DMA_REQUEST_USART3_RX 28U
#define DMA_REQUEST_USART3_TX 29U

typedef struct {
    uint32_t periph_addr;
    uint32_t mem_addr;
    uint16_t tx_size;
    uint8_t mem_size;

    bool increment;
    dma_dir_t dir;
    bool mem_inc;
    bool periph_inc;
    dma_mode_t mode;
    uint8_t priority;
    uint8_t periph_size;
    bool tx_isr_en;
    uint8_t dma_chan_request;
    uint8_t channel_idx;
    uint8_t mux_request;

    DMA_TypeDef* periph;
    DMA_Channel_TypeDef* channel; // Example DMA1_Stream0 or DMA2_Stream7
} dma_init_t;

/**
 * @brief Initialize DMA peripheral to set m2m, p2p, or p2m with set size
 *        and length of txfer
 *
 * @param dma -> Address of initialization structure
 * @return true -> Successful init (no clashing params)
 * @return false -> Init not complete (parameters clash)
 */
bool PHAL_initDMA(dma_init_t* dma);

/**
 * @brief Start txfer after sucessful DMA peripheral initialization
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_startTxfer(dma_init_t* dma);

/**
 * @brief Stop txfer
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_stopTxfer(dma_init_t* dma);

/**
 * @brief Re-enable DMA txfer after error ISR fires
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_reEnable(dma_init_t* dma);

/**
 * @brief Set memory address for DMA transfer. In Mem to Mem this acts as the source address
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_setMemAddress(dma_init_t* dma, const uint32_t address);

/**
 * @brief Set transfer length for DMA transaction
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_setTxferLength(dma_init_t* dma, const uint32_t length);

#endif // __PHAL_G4_DMA_H__
