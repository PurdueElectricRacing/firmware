/**
 * @file i2c.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-10-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _PHAL_I2C_H
#define _PHAL_I2C_H

#include "stm32l4xx.h"
#include <stdbool.h>

#define PHAL_I2C_TX_TIMEOUT (1000U)
#define PHAL_I2C_RX_TIMEOUT (1000U)
#define PHAL_I2C_MODE_TX 0
#define PHAL_I2C_MODE_RX 1

// TODO: doc
bool PHAL_initI2C();
bool PHAL_I2C_gen_start(uint8_t address, uint8_t length, uint8_t mode);
bool PHAL_I2C_send_byte(uint8_t data);
bool PHAL_I2C_read_byte(uint8_t* data_a);
bool PHAL_I2C_gen_stop();
//bool PHAL_txI2CMessage(uint8_t address, uint8_t* data_a, uint8_t length);
//bool PHAL_rxI2CMessage(uint8_t address, uint8_t* data_a, uint8_t length);



#endif