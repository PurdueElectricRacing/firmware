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
#include "common/phal_L4/dma/dma.h"
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
    uint32_t       baud_rate;       // Baud rate for communication
    word_length_t  word_length;     // Word length for tx/rx (8 default)
    stop_bits_t    stop_bits;       // Number of stop bits to use (1 default)
    parity_t       parity;          // Parity of communication (none default)
    mode_t         mode;            // TX/RX mode (TX & RX default)
    hw_flow_ctl_t  hw_flow_ctl;     // Special hardware modes (none default)
    ovsample_t     ovsample;        // 8x or 16x oversample (16x default)
    obsample_t     obsample;        // One bit sampling enable (off default)

    // Advanced features
    adv_feature_t  adv_feature;     // "Advanced" feature set (only use if necessary)

    // DMA configurations
    dma_init_t*    tx_dma_cfg;      // TX configuration
    dma_init_t*    rx_dma_cfg;      // RX configuration

    volatile bool  _busy;           // Transaction busy
    volatile bool  _error;          // Transaction error
} usart_init_t;

typedef struct {
    uint32_t err_occ;
    uint32_t bytes_tx;
    uint32_t bytes_rx;
} usart_debug_t;

// Function Prototypes
bool PHAL_initUSART(USART_TypeDef* instance, usart_init_t* handle, const uint32_t fck);
void PHAL_usartTxBl(USART_TypeDef* instance, const uint16_t* data, uint32_t len);
void PHAL_usartRxBl(USART_TypeDef* instance, uint16_t* data, uint32_t len);
bool PHAL_usartTxInt(USART_TypeDef* instance, uint16_t* data, uint32_t len);
bool PHAL_usartRxInt(USART_TypeDef* instance, uint16_t* data, uint32_t len);
bool PHAL_usartTxDma(USART_TypeDef* instance, uint16_t* data, uint32_t len);
bool PHAL_usartRxDma(USART_TypeDef* instance, uint16_t* data, uint32_t len);

#define USART1_TXDMA_CONT_CONFIG(tx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(USART1->TDR), .mem_addr=(uint32_t) (tx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b1, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=false, .dma_chan_request=0b0001, .channel_idx=3,                \
     .periph=DMA1, .channel=DMA1_Channel5, .request=DMA1_CSELR}

#define USART1_RXDMA_CONT_CONFIG(rx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(USART1->RDR), .mem_addr=(uint32_t) (rx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b0, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=true, .dma_chan_request=0b0001, .channel_idx=2,                 \
     .periph=DMA1, .channel=DMA1_Channel4, .request=DMA1_CSELR}

#endif