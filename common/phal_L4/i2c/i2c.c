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


bool PHAL_initI2C()
{
    // Enable I2C3 clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C3EN;

    // Disable I2C peripheral
    I2C3->CR1 &= ~I2C_CR1_PE;

    // Confiure timing
    I2C3->TIMINGR &= 0x0F000000;
    I2C3->TIMINGR |= 0x30408CFF;

    //OA1
    I2C3->OAR1 |= I2C_OAR1_OA1EN;

    // Enable I2C peripheral
    I2C3->CR1 |= I2C_CR1_PE;

    return true;
}

bool PHAL_I2C_gen_start(uint8_t address, uint8_t length, uint8_t mode)
{
    uint32_t timeout = 0;
    // Wait until not busy
    while((I2C3->ISR & I2C_ISR_BUSY) && ++timeout < PHAL_I2C_TX_TIMEOUT)
    ;
    if (timeout == PHAL_I2C_TX_TIMEOUT)
        return false;
    timeout = 0;

    // Configure for start write
    I2C3->CR2 &= 0xF0000000;
    if (mode == PHAL_I2C_MODE_RX)
    {
        I2C3->CR2 |= I2C_CR2_RD_WRN;
    }
    I2C3->CR2 |= I2C_CR2_START | ((uint32_t) address) | I2C_CR2_AUTOEND |
                 (((uint32_t) length) << I2C_CR2_NBYTES_Pos);
    
    return true;
}

bool PHAL_I2C_send_byte(uint8_t data)
{

    uint32_t timeout = 0;
    // wait for TXIS flag
    while(!(I2C3->ISR & I2C_ISR_TXIS) && ++timeout < PHAL_I2C_TX_TIMEOUT)
    {
        // check NACK
        if (I2C3->ISR & I2C_ISR_NACKF)
        {
            return false;
        }
    }
    if (timeout == PHAL_I2C_TX_TIMEOUT)
        return false;
    timeout = 0;

    // write data to TXDR
    I2C3->TXDR = data;

    return true;
}

bool PHAL_I2C_read_byte(uint8_t* data_a)
{
    uint32_t timeout = 0;
    // wait for RXNE flag
    while(!(I2C3->ISR & I2C_ISR_RXNE) && ++timeout < PHAL_I2C_RX_TIMEOUT)
    {
        // check NACK
        if (I2C3->ISR & I2C_ISR_NACKF)
        {
            return false;
        }
        // check stopf
        if (I2C3->ISR & I2C_ISR_STOPF)
        {
            if (I2C3->ISR & I2C_ISR_RXNE)
            {
                break;
            }
        }
    } 
    if (timeout == PHAL_I2C_RX_TIMEOUT)
        return false;
    timeout = 0;

    // read data from RXDR
    *data_a = I2C3->RXDR;

    return true;
}

bool PHAL_I2C_gen_stop()
{
    uint32_t timeout = 0;

    // check TC flag

    // request stop
    //I2C3->CR2 |= I2C_CR2_STOP;

    // wait for STOPF flag
    while(!(I2C3->ISR & I2C_ISR_STOPF) && ++timeout < PHAL_I2C_TX_TIMEOUT)
    {
        // check NACK
        if (I2C3->ISR & I2C_ISR_NACKF)
        {
            return false;
        }
    }
    if (timeout == PHAL_I2C_TX_TIMEOUT)
        return false;
    timeout = 0;

    // clear STOPF flag
    I2C3->ICR |= I2C_ICR_STOPCF;

    // clear config register
    I2C3->CR2 &= 0xF0000000;

    return true;
}

// bool PHAL_txI2CMessage(uint8_t address, uint8_t* data_a, uint8_t length)
// {
//     //TODO: support for > 255
    
    
//     for(uint8_t i = 0; i < length; i++)
//     {
        
//     }

//     // unnecessary to check TC flag in auto end mode
//     // wait for STOPF flag
//     while(!(I2C3->ISR & I2C_ISR_STOPF) && ++timeout < PHAL_I2C_TX_TIMEOUT)
//     ;
//     if (timeout == PHAL_I2C_TX_TIMEOUT)
//         return false;
//     timeout = 0;
//     // clear STOPF flag
//     I2C3->ISR &= ~I2C_ISR_STOPF;
//     // clear CR2
//     I2C3->CR2 &= 0xF0000000;

//     return true;
// }

// bool PHAL_rxI2CMessage(uint8_t address, uint8_t* data_a, uint8_t length)
// {
//     // TODO: support > 255
//     uint32_t timeout = 0;
//     // Wait until not busy
//     while((I2C3->ISR & I2C_ISR_BUSY) && ++timeout < PHAL_I2C_RX_TIMEOUT)
//     ;
//     if (timeout == PHAL_I2C_RX_TIMEOUT)
//         return false;
//     timeout = 0;

//     // Configure for start read with auto end
//     I2C3->CR2 &= 0xF0000000;
//     I2C3->CR2 |= I2C_CR2_AUTOEND | I2C_CR2_START | I2C_CR2_RD_WRN | ((uint32_t) address) | 
//                  (((uint32_t) length) << I2C_CR2_NBYTES_Pos);
    
//     for(uint8_t i = 0; i < length; i++)
//     {
        
//     }

    
//     // clear CR2
//     I2C3->CR2 &= 0xF0000000;

//     return true;
// }