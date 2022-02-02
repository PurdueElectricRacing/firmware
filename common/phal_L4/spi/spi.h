/**
 * @file spi.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2022-01-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _PHAL_SPI_H
#define _PHAL_SPI_H

#include "stm32l4xx.h"
#include "common/phal_L4/dma/dma.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common_defs.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Configuration entry for SPI initilization
 */
typedef struct {
    uint32_t data_rate;     /* Target baudrate in b/s (might be different depending on peripheral clock divison) */
    uint8_t data_len;       /* Number of bits per transaction */
    bool nss_sw;            /* Save Select controlled by Software */
    GPIO_TypeDef* nss_gpio_bank;
    uint32_t nss_gpio_pin;

    dma_init_t* rx_dma_cfg;    /* DMA initilization for RX transfer */
    dma_init_t* tx_dma_cfg;    /* DMA initilization for TX transfer */
    
    volatile bool _busy;
} SPI_InitConfig_t;

bool PHAL_SPI_init(SPI_InitConfig_t* handle);

bool PHAL_SPI_transfer(SPI_InitConfig_t* spi, const uint8_t* out_data, const uint32_t data_len, const uint8_t* in_data);
bool PHAL_SPI_busy();

#define SPI1_RXDMA_CONT_CONFIG(rx_addr_, priority_)        \
    {.periph_addr=(uint32_t) &(SPI1->DR), .mem_addr=(uint32_t) (rx_addr_),      \
     .tx_size=1, .increment=false, .circular=false,            \
     .dir=0b0, .mem_inc=true, .periph_inc=false, .mem_to_mem=false, \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,        \
     .tx_isr_en=true, .dma_chan_request=0b0001, .channel_idx=2,    \
     .periph=DMA1, .channel=DMA1_Channel2, .request=DMA1_CSELR}
    
#define SPI1_TXDMA_CONT_CONFIG(tx_addr_, priority_)        \
    {.periph_addr=(uint32_t) &(SPI1->DR), .mem_addr=(uint32_t) (tx_addr_),      \
     .tx_size=1, .increment=false, .circular=false,            \
     .dir=0b1, .mem_inc=true, .periph_inc=false, .mem_to_mem=false, \
     .priority=(priority_), .mem_size=0b00, .periph_size=0b00,        \
     .tx_isr_en=false, .dma_chan_request=0b0001, .channel_idx=3,    \
     .periph=DMA1, .channel=DMA1_Channel3, .request=DMA1_CSELR}


#endif /* _PHAL_SPI_H */