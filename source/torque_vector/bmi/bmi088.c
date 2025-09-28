/**
 * @file bmi.c
 * @author Adam Busch (busch8@purdue.edu)
 * @author Irving Wang (wang5952@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "bmi088.h"
#include "common/phal/spi.h"
#include "common_defs.h"
#include "main.h"
#include <stdbool.h>
#include <stdint.h>


static inline void BMI088_selectGyro(BMI088_Handle_t *bmi);
static inline void BMI088_selectAccel(BMI088_Handle_t *bmi);

bool BMI088_init(BMI088_Handle_t *bmi)
{
    bmi->isAccelReady = false;
    /* Gyro initilization  */
    BMI088_selectGyro(bmi);
    if (PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_CHIP_ID_ADDR, true) != BMI088_GYRO_CHIP_ID)
        return false;

    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_BANDWIDTH_ADDR, bmi->gyro_datarate);
    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);

    // Perform self tests for sensor
    BMI088_gyroSelfTestStart(bmi);
    while (!BMI088_gyroSelfTestComplete(bmi)) {
        __asm__("nop");
    }

    if (!BMI088_gyroSelfTestPass(bmi))
        return false;

    return true;
}

void BMI088_wakeAccel(BMI088_Handle_t *bmi)
{
    BMI088_selectAccel(bmi);
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_PWR_CONF_ADDR, 0);
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_PWR_CTRL_ADDR, BMI088_ACC_PWR_CTRL_NORMAL);
    return;
}

bool BMI088_initAccel(BMI088_Handle_t *bmi)
{
    BMI088_selectAccel(bmi);

    /* Wait a long time before you call this function (50ms) */
    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_PWR_CONF_ADDR, 0);
    PHAL_SPI_readByte(bmi->spi, BMI088_ACC_CHIP_ID_ADDR, false);

    PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_RANGE_ADDR, bmi->accel_range) &&
        PHAL_SPI_writeByte(bmi->spi, BMI088_ACC_CONFIG_ADDR, (bmi->accel_bwp << 4) | bmi->accel_odr);

    uint8_t read_back = PHAL_SPI_readByte(bmi->spi, BMI088_ACC_CHIP_ID_ADDR, false);

    bmi->isAccelReady = true;
    return true;
}

bool BMI088_gyroOK(BMI088_Handle_t *bmi)
{
    BMI088_gyroSelfTestStart(bmi);
    while (!BMI088_gyroSelfTestComplete(bmi)) {
        __asm__("nop");
    }

    if (!BMI088_gyroSelfTestPass(bmi))
        return false;

    return true;
}

bool BMI088_gyroSelfTestStart(BMI088_Handle_t *bmi)
{
    BMI088_selectGyro(bmi);
    PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, 0x01U);
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    return true;
}

bool BMI088_gyroSelfTestComplete(BMI088_Handle_t *bmi)
{
    BMI088_selectGyro(bmi);
    uint8_t self_test_res = PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, true);
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    return (self_test_res & 0b10) == 0b10;
}

bool BMI088_gyroSelfTestPass(BMI088_Handle_t *bmi)
{
    BMI088_selectGyro(bmi);
    uint8_t test_result = PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, true);
    if (test_result & 0b10)
    {
        // Self test completed
        PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
        return (test_result & 0b10100) == 0b10000;
    }
    // Self test was not yet run
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    return false;
}

uint8_t BMI088_checkGyroHealth(BMI088_Handle_t *bmi)
{
    uint8_t self_test_register_result = PHAL_SPI_readByte(bmi->spi, BMI088_GYRO_SELFTEST_ADDR, true);
    // self_test_register_result &= 0b00010000;
    return (self_test_register_result == 0b00010000);
}

/*
* Dynamically adjust the gyro scale based on the range.
*/
static inline float BMI088_getGyroScale(BMI088_Handle_t *bmi, bool range_up, bool range_down) {
    float scale = 0.0;
    switch (bmi->gyro_range) {
    case (GYRO_RANGE_2000):
        scale = (16.384 / DEG_TO_RAD);
        break;
    case (GYRO_RANGE_1000):
        scale = (32.768 / DEG_TO_RAD);
        break;
    case (GYRO_RANGE_500):
        scale = (65.536 / DEG_TO_RAD);
        if (range_down)
        {
            bmi->gyro_range = GYRO_RANGE_250;
            PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
        }
        break;
    case (GYRO_RANGE_250):
        scale = (131.072 / DEG_TO_RAD);

        if (range_up) {
            bmi->gyro_range = GYRO_RANGE_500;
            PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
        } else if (range_down) {
            bmi->gyro_range = GYRO_RANGE_125;
            PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
        }
        break;
    case (GYRO_RANGE_125):
        scale = (262.144 / DEG_TO_RAD);

        if (range_up) {
            bmi->gyro_range = GYRO_RANGE_250;
            PHAL_SPI_writeByte(bmi->spi, BMI088_GYRO_RANGE_ADDR, bmi->gyro_range);
        }
        break;
    default:
        scale = 1.0; // prevent div by zero
    }
}

bool BMI088_readGyro(BMI088_Handle_t *bmi) {
    uint8_t *tx_buffer = bmi->bmi_tx_buffer;
    uint8_t *rx_buffer = bmi->bmi_rx_buffer;

    BMI088_selectGyro(bmi);
    while (PHAL_SPI_busy(bmi->spi)) {
        __asm__("nop");
    }

    tx_buffer[0] = (1 << 7) | BMI088_GYRO_RATE_X_LSB_ADDR;
    PHAL_SPI_transfer_noDMA(bmi->spi, tx_buffer, 1, 7, rx_buffer);
    // while (PHAL_SPI_busy(bmi->spi))
    //     ;
    int16_t raw_x, raw_y, raw_z;
    raw_x = (((int16_t)rx_buffer[2]) << 8) | rx_buffer[1];
    raw_y = (((int16_t)rx_buffer[4]) << 8) | rx_buffer[3];
    raw_z = (((int16_t)rx_buffer[6]) << 8) | rx_buffer[5];

    int16_t max_raw = MAX(MAX(raw_x, raw_y), raw_z);
    bool range_up = bmi->enableDynamicRange && (ABS(max_raw) >= 32000); // int16_t range is -32,768 to +32,767
    bool range_down = bmi->enableDynamicRange && (ABS(max_raw) <= 1000);

    // Convert raw values into physical values based on range
    // Decimal is fixed in the first place
    float scale = BMI088_getGyroScale(bmi, range_up, range_down);

    bmi->data.gyro_x = raw_x / scale;
    bmi->data.gyro_y = raw_y / scale;
    bmi->data.gyro_z = raw_z / scale;

    // Write the CSB state to high to deselect the gyro
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    return true;
}

bool BMI088_readAccel(BMI088_Handle_t *bmi) {
    uint8_t *tx_buffer = bmi->bmi_tx_buffer;
    uint8_t *rx_buffer = bmi->bmi_rx_buffer;

    BMI088_selectAccel(bmi);
    while (PHAL_SPI_busy(bmi->spi)) {
        __asm__("nop");
    }

    tx_buffer[0] = (1 << 7) | BMI088_ACC_RATE_X_LSB_ADDR;
    PHAL_SPI_transfer_noDMA(bmi->spi, tx_buffer, 1, 7, rx_buffer);
    // while (PHAL_SPI_busy(bmi->spi))
    //     ;
    int16_t raw_ax, raw_ay, raw_az;
    raw_ax = (((int16_t)rx_buffer[2 + 1]) << 8) | rx_buffer[1 + 1];
    raw_ay = (((int16_t)rx_buffer[4 + 1]) << 8) | rx_buffer[3 + 1];
    raw_az = (((int16_t)rx_buffer[6 + 1]) << 8) | rx_buffer[5 + 1];

    // Conversion taken from datasheet pg 22.

    // NOTE - changed x and y to correspond to data sheet
    bmi->data.accel_x = (float)(raw_ax << (bmi->accel_range + 1)) / 32768.0f * G_TO_M_S * 1.5f;
    bmi->data.accel_y = (float)(raw_ay << (bmi->accel_range + 1)) / 32768.0f * G_TO_M_S * 1.5f;
    bmi->data.accel_z = (float)(raw_az << (bmi->accel_range + 1)) / 32768.0f * G_TO_M_S * 1.5f;

    // Write the CSB state to high to deselect the accelerometer
    PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);
    return true;
}

static inline void BMI088_selectGyro(BMI088_Handle_t *bmi) {
    // Write the current CSB state to high to deselect the accelerometer
    PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);

    // Set the CSB for the gyro
    bmi->spi->nss_gpio_port = SPI_CS_GYRO_GPIO_Port;
    bmi->spi->nss_gpio_pin = SPI_CS_GYRO_Pin;
}

static inline void BMI088_selectAccel(BMI088_Handle_t *bmi) {
    // Write the current CSB state to high to deselect the gyro
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);

    // Set the CSB for the accelerometer
    bmi->spi->nss_gpio_port = SPI_CS_ACEL_GPIO_Port;
    bmi->spi->nss_gpio_pin = SPI_CS_ACEL_Pin;
}