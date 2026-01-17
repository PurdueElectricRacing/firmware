/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    mcuWrapper.h
* @brief:   Generic wrapper header file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/** @addtogroup MUC_DRIVER
*  @{
*
*/

/** @addtogroup SPI_DRIVER SPI DRIVER
*  @{
*
*/
#ifndef __ADBMSWRAPPER_H
#define __ADBMSWRAPPER_H
#include "common.h"

#ifdef MBED

#else
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_it.h"

extern ADC_HandleTypeDef hadc1;         /* Mcu dependent ADC handler */
extern I2C_HandleTypeDef hi2c1;         /* Mcu dependent I2C handler */
extern I2C_HandleTypeDef hi2c3;         /* Mcu dependent I2C handler */
extern SPI_HandleTypeDef hspi1;         /* Mcu dependent SPI handler */
extern SPI_HandleTypeDef hspi5;         /* Mcu dependent SPI handler */
extern UART_HandleTypeDef huart4;       /* Mcu dependent UART handler */
extern UART_HandleTypeDef huart5;       /* Mcu dependent UART handler */
extern TIM_HandleTypeDef htim2;         /* Mcu dependent TIM handler */
extern TIM_HandleTypeDef htim5;         /* Mcu dependent TIM handler */

#define CS_PIN ARDUINO_GPIO10_Pin               /* Mcu dependent chip select */
#define GPIO_PORT ARDUINO_GPIO10_GPIO_Port      /* Mcu dependent adc chip select port */
#endif

void Delay_ms(uint32_t delay);
void adBmsCsLow(void);
void adBmsCsHigh(void);
void spiWriteBytes
( 
  uint16_t size,                                /*Option: Number of bytes to be written on the SPI port*/
  uint8_t *tx_Data                              /*Array of bytes to be written on the SPI port*/
);
void spiWriteReadBytes
(
  uint8_t *tx_data,                             /*array of data to be written on SPI port*/
  uint8_t *rx_data,                             /*Input: array that will store the data read by the SPI port*/
  uint16_t size                             /*Option: number of bytes*/
);
void spiReadBytes(uint16_t size, uint8_t *rx_data);
void startTimer(void);
void stopTimer(void);
uint32_t getTimCount(void);
void adBmsWakeupIc(uint8_t total_ic);

#endif
/** @}*/
/** @}*/