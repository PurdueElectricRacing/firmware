/*
 * daq.c

 *
 *  Created on: Sep 29, 2018
 *      Author: Chris
 *
 *  Edited on: Jan 25, 2019
 *  		Editor: Chris
 * 
 * lol i'm not doing this you can look at the git history
 */


#include "daq.h"
#include "can_parse.h"
#include "common/phal_L4/can/can.h"

void initCompleteFlash()
{
  HAL_Delay(500);
  for (int i = 0; i < 6; i++)
  {
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    HAL_Delay(150);
  }
}


DAQ_Status_TypeDef daqInit(I2C_HandleTypeDef *hi2c, CAN_HandleTypeDef *hcan,
														DAQ_TypeDef *daq)
{
	daq->hcan = hcan;

	if (imuInit(&(daq->imu), hi2c) != HAL_OK)
	{
		return IMU_ERROR;
	}

	//Start CAN, return CAN error if failed
	if (HAL_CAN_Start(daq->hcan))
	{
		return CAN_ERROR;
	}

	//all went well, return OK
	return DAQ_OK;
}


DAQ_Status_TypeDef daqReadData(DAQ_TypeDef *daq)
{
  IMU_t * imu = &(daq->imu);
  ImuSensor * accel = &(imu->accelerometer);
  ImuSensor * gyro = &(imu->gyro);
  I2C_HandleTypeDef * hi2c = imu->i2c;

	//read the data from accelerometer, return error if failed
	if (readAccel(accel, hi2c) != HAL_OK)
	{
		return ACCEL_ERROR;
	}

	//read the data from gyro , return error if failed
	if (readGyro(gyro, hi2c) != HAL_OK)
	{
		return GYRO_ERROR;
	}


	return DAQ_OK;
}
int16_t gyro_x;
int16_t gyro_y;
int16_t gyro_z;
int16_t accel_x;
int16_t accel_y;
int16_t accel_z;

DAQ_Status_TypeDef daqSendImuData(DAQ_TypeDef *daq, IMU_Data_TypeDef data_type)
{
	IMU_t * imu = &(daq->imu);
  	ImuSensor * accel = &(imu->accelerometer);
  	ImuSensor * gyro = &(imu->gyro);

	gyro_x = gyro->x.high<<8 | gyro->x.low;
	gyro_y = gyro->y.high<<8 | gyro->y.low;
	gyro_z = gyro->z.high<<8 | gyro->z.low;
	accel_x = accel->x.high<<8 | accel->x.low;
	accel_y = accel->y.high<<8 | accel->y.low;
	accel_z = accel->z.high<<8 | accel->z.low;

	CanMsgTypeDef_t msg1 = {.ExtId=ID_ACCEL_DATA, .DLC=DLC_ACCEL_DATA, .IDE=1};
	CanParsedData_t* data_a = (CanParsedData_t *) &msg1.Data;
	data_a->accel_data.accel_x = accel_x;
	data_a->accel_data.accel_y = accel_y;
	data_a->accel_data.accel_z = accel_z;
	PHAL_txCANMessage(&msg1);

	CanMsgTypeDef_t msg2 = {.ExtId=ID_GYRO_DATA, .DLC=DLC_GYRO_DATA, .IDE=1};
	CanParsedData_t* data_b = (CanParsedData_t *) &msg2.Data;
	data_b->gyro_data.gyro_x = gyro_x;
	data_b->gyro_data.gyro_y = gyro_y;
	data_b->gyro_data.gyro_z = gyro_z;
	PHAL_txCANMessage(&msg2);
	
	return DAQ_OK;

}



DAQ_Status_TypeDef yawRateGetter(DAQ_TypeDef *daq)
{
	IMU_t * imu = &(daq->imu);
	ImuSensor * accel = &(imu->accelerometer);
  	ImuSensor * gyro = &(imu->gyro);
	gyro_x = gyro->x.high<<8 | gyro->x.low;
	gyro_y = gyro->y.high<<8 | gyro->y.low;
	gyro_z = gyro->z.high<<8 | gyro->z.low;
	accel_x = accel->x.high<<8 | accel->x.low;
	accel_y = accel->y.high<<8 | accel->y.low;
	accel_z = accel->z.high<<8 | accel->z.low;
	return DAQ_OK;
}