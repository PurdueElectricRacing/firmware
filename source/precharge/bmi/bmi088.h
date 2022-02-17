/**
 * @file bmi.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "stm32l496xx.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/spi/spi.h"

#define BMI088_GYRO_CHIP_ID         (0x0FU)
#define BMI088_ACC_CHIP_ID          (0x1EU)
#define BMI088_ACC_PWR_CTRL_NORMAL  (0x04U)

#define BMI088_GYRO_CHIP_ID_ADDR    (0x00U) /* Gyro Chip ID address */
#define BMI088_GYRO_RATE_X_LSB_ADDR (0x02U) /* Gyro X rate LSB address */
#define BMI088_GYRO_RATE_Y_LSB_ADDR (0x04U) /* Gyro Y rate LSB address */
#define BMI088_GYRO_RATE_Z_LSB_ADDR (0x04U) /* Gyro Z rate LSB address */
#define BMI088_GYRO_RANGE_ADDR      (0x0FU) /* Gyro data range address */
#define BMI088_GYRO_BANDWIDTH_ADDR  (0x10U) /* Gyro data bandwidth address */
#define BMI088_GYRO_SELFTEST_ADDR   (0x3CU) /* Gyro self test address */

#define BMI088_ACC_CHIP_ID_ADDR     (0x00U) /* Accelerometer Chip ID address */
#define BMI088_ACC_STATUS_ADDR      (0x03U) /* Accelerometer status address */
#define BMI088_ACC_RATE_X_LSB_ADDR  (0x12U) /* Accelerometer X Rate LSB address */
#define BMI088_ACC_RATE_Y_LSB_ADDR  (0x14U) /* Accelerometer Y Rate LSB address */
#define BMI088_ACC_RATE_Z_LSB_ADDR  (0x16U) /* Accelerometer Z Rate LSB address */
#define BMI088_ACC_CONFIG_ADDR      (0x40U) /* Accelerometer configuration address */
#define BMI088_ACC_RANGE_ADDR       (0x41U) /* Accelerometer rate range address */
#define BMI088_ACC_PWR_CTRL_ADDR    (0x7DU) /* Accelerometer power control address */


typedef enum {
    ACC_ODR_12_5Hz  = 0x05,
    ACC_ODR_25Hz    = 0x06,
    ACC_ODR_50Hz    = 0x07,
    ACC_ODR_100Hz   = 0x08,
    ACC_ODR_200Hz   = 0x09,
    ACC_ODR_400Hz   = 0x0A,
    ACC_ODR_800Hz   = 0x0B,
    ACC_ODR_1600Hz  = 0x0C,
} BMI088_AccODR_t;

typedef enum {
    ACC_OS_NORMAL  = 0x0A,
    ACC_OS_2       = 0x09,
    ACC_OS_4       = 0x08,
} BMI088_AccBWP_t;

typedef enum {
    ACC_RANGE_3G  = 0x00,
    ACC_RANGE_6G  = 0x01,
    ACC_RANGE_12G = 0x02,
    ACC_RANGE_24G = 0x03,
} BMI088_AccRange_t;

typedef enum {
    GYRO_RANGE_2000 = 0x00,
    GYRO_RANGE_1000 = 0x01,
    GYRO_RANGE_500  = 0x02,
    GYRO_RANGE_250  = 0x03,
    GYRO_RANGE_125  = 0x04,
} BMI088_GyroRange_t;

typedef enum {
    GYRO_DR_2000Hz_532Hz = 0x00,
    GYRO_DR_2000Hz_230Hz = 0x01,
    GYRO_DR_1000Hz_116Hz = 0x02,
    GYRO_DR_400Hz_47Hz = 0x03,
    GYRO_DR_200Hz_23Hz = 0x04,
    GYRO_DR_100Hz_12Hz = 0x05,
    GYRO_DR_200Hz_64Hz = 0x06,
    GYRO_DR_100Hz_32Hz = 0x07,
} BMI088_GyroDrBw_t;

typedef struct {
    SPI_InitConfig_t*   spi;
    GPIO_TypeDef*       acc_csb_gpio_port;
    uint32_t            acc_csb_pin;
    BMI088_AccRange_t   acc_range;
    BMI088_AccBWP_t     acc_bwp;
    BMI088_AccODR_t     acc_odr;
    GPIO_TypeDef*       gyro_csb_gpio_port;
    uint32_t            gyro_csb_pin;
    BMI088_GyroRange_t  gyro_range;
    BMI088_GyroDrBw_t   gyro_datarate;
    bool                gyro_dynamic_range;
} BMI088_Handle_t;

/**
 * @brief 
 * 
 * @param bmi 
 * @return true 
 * @return false 
 */
bool BMI088_init(BMI088_Handle_t* bmi);

/**
 * @brief Start the gyro self test
 * 
 * @return true 
 * @return false 
 */
bool BMI088_gyroSelfTestStart(BMI088_Handle_t* bmi);

/**
 * @brief Check the status of the most recent Gyro self test
 * 
 * @return true Self test passed, gyro data is good
 * @return false Self test failed, gyro data bad or no test was conduced
 */
bool BMI088_gyroSelfTestComplete(BMI088_Handle_t* bmi);

/**
 * @brief Check the status of the most recent Gyro self test
 * 
 * @return true Self test passed, gyro data is good
 * @return false Self test failed, gyro data bad or no test was conduced
 */
bool BMI088_gyroSelfTestPass(BMI088_Handle_t* bmi);

/**
 * @brief Blocking function to read the most recent Data Sample from the gyro
 * 
 * @param x Acceleration around the X axis (pitch acceleration) in 0.1 deg/s
 * @param y Acceletation around the Y axis (roll acceleration) in 0.1 deg/s
 * @param z Acceleration around the Z axis (yaw acceleration) in 0.1 deg/s
 * @return true Successful data Tx/Rx
 * @return false Unsuccessful data Tx/Rx
 */
bool BMI088_readGyro(BMI088_Handle_t* bmi, int16_t* x, int16_t* y, int16_t* z);

// bool BMI088_readAccel(uint16_t* x,uint16_t* y, uint16_t* z);