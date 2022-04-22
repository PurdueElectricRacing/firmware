#ifndef _I2C_ALT_H_
#define _I2C_ALT_H_

#include "stm32l4xx.h"
#include <stdbool.h>

#include "common/phal_L4/dma/dma.h"

// Defines
#define MAX_ERR             4       // Maximum errors to store
typedef uint32_t ptr_int;           // Pointer to uint32_t

// Enumerations
enum {
    E_CONFIG = 1,                   // Improper config error
    E_NACK,                         // NACK when shouldn't see a NACK
    E_BUSY,                         // I2C busy
    E_NO_START                      // Start not generated
};

// Structures
typedef struct {
    int         error[MAX_ERR];     // Previous and current error codes
    uint8_t     err_idx;            // Current error index

    // DMA configurations
    dma_init_t* tx_dma_cfg;         // TX configuration
    dma_init_t* rx_dma_cfg;         // RX configuration

    uint8_t     _tx_busy;           // Waiting on a transmission to finish
    uint8_t     _rx_busy;           // Waiting on a reception to finish
} i2c_core_t;

// Prototypes
int PHAL_initI2C_a(I2C_TypeDef* instance, uint32_t freq, const uint32_t fck);
int PHAL_I2Cwrite(I2C_TypeDef* instance, uint8_t addr, const uint8_t* data, uint8_t len, bool force);
int PHAL_I2Cread(I2C_TypeDef* instance, uint8_t addr, uint8_t* data, uint8_t len, bool force);

// TODO: Fix channels to actually align with what I2C wants
#define I2C1_TXDMA_CONT_CONFIG(tx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(I2C1->TXDR), .mem_addr=(uint32_t) (tx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b1, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=true, .dma_chan_request=0b0010, .channel_idx=4,                \
     .periph=DMA1, .channel=DMA1_Channel4, .request=DMA1_CSELR}

#define I2C1_RXDMA_CONT_CONFIG(rx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(I2C1->RXDR), .mem_addr=(uint32_t) (rx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b0, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=true, .dma_chan_request=0b0010, .channel_idx=5,                 \
     .periph=DMA1, .channel=DMA1_Channel5, .request=DMA1_CSELR}

#define I2C3_TXDMA_CONT_CONFIG(tx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(I2C3->TXDR), .mem_addr=(uint32_t) (tx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b1, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=true, .dma_chan_request=0b0010, .channel_idx=4,                \
     .periph=DMA1, .channel=DMA1_Channel4, .request=DMA1_CSELR}

#define I2C3_RXDMA_CONT_CONFIG(rx_addr_, priority_)                           \
    {.periph_addr=(uint32_t) &(I2C3->RXDR), .mem_addr=(uint32_t) (rx_addr_),   \
     .tx_size=1, .increment=false, .circular=false,                             \
     .dir=0b0, .mem_inc=true, .periph_inc=false, .mem_to_mem=false,             \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,                  \
     .tx_isr_en=true, .dma_chan_request=0b0010, .channel_idx=5,                 \
     .periph=DMA1, .channel=DMA1_Channel5, .request=DMA1_CSELR}

#endif