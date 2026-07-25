#ifndef __PHAL_G4_USART_H__
#define __PHAL_G4_USART_H__

#include "common/phal_G4/phal_G4.h"

typedef struct {
    USART_TypeDef* periph;
    uint32_t baud_rate;
} PHAL_USART_Handle_t;

/**
 * @brief Initialize a USART Peripheral with desired settings
 *
 * @param handle Handle containing settings for USART peripheral
 * @param clock_rate Rate feeding USART peripheral (APBx bus)
 * @return true if successfully initialized USART peripheral, false otherwise
 */
bool PHAL_USART_init(PHAL_USART_Handle_t* handle, const uint32_t clock_rate);

/**
 * @brief Starts a tx using dma
 *
 * @param handle The handle for the usart configuration
 * @param data The address of the data to send
 * @param len Number of bytes
 * @return true if the transfer started, false otherwise
 */
bool PHAL_USART_txDMA(PHAL_USART_Handle_t* handle, uint8_t* data, uint32_t len);

/**
 * @brief Starts an rx using dma of a specific length
 *
 * @param handle The handle for the usart configuration
 * @param data The address to put the received data
 * @param len Number of bytes
 * @param cont Enable continuous RX using the idle line interrupt (only need to call this function once, HAL will keep receiving messages of the same length)
 * @return true if receiving started, false otherwise
 */
bool PHAL_USART_rxDMA(PHAL_USART_Handle_t* handle, uint8_t* data, uint32_t len, bool cont);

/**
 * @brief Returns whether USART peripheral is currently transmitting data
 *
 * @param handle Handle of USART peripheral to check
 * @return true USART peripheral is currently sending a message, false otherwise
 */
bool PHAL_USART_txBusy(PHAL_USART_Handle_t* handle);

/**
 * @brief Weak callback invoked when a full RX frame is received. Override in application code and runs in ISR-context.
 * 
 * @param handle Handle of the USART that received the frame
 */
extern void PHAL_USART_rxCallback(PHAL_USART_Handle_t* handle);

#endif
