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
#include <stdbool.h>

/**
 * @brief Configuration entry for SPI initilization
 */
typedef struct {
    SPI_TypeDef* handle;
    uint32_t baud;
    uint8_t data_len; /* Number of bits per transaction */
    bool nss_hw; /* Save Select controlled by HW */
} SPI_Handle_t;

bool PHAL_SPI_init(const SPI_Handle_t* handle);

bool PHAL_SPI_read();


#endif /* _PHAL_SPI_H */