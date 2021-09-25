#ifndef __LSM6DS33_H__
#define __LSM6DS33_H__

#include "stm32l4xx_hal.h"
#include "main.h"
#include "stdbool.h"

#define LSM6DS33_ADDR   		0xD6

//DEFINE REGISTERS
//Device identification Register

typedef enum LSM6DS33_Addresses 
{
  FUNC_CFG_ACCESS   = 0x01,

  FIFO_CTRL1        = 0x06,
  FIFO_CTRL2        = 0x07,
  FIFO_CTRL3        = 0x08,
  FIFO_CTRL4        = 0x09,
  FIFO_CTRL5        = 0x0A,
  ORIENT_CFG_G      = 0x0B,

  INT1_CTRL         = 0x0D,
  INT2_CTRL         = 0x0E,
  WHO_AM_I          = 0x0F,
  CTRL1_XL          = 0x10,
  CTRL2_G           = 0x11,
  CTRL3_C           = 0x12,
  CTRL4_C           = 0x13,
  CTRL5_C           = 0x14,
  CTRL6_C           = 0x15,
  CTRL7_G           = 0x16,
  CTRL8_XL          = 0x17,
  CTRL9_XL          = 0x18,
  CTRL10_C          = 0x19,

  WAKE_UP_SRC       = 0x1B,
  TAP_SRC           = 0x1C,
  D6D_SRC           = 0x1D,
  STATUS_REG        = 0x1E,

  OUT_TEMP_L        = 0x20,
  OUT_TEMP_H        = 0x21,
  OUTX_L_G          = 0x22,
  OUTX_H_G          = 0x23,
  OUTY_L_G          = 0x24,
  OUTY_H_G          = 0x25,
  OUTZ_L_G          = 0x26,
  OUTZ_H_G          = 0x27,
  OUTX_L_XL         = 0x28,
  OUTX_H_XL         = 0x29,
  OUTY_L_XL         = 0x2A,
  OUTY_H_XL         = 0x2B,
  OUTZ_L_XL         = 0x2C,
  OUTZ_H_XL         = 0x2D,

  FIFO_STATUS1      = 0x3A,
  FIFO_STATUS2      = 0x3B,
  FIFO_STATUS3      = 0x3C,
  FIFO_STATUS4      = 0x3D,
  FIFO_DATA_OUT_L   = 0x3E,
  FIFO_DATA_OUT_H   = 0x3F,
  TIMESTAMP0_REG    = 0x40,
  TIMESTAMP1_REG    = 0x41,
  TIMESTAMP2_REG    = 0x42,

  STEP_TIMESTAMP_L  = 0x49,
  STEP_TIMESTAMP_H  = 0x4A,
  STEP_COUNTER_L    = 0x4B,
  STEP_COUNTER_H    = 0x4C,

  FUNC_SRC          = 0x53,

  TAP_CFG           = 0x58,
  TAP_THS_6D        = 0x59,
  INT_DUR2          = 0x5A,
  WAKE_UP_THS       = 0x5B,
  WAKE_UP_DUR       = 0x5C,
  FREE_FALL         = 0x5D,
  MD1_CFG           = 0x5E,
  MD2_CFG           = 0x5F,
};


typedef enum ACCEL_DATA_RATE_t
{
	ACCEL_DR_POWER_DOWN  = 0x00,
	ACCEL_DR_13_Hz       = 0x01,
	ACCEL_DR_26_Hz       = 0x02,
	ACCEL_DR_52_Hz       = 0x03,
	ACCEL_DR_104_Hz      = 0x04,
	ACCEL_DR_208_Hz      = 0x05,
	ACCEL_DR_416_Hz      = 0x06,
	ACCEL_DR_813_Hz      = 0x07,
	ACCEL_DR_1660_Hz     = 0x08,
	ACCEL_DR_3330_Hz     = 0x09,
	ACCEL_DR_6660_Hz     = 0x0A,
} ACCEL_DATA_RATE;


/*Accelerometer anti-alias filter bandwidth
  Used in CTRL2*/
typedef enum ACCEL_AA_FILTER_t
{
	AA_400_Hz    = 0x00,
	AA_200_Hz    = 0x01,
	AA_100_Hz    = 0x02,
	AA_50_Hz     = 0x03,
} ACCEL_AA_FILTER;

/*Acceleration full-scale selections
  Used in CTRL2
  LSB is measured in mg/LSB*/
typedef enum ACCEL_FS_t
{
	ACCEL_2G  = 0x00,   //conversion = 0.061
	ACCEL_16G = 0x01,   //conversion = 0.488
	ACCEL_4G  = 0x02,   //conversion = 0.122		DEFAULT
	ACCEL_8G  = 0x03,   //conversion = 0.244
} ACCEL_FS;



typedef enum GYRO_DATA_RATE_t
{
	GYRO_DR_POWER_DOWN  = 0x00,
	GYRO_DR_13_Hz       = 0x01,
	GYRO_DR_26_Hz       = 0x02,
	GYRO_DR_52_Hz       = 0x03,
	GYRO_DR_104_Hz      = 0x04,
	GYRO_DR_208_Hz      = 0x05,
	GYRO_DR_416_Hz      = 0x06,
	GYRO_DR_833_Hz      = 0x07,
	GYRO_DR_1660_Hz     = 0x08,
} GYRO_DATA_RATE;

//Gyro sensitivity range

typedef enum GYRO_FULL_SCALE_t
{
	FS_245_DPS	  = 0x00,   //Sensitivity: 8.75  mdps/LSB		DEFAULT
	FS_500_DPS   	= 0x01,   //Sensitivity: 17.50 mdps/LSB
	FS_1000_DPS  	= 0x02,   //Sensitivity: 35.00 mdps/LSB
	FS_2000_DPS  	= 0x03,   //Sensitivity: 70.00 mdps/LSB
} GYRO_FULL_SCALE;


typedef struct axis_data_t
{
  uint8_t high;
  uint8_t low;
} AxisData_t;



typedef struct imu_sensor_t
{
  AxisData_t x;
  AxisData_t y;
  AxisData_t z;

  float conversion;
	uint8_t broke;
} ImuSensor;

typedef struct IMU
{
  I2C_HandleTypeDef * i2c;
  ImuSensor accelerometer;
  ImuSensor gyro;
} IMU_t;


/*Initialize the gyroscope
 * I2C_HandleTypeDef *hi2c 		-- I2C pointer
 * GYRO_DATA_RATE data rate 	-- the gyro data rate, range: [100hz, 800hz]
 * GYRO_MEASUREMENT_RANGE range -- the full-scale range of the gyroscope, range: [245 degrees/sec, 2000 degrees/sec]
 * int high_pass_filter			-- enable high-pass filter, default 0 (disabled)*/


HAL_StatusTypeDef imuInit(IMU_t * imu, I2C_HandleTypeDef * hi2c);
HAL_StatusTypeDef gyroInit(IMU_t * imu, GYRO_DATA_RATE data_rate,
                           GYRO_FULL_SCALE full_scale, int high_pass_filter);
HAL_StatusTypeDef accelInit(IMU_t * imu, ACCEL_DATA_RATE dr, 
                            ACCEL_FS full_scale);
HAL_StatusTypeDef readGyro(ImuSensor * gyro, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef readAccel(ImuSensor * accel, I2C_HandleTypeDef *hi2c);


#endif
