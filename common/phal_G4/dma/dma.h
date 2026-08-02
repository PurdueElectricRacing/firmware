/**
 * @file dma.h
 * @brief G4 DMA Peripheral public API implementation
 * @author Shriya Balu (balu@purdue.edu)
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#ifndef PHAL_G4_DMA_H
#define PHAL_G4_DMA_H

#include <stdint.h>

#include "common/phal_G4/phal_G4.h"


/// Width of each DMA data element
typedef enum {
    DMA_SIZE_8BIT  = 0,
    DMA_SIZE_16BIT = 1,
    DMA_SIZE_32BIT = 2,
} PHAL_DMA_Size_t;

/**
 * @brief Direction of a peripheral <-> memory transfer
 * 
 * Ignored for DMA_MODE_MEM2MEM, which moves data between two memory
 * addresses with no peripheral or DMAMUX request involved.
 */
typedef enum {
    DMA_PERIPH_TO_MEMORY = 0,
    DMA_MEMORY_TO_PERIPH = 1,
} PHAL_DMA_Direction_t;

/// Relative priority when multiple DMA requests are pending at once
typedef enum {
    DMA_PRIORITY_LOW       = 0,
    DMA_PRIORITY_MEDIUM    = 1,
    DMA_PRIORITY_HIGH      = 2,
    DMA_PRIORITY_VERY_HIGH = 3,
} PHAL_DMA_Priority_t;

/// What a channel does once its transfer count reaches zero
typedef enum {
    DMA_MODE_NORMAL   = 0, /**< Stops (call PHAL_DMA_restart() to re-run) */
    DMA_MODE_CIRCULAR = 1, /**< Automatically reload the count and restart */
    DMA_MODE_MEM2MEM  = 2, /**< Copy memory-to-memory (no DMAMUX request involved_ */
} PHAL_DMA_Mode_t;


/**
 * @brief DMAMUX request IDs (RM0440 Table 91)
 * 
 * Not every peripheral is listed yet
 */
typedef enum : uint8_t {
    DMA_REQUEST_ADC1 = 5U,
    DMA_REQUEST_ADC2 = 36U,
    DMA_REQUEST_ADC3 = 37U,
    DMA_REQUEST_ADC4 = 38U,

    DMA_REQUEST_SPI1_RX = 10U,
    DMA_REQUEST_SPI1_TX = 11U,
    DMA_REQUEST_SPI2_RX = 12U,
    DMA_REQUEST_SPI2_TX = 13U,
    DMA_REQUEST_SPI3_RX = 14U,
    DMA_REQUEST_SPI3_TX = 15U,

    DMA_REQUEST_USART1_RX = 24U,
    DMA_REQUEST_USART1_TX = 25U,
    DMA_REQUEST_USART2_RX = 26U,
    DMA_REQUEST_USART2_TX = 27U,
    DMA_REQUEST_USART3_RX = 28U,
    DMA_REQUEST_USART3_TX = 29U,
} PHAL_DMA_Request_t;

/**
 * @brief Fixed hardware wiring for one peripheral + direction's DMA use
 *
 * Define exactly one of these per peripheral + direction combo as a static/global constant.
 * - Ex: one for SPI1 RX, one for SPI1 TX
 * Don't construct inline in a PHAL_DMA_Handle_t usage. The fields are a direct representation
 * about how the MCU is internally connected, not a user choice.
 */
typedef struct {
    DMA_TypeDef *periph;            /*!< DMA1 or DMA2 */
    uint8_t channel_idx;            /*!< Channel number, 1-8 */
    PHAL_DMA_Request_t mux_request; /*!< DMAMUX request ID */
    volatile void *periph_reg;      /*!< Peripheral data register DMA reads/writes */
    union {
        PHAL_DMA_Direction_t dir;   /*!< Ignored for DMA_MODE_MEM2MEM */
    };
    PHAL_DMA_Size_t data_size;      /*!< Used for both the memory and peripheral side */
} PHAL_DMA_Wiring_t;

/**
 * @brief Per-transfer parameters (user can configure)
 */
typedef struct {
    uint32_t mem_addr;         /*!< Memory buffer address, second memory address for MEM2MEM */
    uint16_t tx_size;          /*!< Number of data elements to transfer */
    PHAL_DMA_Priority_t priority;
    PHAL_DMA_Mode_t mode;
    bool tx_isr_en;            /*!< Enable transfer-complete and transfer-error interrupts */
} PHAL_DMA_Params_t;

/**
 * @brief A configured DMA transfer: fixed wiring + chosen parameters
 *
 * channel is populated by PHAL_DMA_init(), do not specify when constructing PHAL_DMA_Handle_t.
 */
typedef struct {
    const PHAL_DMA_Wiring_t *wiring;
    PHAL_DMA_Params_t params;
    DMA_Channel_TypeDef *channel; /**< Populated by PHAL_DMA_init()!! */
} PHAL_DMA_Handle_t;


/**
 * @brief Claim a DMA channel and configure it from handle->wiring and handle->params
 
 * Does not start the transfer (call PHAL_DMA_start() afterward).
 *
 * @param handle wiring + params to configure. handle->channel is populated in on success
 * @return true on success; false if handle/wiring invalid (NULL, channel_idx
 * is outside 1-8, periph isn't DMA1/DMA2, or that periph/channel_idx
 * is already claimed by another live handle)
 */
bool PHAL_DMA_init(PHAL_DMA_Handle_t *handle);

/**
 * @brief Disable the channel and release its claim so another handle can
 * use the periph/channel_idx afterward
 * @return true on success, false if handle was never successfully init-ed
 */
bool PHAL_DMA_deinit(PHAL_DMA_Handle_t *handle);

/**
 * @brief Enable the channel, starting the transfer configured by PHAL_DMA_init()
 * @return true on success, false if handle was never successfully init-ed
 */
bool PHAL_DMA_start(PHAL_DMA_Handle_t *handle);

/**
 * @brief Disable the channel, stopping the transfer immediately
 * 
 * Whatever hasn't transferred yet is left un-transferred
 * 
 * @return true on success, false if handle was never successfully init-ed
 */
bool PHAL_DMA_stop(PHAL_DMA_Handle_t *handle);

/**
 * @brief Reload the transfer count from handle->params.tx_size and start again
 * 
 * Safe to call regardless of whether the channel state (enabled, mid-transfer, or stopped on an error),
 * it always disables first.
 *
 * @return true on success, false if handle was never successfully init-ed
 */
bool PHAL_DMA_restart(PHAL_DMA_Handle_t *handle);

/**
 * @brief Change the memory address for the next transfer
 * @return true on success, false if handle was never successfully
 * init-ed or the channel is currently enabled
 */
bool PHAL_DMA_setMemAddress(PHAL_DMA_Handle_t *handle, uint32_t address);

/**
 * @brief Change the transfer length (element count) for the next transfer
 * @return true on success, false if handle was never successfully
 * init-ed, or the channel is currently enabled
 */
bool PHAL_DMA_setLength(PHAL_DMA_Handle_t *handle, uint16_t length);

/**
 * @brief Check whether the channel is currently enabled (transfer in
 * progress, or in circular mode, running continuously)
 *
 * @return true if the channel is enabled, false if disabled or handle
 * was never successfully init-ed
 */
bool PHAL_DMA_isBusy(PHAL_DMA_Handle_t *handle);

/**
 * @brief Weak callback for when a channel's transfer completes
 * 
 * Called from DMAx_ChannelY_IRQHandler. Override in application code.
 * Default implementation does nothing.
 */
extern void PHAL_DMA_txCompleteCallback(DMA_TypeDef *periph, uint8_t channel_idx);

/**
 * @brief Weak callback for when a channel reports a transfer error
 *
 * Called from DMAx_ChannelY_IRQHandler. Override in application code.
 * Default implementation does nothing.
 */
extern void PHAL_DMA_txErrorCallback(DMA_TypeDef *periph, uint8_t channel_idx);

#endif // PHAL_G4_DMA_H