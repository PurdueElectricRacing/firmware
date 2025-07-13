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

#include <stdbool.h>

#include "stm32l4xx.h"

#define PHAL_I2C_TX_TIMEOUT (0x000000FF) //(1000U)
#define PHAL_I2C_RX_TIMEOUT (0x000000FF) //(1000U)

typedef enum {
    PHAL_I2C_MODE_TX = 0,
    PHAL_I2C_MODE_RX = 1
} I2CDirection_t;

/**
 * @brief  Enables the I2C3 peripheral by enabling the 
 *         peripheral clock and configuring I2C timing
 * 
 * @return Returns true for succesful initialization
 */
bool PHAL_initI2C(I2C_TypeDef* i2c);

/**
 * @brief         Generates an I2C start command, call
 *                before reading or writing
 * 
 * @param address I2C address of the device
 * @param length  Data length for the read or write in bytes
 * @param mode    I2C data direction 
 * @return        Returns true if the operation was successful
 */
bool PHAL_I2C_gen_start(I2C_TypeDef* i2c, uint8_t address, uint8_t length, I2CDirection_t mode);

/**
 * @brief      Sends a byte over I2C
 *             Call PHAL_I2C_gen_start before using this command
 * 
 * @param data Data to send
 * @return     Returns true if the operation was successful
 */
bool PHAL_I2C_write(I2C_TypeDef* i2c, uint8_t data);

/**
 * @brief      Sends multiple bytes over I2C
 *             Call PHAL_I2C_gen_start before using this command
 * 
 * @param data Data to send 
 * @param size Number of bytes to send
 * @return     Returns true if the operation was successful
 */
bool PHAL_I2C_write_multi(I2C_TypeDef* i2c, uint8_t* data, uint8_t size);

/**
 * @brief        Reads a byte over I2C
 *               Call PHAL_I2C_gen_start before using this command
 * 
 * @param data_a Address to write incoming data to
 * @return       Returns true if the operation was successful
 */
bool PHAL_I2C_read(I2C_TypeDef* i2c, uint8_t* data_a);

/**
 * @brief        Reads a multiple bytes over I2C
 *               Call PHAL_I2C_gen_start before using this command
 * 
 * @param data_a Address to write incoming data to
 * @return       Returns true if the operation was successful
 */
bool PHAL_I2C_read_multi(I2C_TypeDef* i2c, uint8_t* data_a, uint8_t size);

/**
 * @brief  Call this command after any I2C transmission
 * 
 * @return Returns true if the operation was successful
 */
bool PHAL_I2C_gen_stop(I2C_TypeDef* i2c);

#endif