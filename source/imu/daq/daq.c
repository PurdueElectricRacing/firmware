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


DAQ_Status_TypeDef daqSendImuData(DAQ_TypeDef *daq, IMU_Data_TypeDef data_type)
{

  IMU_t * imu = &(daq->imu);
  ImuSensor * accel = &(imu->accelerometer);
  ImuSensor * gyro = &(imu->gyro);

	uint32_t mailbox;

	uint8_t data[8];
	data[6] = 0;
	data[7] = 0;

	CAN_TxHeaderTypeDef header;
	header.StdId = IMU_ADDR;
	header.IDE= CAN_ID_STD;
	header.RTR = CAN_RTR_DATA;
	header.DLC = 8;
	header.TransmitGlobalTime = DISABLE;


	data[0] = data_type;

	//determine which data values to get based on the data type
	switch (data_type) {
		case (ACCEL):
				if (!accel->broke)
				{
					data[1] = accel->x.low;
					data[2] = accel->x.high;
					data[3] = accel->y.low;
					data[4] = accel->y.high;
					data[5] = accel->z.low;
					data[6] = accel->z.high;
					break;
				}
				else
				{
					return ACCEL_ERROR;
				}
		case (GYRO):
				if (!gyro->broke)
				{
					data[1] = gyro->x.low;
					data[2] = gyro->x.high;
					data[3] = gyro->y.low;
					data[4] = gyro->y.high;
					data[5] = gyro->z.low;
					data[6] = gyro->z.high;
					break;
				}
				else
				{
					return GYRO_ERROR;
				}
		default:
		  return GENERIC_ERROR;
	}

	while (HAL_CAN_GetTxMailboxesFreeLevel(daq->hcan) == 0); // while mailboxes not free

	if (HAL_CAN_AddTxMessage(daq->hcan, &header, data, &mailbox) != HAL_OK)
	{
		return CAN_ERROR;
	}

	return DAQ_OK;

}
