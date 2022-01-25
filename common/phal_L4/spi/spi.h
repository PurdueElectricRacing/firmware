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
    SPI_TypeDef* bus;
    
} SPIInitConfig_t;

bool PHAL_initSPI();




#endif /* _PHAL_SPI_H */