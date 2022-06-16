/**
 * @file i2c.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief I2C library for sending and receiving data
 * @version 0.1
 * @date 2021-10-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "common/phal_L4/i2c/i2c.h"


bool PHAL_initI2C(I2C_TypeDef *i2c)
{
    // Enable I2C clock
    if (i2c == I2C1) RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    else if (i2c == I2C3) RCC->APB1ENR1 |= RCC_APB1ENR1_I2C3EN;

    // Disable I2C peripheral
    i2c->CR1 &= ~I2C_CR1_PE;

    // Confiure timing
    i2c->TIMINGR &= 0x0F000000;
    i2c->TIMINGR |= 0x00103131; // Generating using CubeMx

    //OA1
    i2c->OAR1 |= I2C_OAR1_OA1EN;

    // Enable I2C peripheral
    i2c->CR1 |= I2C_CR1_PE;

    return true;
}

bool PHAL_I2C_gen_start(I2C_TypeDef *i2c, uint8_t address, uint8_t length, I2CDirection_t mode)
{
    uint32_t timeout = 0;
    // Wait until not busy
    while((i2c->ISR & I2C_ISR_BUSY) && ++timeout < PHAL_I2C_TX_TIMEOUT);
    if (timeout == PHAL_I2C_TX_TIMEOUT) return false;
    timeout = 0;

    // Configure for start write
    i2c->CR2 &= 0xF0000000;         // clear register
    if (mode == PHAL_I2C_MODE_RX) 
    {
        i2c->CR2 |= I2C_CR2_RD_WRN; // configure for reading
    }
    i2c->CR2 |= ((uint32_t) address) | I2C_CR2_AUTOEND | (((uint32_t) length) << I2C_CR2_NBYTES_Pos);
    i2c->CR2 |= I2C_CR2_START;

    return true;
}

bool PHAL_I2C_write(I2C_TypeDef *i2c, uint8_t data)
{

    uint32_t timeout = 0;
    // wait for TXIS flag
    while(!(i2c->ISR & I2C_ISR_TXIS) && ++timeout < PHAL_I2C_TX_TIMEOUT)
    {
        // check NACK
        if (i2c->ISR & I2C_ISR_NACKF) return false;
    }
    if (timeout == PHAL_I2C_TX_TIMEOUT) return false;
    timeout = 0;

    i2c->TXDR = data; // write data

    return true;
}

bool PHAL_I2C_write_multi(I2C_TypeDef *i2c, uint8_t* data, uint8_t size)
{
    for(uint8_t i = 0; i < size; i++)
    {
        if (!PHAL_I2C_write(i2c, data[i])) return false;
    }
    return true;
}

bool PHAL_I2C_read(I2C_TypeDef *i2c, uint8_t* data_a)
{
    uint32_t timeout = 0;
    // wait for RXNE flag
    while(!(i2c->ISR & I2C_ISR_RXNE) && ++timeout < PHAL_I2C_RX_TIMEOUT)
    {
        if (i2c->ISR & I2C_ISR_NACKF) return false; // check NACK

        // check stopf
        if (i2c->ISR & I2C_ISR_STOPF)
        {
            if (i2c->ISR & I2C_ISR_RXNE)
            {
                timeout += 1;
                break;
            }
        }
    }

    timeout = 0;

    // read data from RXDR
    *data_a = i2c->RXDR;

    return true;
}

bool PHAL_I2C_read_multi(I2C_TypeDef *i2c, uint8_t* data_a, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        if(!PHAL_I2C_read(i2c, &(data_a[i]))) return false;
    }
    return true;
}

bool PHAL_I2C_gen_stop(I2C_TypeDef *i2c)
{
    uint32_t timeout = 0;

    // wait for STOPF flag
    while(!(i2c->ISR & I2C_ISR_STOPF) && ++timeout < PHAL_I2C_TX_TIMEOUT)
    {
        // check NACK
        if (i2c->ISR & I2C_ISR_NACKF)
        {
            return false;
        }
    }
    if (timeout == PHAL_I2C_TX_TIMEOUT)
        return false;
    timeout = 0;

    // clear STOPF flag
    i2c->ICR |= I2C_ICR_STOPCF;

    // clear config register
    i2c->CR2 &= 0xF0000000;

    return true;
}