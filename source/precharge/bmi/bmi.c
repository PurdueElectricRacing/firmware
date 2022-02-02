/**
 * @file bmi.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2022-02-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "bmi.h"
#include "common/phal_L4/spi/spi.h"

static uint8_t spi_rx_buff[32] = {0}; 
static uint8_t spi_tx_buff[32] = {0};

bool BMI088_init(BMI088_Handle_t* bmi)
{

    /* Initilize gyro */
    bmi->spi->nss_gpio_bank = 

    spi_tx_buff[0] = (1 << 7) | (BMI088_GYRO_CHIP_ID_ADDR);
    PHAL_SPI_transfer(&spi, spi_tx_buff, 2, spi_rx_buff);
    while(PHAL_SPI_busy())
        ;
    if (spi_rx_buff[1] != 0x0F) // Check Gyro Chip ID
        return false;

    spi_rx_buff[1] = 0;
}

bool BMI088_startSelfTestGyro()
{

}

bool BMI088_verifySelfTestGyro()
{

}

bool BMI088_readGyro(uint16_t* x,uint16_t* y, uint16_t* z);