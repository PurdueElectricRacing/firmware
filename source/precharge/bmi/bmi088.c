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

#include "bmi088.h"
#include "common/phal_L4/spi/spi.h"
#include "common_defs.h"

static inline void BMI088_selectGyro(BMI088_Handle_t* bmi);
static inline void BMI088_selectAcc(BMI088_Handle_t* bmi);


bool BMI088_init(BMI088_Handle_t* bmi)
{
    /* Gyro initilization  */
    BMI088_selectGyro(bmi);
    if (PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_CHIP_ID_ADDR, false) != BMI088_GYRO_CHIP_ID)
        return false;
    
    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_BANDWIDTH_ADDR, bmi->gyro_datarate);
    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);

    // Perform self tests for sensor
    BMI088_gyroSelfTestStart(bmi);
    while (!BMI088_gyroSelfTestComplete(bmi))
        ;
    
    if (!BMI088_gyroSelfTestPass(bmi))
        return false;

    /* Accelerometer initilization  */  
    BMI088_selectAcel(bmi); 
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_PWR_CTRL_ADDR, BMI088_ACC_PWR_CTRL_NORMAL);
    /* Wait a long time */
    for (int i = 0; i < 100000; i++)
        asm("nop");

    if (PHAL_SPI_readByte(bmi->spi, BMI088_ACC_CHIP_ID_ADDR, false) != BMI088_ACC_CHIP_ID)
        return false;
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_RANGE_ADDR, bmi->acc_range);
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_CONFIG_ADDR, (bmi->acc_bwp << 4) | bmi->acc_odr);
}

bool BMI088_gyroSelfTestStart(BMI088_Handle_t* bmi)
{
    BMI088_selectGyro(bmi);
    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, 0x01U);
}

bool BMI088_gyroSelfTestComplete(BMI088_Handle_t* bmi)
{
    BMI088_selectGyro(bmi);
    uint8_t self_test_res = PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, false);
    return (self_test_res & 0b10) == 0b10;
}

bool BMI088_gyroSelfTestPass(BMI088_Handle_t* bmi)
{
    BMI088_selectGyro(bmi);
    uint8_t test_result = PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, false);
    if (test_result & 0b10)
    {
        // Self test completed
        return (test_result & 0b10100) == 0b10000;
    }
    // Self test was not yet run
    return false;
}

bool BMI088_readGyro(BMI088_Handle_t* bmi, int16_t* x, int16_t* y, int16_t* z)
{
    static uint8_t spi_rx_buff[16] = {0}; 
    static uint8_t spi_tx_buff[16] = {0};
    
    BMI088_selectGyro(bmi);
    while (PHAL_SPI_busy())
        ;

    spi_tx_buff[0] = (1 << 7) | BMI088_GYRO_RATE_X_LSB_ADDR;
    PHAL_SPI_transfer(bmi->spi, spi_tx_buff, 7, spi_rx_buff);
    while (PHAL_SPI_busy())
        ;
    int16_t raw_x, raw_y, raw_z;
    raw_x =  (((int16_t) spi_rx_buff[2]) << 8) | spi_rx_buff[1];
    raw_y =  (((int16_t) spi_rx_buff[4]) << 8) | spi_rx_buff[3];
    raw_z =  (((int16_t) spi_rx_buff[6]) << 8) | spi_rx_buff[5];

    int16_t max_raw = MAX(MAX(raw_x, raw_y), raw_z);
    bool range_up   = bmi->gyro_dynamic_range && (ABS(max_raw) >= 32000); // int16_t range is -32,768 to +32,767 
    bool range_down = bmi->gyro_dynamic_range && (ABS(max_raw) <=  1000);

    // Convert raw values into physical values based on range
    // Decimal is fixed in the first place
    float scale = 0.0;
    switch(bmi->gyro_range)
    {
        case (GYRO_RANGE_2000):
            scale = (16.384 / 10);
            break;
        case (GYRO_RANGE_1000):
            scale = (32.768 / 10);
            break;
        case (GYRO_RANGE_500):
            scale = (65.536 / 10);
            if (range_down)
            {
                bmi->gyro_range = GYRO_RANGE_250;
                PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
            }
            break;
        case (GYRO_RANGE_250):
            scale = (131.072 / 10);

            if (range_up)
            {
                bmi->gyro_range = GYRO_RANGE_500;
                PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
            }
            else
            if (range_down)
            {
                bmi->gyro_range = GYRO_RANGE_125;
                PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
            }
            break;
        case (GYRO_RANGE_125):
            scale = (262.144 / 10);

            if (range_up)
            {
                bmi->gyro_range = GYRO_RANGE_250;
                PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
            }
            break;
    }

    *x = (int16_t) (raw_x / scale);
    *y = (int16_t) (raw_y / scale);
    *z = (int16_t) (raw_z / scale);

    return true;
}

bool BMI088_readAcc(BMI088_Handle_t* bmi, int16_t* ax, int16_t* ay, int16_t* az)
{
    static uint8_t spi_rx_buff[16] = {0}; 
    static uint8_t spi_tx_buff[16] = {0};

    BMI088_selectAcc(bmi);
    while (PHAL_SPI_busy())
        ;
    
    spi_tx_buff[0] = (1 << 7) | BMI088_ACC_RATE_X_LSB_ADDR;
    PHAL_SPI_transfer(bmi->spi, spi_tx_buff, 7, spi_rx_buff);
    while (PHAL_SPI_busy())
        ;
    int16_t raw_ax, raw_ay, raw_az;
    raw_ax =  (((int16_t) spi_rx_buff[2]) << 8) | spi_rx_buff[1];
    raw_ay =  (((int16_t) spi_rx_buff[4]) << 8) | spi_rx_buff[3];
    raw_az =  (((int16_t) spi_rx_buff[6]) << 8) | spi_rx_buff[5];

    // Conversion taken from
    float scale = 0.0457763f * (2 << (bmi->acc_range + 1));

    *ax = (int16_t) (raw_ax / scale);
    *ay = (int16_t) (raw_ay / scale);
    *az = (int16_t) (raw_az / scale);

    return true;
}


static inline void BMI088_selectGyro(BMI088_Handle_t* bmi)
{
    bmi->spi->nss_gpio_port = bmi->gyro_csb_gpio_port;
    bmi->spi->nss_gpio_pin = bmi->gyro_csb_pin;
}

static inline void BMI088_selectAcc(BMI088_Handle_t* bmi)
{
    bmi->spi->nss_gpio_port = bmi->acc_csb_gpio_port;
    bmi->spi->nss_gpio_pin  = bmi->acc_csb_pin;
}