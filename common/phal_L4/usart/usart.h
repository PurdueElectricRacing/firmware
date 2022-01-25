/**
 * @file usart.h
 * @author Dawson Moore (moore800@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-12-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _PHAL_USART_H_
#define _PHAL_USART_H_

// Includes
#include "stm32l4xx.h"
#include <stdbool.h>

// Generic Defines
#define PHAL_USART_TX_TIMEOUT (0xFFFFFFFF)
#define PHAL_USART_RX_TIMEOUT (0xFFFFFFFF)

typedef uint32_t ptr_int;

// Enumerations
typedef enum {
    PT_NONE,
    PT_EVEN,
    PT_ODD
} parity_t;

typedef enum {
    WORD_8 = 0,
    WORD_9 = 0x1000,
    WORD_7 = 0x10000000
} word_length_t;

typedef enum {
    SB_ONE,
    SB_TWO,
    SB_HALF,
    SB_ONE_HALF
} stop_bits_t;

typedef enum {
    MODE_TX_RX = 0b11,
    MODE_TX    = 0b10,
    MODE_RX    = 0b01
} mode_t;

typedef enum {
    HW_DISABLE,
    CTS,
    RTS,
    CTS_RTS
} hw_flow_ctl_t;

typedef enum {
    OV_16 = 0,
    OV_8  = 1
} ovsample_t;

typedef enum {
    OB_DISABLE,
    OB_ENABLE
} obsample_t;

typedef enum {
    AB_START,
    AB_FALLING,
    AB_0X7F,
    AB_0X55
} ab_mode_t;

typedef enum {
    BLOCKING,
    INTERRUPT,
    DMA
} tx_mode_t;

typedef struct {
    bool      auto_baud;
    ab_mode_t ab_mode;
    bool      tx_inv;
    bool      rx_inv;
    bool      data_inv;
    bool      tx_rx_swp;
    bool      overrun;
    bool      dma_on_rx_err;
    bool      msb_first;
} adv_feature_t;

// Structures
typedef struct {
    // Required parameters
    USART_TypeDef* instance;        // Address of instance
    uint32_t       baud_rate;       // Baud rate for communication
    word_length_t  word_length;     // Word length for tx/rx
    stop_bits_t    stop_bits;       // Number of stop bits to use
    parity_t       parity;          // Parity of communication
    mode_t         mode;            // TODO: Find this in the reference
    hw_flow_ctl_t  hw_flow_ctl;     // TODO: Find this in the reference
    ovsample_t     ovsample;        // TODO: Find this in the reference
    obsample_t     obsample;        // TODO: Find this in the reference

    // Advanced features
    adv_feature_t  adv_feature;     // TODO: Find this in the reference
} usart_handle_t;

// Function Prototypes
bool PHAL_initUSART(const usart_handle_t* handle, const uint32_t fck);
void PHAL_usartTxBl(const usart_handle_t* handle, const uint16_t* data, uint32_t len);
void PHAL_usartRxBl(const usart_handle_t* handle, uint16_t* data, uint32_t len);
bool PHAL_usartTxInt(const usart_handle_t* handle, const uint16_t* data, uint32_t len);
bool PHAL_usartRxint(const usart_handle_t* handle, uint16_t* data, uint32_t len);
bool PHAL_usartTxDMA(const usart_handle_t* handle, const uint16_t* data, uint32_t len);
bool PHAL_usartRxDMA(const usart_handle_t* handle, uint16_t* data, uint32_t len);

#endif