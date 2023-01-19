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

#define _BMI_H_
#define _BMI_H_

#include "stm32l496xx.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "bsxlite_interface.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/spi/spi.h"

#define BMM150_MAG_CHIP_ID (0x32) // Mag chip ID

#define BMM150_X_LSB_ADDR (0x42)               /* X rate LSB address */
#define BMM150_X_MSB_ADDR (0x43)               /* X rate MSB address */
#define BMM150_Y_LSB_ADDR (0x44)               /* Y rate LSB address */
#define BMM150_Y_MSB_ADDR (0x45)               /* Y rate MSB address */
#define BMM150_Z_LSB_ADDR (0x46)               /* Z rate LSB address */
#define BMM150_Z_MSB_ADDR (0x47)               /* Z rate MSB address */
#define BMM150_HALL_RESISTANCE_LSB_ADDR (0x48) /* Hall Resistance rate LSB address */
#define BMM150_HALL_RESISTANCE_MSB_ADDR (0x49) /* Hall Resistance rate MSB address */

#define BMM150_MAG_CHIP_ID_ADDR (0x40) /* Magnetometer Chip ID address */

#define BMI088_MAG_POWER_MODES_ADDR (0x4B)           /* control bits for power control, soft reset and interface SPI mode selection. Suspend Mode */
#define BMM150_MAG_OPPERATION_MODE_ADDR (0x4C)       /* Magnetomter control bits for operation mode, output data rate and self-test */
#define BMM150_MAG_INTERRUPT_SETTING_ADDR (0x4D)     /* Magetometer interrupt settings control register */
#define BMM150_MAG_REG_INTERRUPT_BIT_ADDR (0x4E)     /* Magnetometer Interrupt settings and axes enable bits control register */
#define BMM150_MAG_LOW_INTERRUPT_SETTING_ADDR (0x4F) /* Magnetometer Low-threshold interrupt threshold setting control register */
#define BMI150_MAG_RATE_XY_ADDR_ADDR (0x51)          /* Magnetometer X and Y axis repetitions control registers*/
#define BMM150_MAG_RATE_Z_ADDR (0x52)                /* Magnetometer Z axis repetitions control registers */

typedef enum
{
    ACCEL_OS_NORMAL = 0x0A,
    ACCEL_OS_2 = 0x09,
    ACCEL_OS_4 = 0x08,
} BMI088_AccelBWP_t;

typedef enum
{
    MAG_ST_NORMAL = 0x0,
    MAG_ST_RESERVED = 0x1,
    MAG_ST_NEGATIVE = 0x2,
    MAG_ST_POSITIVE = 0x3,
} BMM150_MAG_SELFTEST_t;

typedef enum
{
    BMM150_DATA_RATE_10HZ = 0x00,
    BMM150_DATA_RATE_02HZ = 0x01,
    BMM150_DATA_RATE_06HZ = 0x02,
    BMM150_DATA_RATE_08HZ = 0x03,
    BMM150_DATA_RATE_15HZ = 0x04,
    BMM150_DATA_RATE_20HZ = 0x05,
    BMM150_DATA_RATE_25HZ = 0x06,
    BMM150_DATA_RATE_30HZ = 0x07,

} BMM150_MAGODR_t;

typedef struct
{
    SPI_InitConfig_t *spi;
    GPIO_TypeDef *mag_csb_gpio_port;
    uint32_t mag_csb_pin;
    BMM150_MagRange_t mag_range;
    BMM150_MagBWP_t mag_bwp;
    BMM150_MagODR_t mag_odr;
    bool mag_ready;
} BMM150_Handle_t;

// /**
//  * @brief
//  *
//  * @param bmi
//  * @return true
//  * @return false
//  */
// bool BMI088_init(BMI088_Handle_t *bmi);

// void BMI088_powerOnAccel(BMI088_Handle_t *bmi);

// /**
//  * @brief Setup the accelerometer, must be done 50ms or longer after POR
//  *
//  * @param bmi
//  * @return true ACCEL responded sucessfully
//  * @return false not good
//  */
// bool BMI088_initAccel(BMI088_Handle_t *bmi);

// /**
//  * @brief Start the gyro self test
//  *
//  * @return true
//  * @return false
//  */
// bool BMI088_gyroSelfTestStart(BMI088_Handle_t *bmi);

// /**
//  * @brief Check the status of the most recent Gyro self test
//  *
//  * @return true Self test passed, gyro data is good
//  * @return false Self test failed, gyro data bad or no test was conduced
//  */
// bool BMI088_gyroSelfTestComplete(BMI088_Handle_t *bmi);

// /**
//  * @brief Check the status of the most recent Gyro self test
//  *
//  * @return true Self test passed, gyro data is good
//  * @return false Self test failed, gyro data bad or no test was conduced
//  */
// bool BMI088_gyroSelfTestPass(BMI088_Handle_t *bmi);

// /**
//  * @brief Blocking function to read the most recent Data Sample from the gyro
//  *
//  * @param v.x Acceleration around the X axis (pitch acceleration) in rad/s
//  * @param v.y Acceletation around the Y axis (roll acceleration) in rad/s
//  * @param v.z Acceleration around the Z axis (yaw acceleration) in rad/s
//  * @return true Successful data Tx/Rx
//  * @return false Unsuccessful data Tx/Rx
//  */
// bool BMI088_readGyro(BMI088_Handle_t *bmi, vector_3d_t *v);

// /**
//  * @brief Blocking function to read the acceleration values form the device.
//  *
//  * @param bmi
//  * @param v.x Returned acceleration over x axis (m/s^2)
//  * @param v.y Returned acceleration over y axis (m/s^2)
//  * @param v.z Returned acceleration over z axis (m/s^2)
//  * @return true
//  * @return false
//  */
// bool BMI088_readAccel(BMI088_Handle_t *bmi, vector_3d_t *v);

#endif