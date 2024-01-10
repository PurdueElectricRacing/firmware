/**
 * @file usart.h
 * @author Aditya Anand (anand89@purdue.edu) - Port of L4 USART HAL by Dawson Moore (moore800@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-1-4
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _PHAL_USART_H_
#define _PHAL_USART_H_

// Includes
#if defined(STM32F407xx)
#include "stm32f4xx.h"
#define TOTAL_NUM_UART 8

// Active Transfer list indexes (Add to this list if updating TOTAL_NUM_UART)
#define USART1_ACTIVE_IDX 0
#define USART2_ACTIVE_IDX 1
#define USART3_ACTIVE_IDX 2
#define USART4_ACTIVE_IDX 3
#define USART5_ACTIVE_IDX 4
#define USART6_ACTIVE_IDX 5
#define USART7_ACTIVE_IDX 6
#define USART8_ACTIVE_IDX 7
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#define TOTAL_NUM_UART 8

// Active Transfer list indexes (Add to this list if updating TOTAL_NUM_UART)
#define USART1_ACTIVE_IDX 0
#define USART2_ACTIVE_IDX 1
#define USART3_ACTIVE_IDX 2
#define USART4_ACTIVE_IDX 3
#define USART5_ACTIVE_IDX 4
#define USART6_ACTIVE_IDX 5
#define USART7_ACTIVE_IDX 6
#define USART8_ACTIVE_IDX 7

// Defines that mean the same thing but are phrased differently in stm32f4xx and stm32f7xx
#define USART_BRR_DIV_Fraction_Pos USART_BRR_DIV_FRACTION_Pos
#define USART_BRR_DIV_Mantissa_Pos USART_BRR_DIV_MANTISSA_Pos
#endif
#include "common/phal_F4_F7/dma/dma.h"
#include <stdbool.h>

// Generic Defines
#define PHAL_USART_TX_TIMEOUT (0xFFFFFFFF)
#define PHAL_USART_RX_TIMEOUT (0xFFFFFFFF)

typedef uint32_t ptr_int;

// Enumerations
// See Table 146 in RM. 0090
// By enabling Parity, you sacrifice one bit from your total word length 
// (eg. if WORD_8 is selected, you now have 7 data bits and 1 parity bit)
typedef enum
{
    PT_EVEN = 0b010,
    PT_ODD = 0b100,
    PT_NONE = 0b000
} parity_t;

typedef enum
{
    WORD_8,
    WORD_9,
} word_length_t;

typedef enum
{
    SB_ONE = 0b00,
    SB_TWO = 0b01,
    SB_HALF = 0b10,
    SB_ONE_HALF = 0b11
} stop_bits_t;

typedef enum
{
    MODE_TX_RX = 0b11,
    MODE_TX = 0b10,
    MODE_RX = 0b01
} usart_mode_t;

typedef enum
{
    HW_DISABLE,
    CTS,
    RTS,
    CTS_RTS
} hw_flow_ctl_t;

typedef enum
{
    OV_16 = 0,
    OV_8 = 1
} ovsample_t;

typedef enum
{
    OB_DISABLE,
    OB_ENABLE
} obsample_t;

typedef enum
{
    USART_DMA_TX,
    USART_DMA_RX
} usart_dma_mode_t;

typedef struct
{
    uint8_t overrun;
    uint8_t noise_detected;
    uint8_t framing_error;
    uint8_t parity_error;
} usart_errors_t;

// Structures
typedef struct
{
    // Required parameters
    uint32_t baud_rate;        // Baud rate for communication
    word_length_t word_length; // Word length for tx/rx (8 default)
    stop_bits_t stop_bits;     // Number of stop bits to use (1 default)
    parity_t parity;           // Parity of communication (none default)
    hw_flow_ctl_t hw_flow_ctl; // Special hardware modes (none default)
    ovsample_t ovsample;       // 8x or 16x oversample (16x default)
    obsample_t obsample;       // One bit sampling enable (off default)
    bool wake_addr; //Wake up when given a specific address
    uint8_t address; //Address to wake up to when addr_mode is enabled
    uint8_t usart_active_num;    // Index of USART in active array (see USARTx_ACTIVE_IDX)

    // DMA configurations
    dma_init_t *tx_dma_cfg; // TX configuration
    dma_init_t *rx_dma_cfg; // RX configuration
    USART_TypeDef *periph;     // USART Peripheral to be used

    // Structs to communicate errors to user
    volatile usart_errors_t tx_errors;
    volatile usart_errors_t rx_errors;

    uint8_t _tx_busy; // Waiting on a transmission to finish
    uint8_t _rx_busy; // Waiting on a reception to finish
                      // The 2 above clear on calling ...xDMAComplete
} usart_init_t;

typedef struct {
    usart_init_t *active_handle;
    uint8_t cont_rx;
    uint8_t _tx_busy; // Waiting on a transmission to finish
    uint8_t _rx_busy; // Waiting on a reception to finish
                      // The 2 above clear on calling ...xDMAComplete
} usart_active_transfer_t;



typedef struct
{
    uint32_t last_msg_time; // Time of last rx message that was size of msg_size
    uint16_t msg_size;      // Size of typical msg
    uint16_t last_msg_loc;  // Index of first byte of last msg received
    uint32_t last_rx_time;  // Time of the last rx
    uint16_t rx_buf_size;   // Size of rx circular buffer for DMA
    uint16_t last_rx_loc;   // Index of byte of last rx
    char *rx_buf;           // Buffer location
} usart_rx_buf_t;

// Global vars
// uint8_t tx_irqn[3] = {DMA1_Channel4_IRQn, DMA1_Channel6_IRQn, DMA2_Channel6_IRQn};
// uint8_t rx_irqn[3] = {DMA1_Channel5_IRQn, DMA1_Channel7_IRQn, DMA2_Channel7_IRQn};

// Function Prototypes
bool PHAL_initUSART(usart_init_t* handle, const uint32_t fck);
void PHAL_usartTxBl(usart_init_t* handle, const uint16_t* data, uint32_t len);
void PHAL_usartRxBl(usart_init_t* handle, uint16_t* data, uint32_t len);

/**
 * @brief           Starts a tx using dma, use PHAL_usartTxDmaComplete
 *                  to ensure the previous transmission is complete
 * @param handle    The handle for the usart configuration
 * @param data      The address of the data to send, ensure a cast to (uint16_t *), even if 8 bits
 * @param len       Number of units of data, depending on the configured word length
 */
bool PHAL_usartTxDma(usart_init_t* handle, uint16_t* data, uint32_t len);

/**
 * @brief           Starts an rx using dma of a specific length
 *                  Use PHAL_usartRxDmaComplete to check if the entire msg has been received
 * @param handle    The handle for the usart configuration
 * @param data      The address to put the received data, ensure a cast to (uint16_t *), even if 8 bits
 * @param len       Number of units of data, depending on the configured word length
 */
bool PHAL_usartRxDma(usart_init_t* handle, uint16_t* data, uint32_t len);
bool PHAL_usartTxDmaComplete(usart_init_t* handle);
bool PHAL_usartRxDmaComplete(usart_init_t *handle);

#ifdef STM32F407xx
    #define USART2_RXDMA_CONT_CONFIG(rx_addr_, priority_)                               \
        {                                                                             \
            .periph_addr = (uint32_t) & (USART2->DR), .mem_addr = (uint32_t)(rx_addr_), \
            .tx_size = 1, .increment = false, .circular = false,                      \
            .dir = 0b0, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
            .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
            .tx_isr_en = false, .dma_chan_request=0b0100, .stream_idx=5,              \
            .periph=DMA1, .stream=DMA1_Stream5                                        \
        }

    #define USART2_TXDMA_CONT_CONFIG(tx_addr_, priority_)                               \
        {                                                                             \
            .periph_addr = (uint32_t) & (USART2->DR), .mem_addr = (uint32_t)(tx_addr_), \
            .tx_size = 1, .increment = false, .circular = false,                      \
            .dir = 0b1, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
            .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
            .tx_isr_en = true, .dma_chan_request=0b0100, .stream_idx=6,               \
            .periph=DMA1, .stream=DMA1_Stream6                                        \
        }
#else 
    #define USART2_RXDMA_CONT_CONFIG(rx_addr_, priority_)                               \
        {                                                                             \
            .periph_addr = (uint32_t) & (USART2->TDR), .mem_addr = (uint32_t)(rx_addr_), \
            .tx_size = 1, .increment = false, .circular = false,                      \
            .dir = 0b0, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
            .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
            .tx_isr_en = false, .dma_chan_request=0b0100, .stream_idx=5,              \
            .periph=DMA1, .stream=DMA1_Stream5                                        \
        }

    #define USART2_TXDMA_CONT_CONFIG(tx_addr_, priority_)                               \
        {                                                                             \
            .periph_addr = (uint32_t) & (USART2->RDR), .mem_addr = (uint32_t)(tx_addr_), \
            .tx_size = 1, .increment = false, .circular = false,                      \
            .dir = 0b1, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
            .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
            .tx_isr_en = true, .dma_chan_request=0b0100, .stream_idx=6,               \
            .periph=DMA1, .stream=DMA1_Stream6                                        \
        }
#endif
#endif