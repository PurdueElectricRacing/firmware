/*
 * daq.h
 *
 *  Created on: Oct 05, 2018
 *      Author: Chris
 */

#ifndef DAQ_H_
#define DAQ_H_

#include "lsm6ds33.h"
#include "stm32l4xx_hal.h"
#include "main.h"


#define IMU_ADDR 0x422;


typedef struct
{
	CAN_HandleTypeDef *hcan;
	IMU_t imu;
	uint32_t adc;
	uint32_t tick;
} DAQ_TypeDef;

typedef enum
{
	DAQ_OK,
	IMU_ERROR,
	ACCEL_ERROR,
	GYRO_ERROR,
	ADC_ERROR,
	CAN_ERROR,
	GENERIC_ERROR,
} DAQ_Status_TypeDef;

typedef enum
{
	ACCEL = 0,
	GYRO = 1,
	IMU_TYPE_MAX,
} IMU_Data_TypeDef;



DAQ_Status_TypeDef daqInit(I2C_HandleTypeDef *hi2c, CAN_HandleTypeDef *hcan,
													 DAQ_TypeDef *daq);
DAQ_Status_TypeDef daqReadData(DAQ_TypeDef *daq);
DAQ_Status_TypeDef daqSendImuData(DAQ_TypeDef *daq, IMU_Data_TypeDef data_type);

void initCompleteFlash();

#endif /* DAQ_H_ */
