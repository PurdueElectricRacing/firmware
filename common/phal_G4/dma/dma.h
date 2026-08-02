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


/**
 * @brief DMA transfer data width
 *
 * Selects the size of each data element transferred by the DMA.
 */
typedef enum {
    DMA_SIZE_8BIT  = 0,
    DMA_SIZE_16BIT = 1,
    DMA_SIZE_32BIT = 2
} dma_size_t;

/**
 * @brief DMA transfer direction
 *
 * Specifies whether the peripheral is the source or destination
 * of the transfer.
 *
 * Memory-to-memory transfers should use DMA_MODE_MEM2MEM
 * instead of a transfer direction.
 */
typedef enum {
    DMA_PERIPH_TO_MEMORY = 0,
    DMA_MEMORY_TO_PERIPH = 1
} dma_dir_t;

/**
 * @brief DMA channel priority.
 *
 * Higher-priority channels are serviced before lower-priority channels
 * when multiple DMA requests are pending simultaneously.
 */
typedef enum {
    DMA_PRIORITY_LOW       = 0,
    DMA_PRIORITY_MEDIUM    = 1,
    DMA_PRIORITY_HIGH      = 2,
    DMA_PRIORITY_VERY_HIGH = 3
} dma_priority_t;


/**
 * @brief DMA operating mode
 *
 * Selects how the DMA channel behaves after completing a transfer.
 *
 * Not all DMAs support memory-to-memory mode
 */
typedef enum {
    DMA_MODE_NORMAL   = 0, /**< Stop after the requested transfer completes */
    DMA_MODE_CIRCULAR = 1, /**< Automatically restart after completion */
    DMA_MODE_MEM2MEM  = 2  /**< Copy data between memory regions */
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

/**
 * @brief DMA channel initialization parameters
 *
 * The DMA channel must be disabled before modifying configuration fields after initialization.
 * Not all peripherals support every transfer direction or data width.
 * Memory-to-memory mode does not use a peripheral request and is not compatible with
 * circular mode.
 */
typedef struct {
    uint32_t periph_addr;     /**< Peripheral register address (DMA source or destination) */
    uint32_t mem_addr;        /**< Memory buffer address (DMA source or destination) */
    uint16_t tx_size;         /**< Number of data elements to transfer */
    dma_size_t mem_size;      /**< Memory transfer width */
    dma_dir_t dir;            /**< DMA transfer direction */
    bool mem_inc;             /**< Enable automatic memory address increment */
    bool periph_inc;          /**< Enable automatic peripheral address increment */
    dma_mode_t mode;          /**< DMA operating mode */
    dma_priority_t priority;  /**< DMA channel priority */
    dma_size_t periph_size;   /**< Peripheral transfer width */
    bool tx_isr_en;           /**< Enable transfer complete and transfer error interrupts */
    uint8_t dma_chan_request; /**< DMA request ID for the selected peripheral */
    uint8_t channel_idx;      /**< DMA channel number (1-8) */
    uint8_t mux_request;      /**< DMAMUX request ID used to route the peripheral request */
    DMA_TypeDef *periph;      /**< DMA peripheral instance */

    DMA_Channel_TypeDef *channel; /**< Populated internally by PHAL_initDMA() based on periph & channel_idx */
} dma_init_t;

/**
 * @brief Initialize DMA peripheral to set m2m, p2p, or p2m with set size
 *        and length of txfer
 *
 * @param dma Address of initialization structure
 * @return true if successful init (no clashing params) or false if init failed (clashing params)
 */
bool PHAL_initDMA(dma_init_t* dma);

/**
 * @brief Start txfer after sucessful DMA peripheral initialization
 *
 * @param dma Address of initialization structure
 */
void PHAL_DMA_startTxfer(dma_init_t* dma);

/**
 * @brief Stop txfer
 *
 * @param dma Address of initialization structure
 */
void PHAL_DMA_stopTxfer(dma_init_t* dma);

/**
 * @brief Re-enable DMA txfer after error ISR fires
 *
 * @param dma Address of initialization structure
 */
void PHAL_DMA_reEnable(dma_init_t* dma);

/**
 * @brief Set memory address for DMA transfer. In Mem to Mem this acts as the source address
 *
 * @param dma Address of initialization structure
 * @param address Memory address to set for DMA transfer
 */
void PHAL_DMA_setMemAddress(dma_init_t* dma, const uint32_t address);

/**
 * @brief Set transfer length for DMA transaction
 *
 * @param dma Address of initialization structure
 * @param length Number of data elements to transfer
 */
void PHAL_DMA_setTxferLength(dma_init_t* dma, const uint32_t length);

#endif // __PHAL_G4_DMA_H__
