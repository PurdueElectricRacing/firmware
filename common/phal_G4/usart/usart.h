#ifndef __PHAL_G4_USART_H__
#define __PHAL_G4_USART_H__

#include "common/phal_G4/phal_G4.h"

typedef struct {
    USART_TypeDef *periph;
    uint32_t baud_rate;
} PHAL_USART_Handle_t;

/**
 * @brief Initialize a USART peripheral for DMA-driven communication.
 *
 * Enables the peripheral clock and applies a fixed frame format:
 *   - 8 data bits, no parity, 1 stop bit (8N1)
 *   - 16x oversampling
 *   - no hardware flow control
 * Derives the baud-rate divisor from clock_rate, enables the IDLE-line
 * interrupt (RX frame completion) and the TX DMA transfer-complete interrupt,
 * then initializes the TX and RX DMA channels.
 *
 * Call once per USART before any tx/rx. The USART GPIO pins must already be
 * configured by the caller.
 *
 * @param handle Handle identifying the peripheral and desired baud rate
 * @param clock_rate Frequency (Hz) of the bus clock feeding this USART (APBx)
 * @return true on success, false if the peripheral is unsupported or DMA init failed
 */
bool PHAL_USART_init(PHAL_USART_Handle_t *handle, const uint32_t clock_rate);

/**
 * @brief Start a transmission using DMA.
 *
 * @param handle The handle for the USART configuration
 * @param data The address of the data to send
 * @param len Number of bytes
 * @return true if the transfer started, false otherwise
 */
bool PHAL_USART_txDMA(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len);

/**
 * @brief Start a reception using DMA of a specific length.
 *
 * @param handle The handle for the USART configuration
 * @param data The address to put the received data
 * @param len Number of bytes
 * @param cont Enable continuous RX using the IDLE-line interrupt. When set, call
 *             this function once and the HAL keeps receiving frames of the same
 *             length, invoking PHAL_USART_rxCallback after each.
 * @return true if receiving started, false otherwise
 */
bool PHAL_USART_rxDMA(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len, bool cont);

/**
 * @brief Returns whether the USART peripheral is currently transmitting data.
 *
 * @param handle Handle of USART peripheral to check
 * @return true if the peripheral is currently sending a message, false otherwise
 */
bool PHAL_USART_txBusy(PHAL_USART_Handle_t *handle);

/**
 * @brief Transmit data, blocking until the transfer completes.
 *
 * Starts a DMA transmission (see PHAL_USART_txDMA) and busy-waits until it
 * finishes. Do not call from an ISR.
 *
 * @param handle Handle of the USART to transmit on
 * @param data Buffer to send
 * @param len Number of bytes to send
 * @return true if the transfer completed, false if it failed to start
 */
bool PHAL_USART_txBl(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len);

/**
 * @brief Receive data, blocking until a one-shot reception completes.
 *
 * Starts a one-shot DMA reception (see PHAL_USART_rxDMA) and busy-waits until
 * the IDLE-line ISR signals the frame is complete. Do not call from an ISR.
 *
 * @param handle Handle of the USART to receive on
 * @param data Buffer to receive into
 * @param len Number of bytes to receive
 * @return true if the reception completed, false if it failed to start
 */
bool PHAL_USART_rxBl(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len);

/**
 * @brief Weak callback invoked when a full RX frame is received. Override in
 *        application code. Runs in ISR context, so keep it light.
 *
 * @param handle Handle of the USART that received the frame
 */
extern void PHAL_USART_rxCallback(PHAL_USART_Handle_t *handle);

#endif // __PHAL_G4_USART_H__
