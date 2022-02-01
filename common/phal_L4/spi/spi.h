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
#include <stdbool.h>

/**
 * @brief Configuration entry for SPI initilization
 */
typedef struct {
    uint32_t data_rate;     /* Target baudrate in b/s (might be different depending on peripheral clock divison) */
    uint8_t data_len;       /* Number of bits per transaction */
    bool nss_sw;            /* Save Select controlled by Software */

    dma_init_t* dma_config; /* DMA configuration structure, SPI will setup the DMA channel */
    uint32_t* rx_buffer;    /* Location for where DMA should place RX data */
    uint32_t* tx_buffer;    /* Location for where DMA should take TX data from */
} SPI_InitConfig_t;

bool PHAL_SPI_init(const SPI_InitConfig_t* handle);

bool PHAL_SPI_read(uint32_t address, uint32_t *rx_data);
bool PHAL_SPI_write(uint32_t data);


#endif /* _PHAL_SPI_H */