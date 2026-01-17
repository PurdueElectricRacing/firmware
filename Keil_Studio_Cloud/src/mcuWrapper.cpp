/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    mcuWrapper.c
* @brief:   BMS SPI driver functions
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! \addtogroup MCU DRIVER
*  @{
*/

/*! @addtogroup Mcu Driver
*  @{
*/
#include "common.h"
#include "mcuWrapper.h"
#define WAKEUP_DELAY 4                          /* BMS ic wakeup delay  */

#ifdef MBED
extern Serial pc;
extern SPI spi;
extern Timer timer;
extern DigitalOut chip_select;

/**
 *******************************************************************************
 * Function: Delay_ms
 * @brief Delay mili second
 *
 * @details This function insert delay in ms.
 *     
 * Parameters:
 * @param [in]  delay   Delay_ms
 *
 * @return None
 *
 *******************************************************************************
*/
void Delay_ms(uint32_t delay)
{
  wait_ms((int)delay);
}

/**
 *******************************************************************************
 * Function: adBmsCsLow
 * @brief Select chip select low
 *
 * @details This function does spi chip select low.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsLow()
{
  spi.lock();
  chip_select = 0;
}

/**
 *******************************************************************************
 * Function: adBmsCsHigh
 * @brief Select chip select High
 *
 * @details This function does spi chip select high.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsHigh()
{
  chip_select = 1;
  spi.unlock();
}

/**
 *******************************************************************************
 * Function: spiWriteBytes
 * @brief Writes an array of bytes out of the SPI port.
 *
 * @details This function wakeup bms ic in IsoSpi mode send dumy byte data in spi line..
 *
 * @param [in]  size            Numberof bytes to be send on the SPI line
 *
 * @param [in]  *tx_Data    Tx data pointer 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteBytes
( 
uint16_t size,                     /*Option: Number of bytes to be written on the SPI port*/
uint8_t *tx_data                       /*Array of bytes to be written on the SPI port*/
)
{
  uint8_t rx_data[size];
  spi.write((char *)tx_data, size ,(char *)rx_data, size);
}

/**
 *******************************************************************************
 * Function: spiWriteReadBytes
 * @brief Writes and read a set number of bytes using the SPI port.
 *
 * @details This function writes and read a set number of bytes using the SPI port.
 *
 * @param [in]  *tx_data    Tx data pointer
 *
 * @param [in]  *rx_data    Rx data pointer 
 *
 * @param [in]  size            Data size 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteReadBytes
(
uint8_t *tx_data,                   /*array of data to be written on SPI port*/
uint8_t *rx_data,                   /*Input: array that will store the data read by the SPI port*/
uint16_t size                           /*Option: number of bytes*/
)
{  
  uint16_t data_size = (4 + size);
  uint8_t cmd[data_size];
  memcpy(&cmd[0], &tx_data[0], 4); /* dst, src, size */
  spi.write((char *)cmd, data_size ,(char *)cmd, data_size);
  memcpy(&rx_data[0], &cmd[4], size); /* dst, src, size */
}

/**
 *******************************************************************************
 * Function: spiReadBytes
 * @brief Read number of bytes using the SPI port.
 *
 * @details This function Read a set number of bytes using the SPI port.
 *
 * @param [in]  size            Data size 
 *
 * @param [in]  *rx_data    Rx data pointer
 * 
 * @return None
 *
 *******************************************************************************
*/
void spiReadBytes(uint16_t size, uint8_t *rx_data)
{   
  uint8_t tx_data[size];
  for(uint8_t i=0; i < size; i++)
  {
    tx_data[i] = 0xFF;
  }
  spi.write((char *)tx_data, size ,(char *)rx_data, size);
}

/**
 *******************************************************************************
 * Function: startTimer()
 * @brief Start timer 
 *
 * @details This function start the timer.
 *
 * @return None
 *
 *******************************************************************************
*/
void startTimer()
{   
  timer.start();
}

/**
 *******************************************************************************
 * Function: stopTimer()
 * @brief Stop timer 
 *
 * @details This function stop the timer.
 *
 * @return None
 *
 *******************************************************************************
*/
void stopTimer()
{   
  timer.stop();
}

/**
 *******************************************************************************
 * Function: getTimCount()
 * @brief Get Timer Count Value 
 *
 * @details This function return the timer count value.
 *
 * @return tim_count
 *
 *******************************************************************************
*/
uint32_t getTimCount()
{   
  uint32_t count = 0;
  count = timer.read_us();
  timer.reset();
  return(count);
}
#else

#define SPI_TIME_OUT HAL_MAX_DELAY              /* SPI Time out delay   */
#define UART_TIME_OUT HAL_MAX_DELAY             /* UART Time out delay  */
#define I2C_TIME_OUT HAL_MAX_DELAY              /* I2C Time out delay   */

SPI_HandleTypeDef *hspi         = &hspi1;       /* MUC SPI Handler      */
UART_HandleTypeDef *huart       = &huart5;      /* MUC UART Handler     */
I2C_HandleTypeDef *hi2c         = &hi2c1;       /* MUC I2C Handler      */
TIM_HandleTypeDef *htim         = &htim2;       /* Mcu TIM handler */


/**
 *******************************************************************************
 * Function: Delay_ms
 * @brief Delay mili second
 *
 * @details This function insert delay in ms.
 *     
 * Parameters:
 * @param [in]  delay   Delay_ms
 *
 * @return None
 *
 *******************************************************************************
*/
void Delay_ms(uint32_t delay)
{
  HAL_Delay(delay);
}

/**
 *******************************************************************************
 * Function: adBmsCsLow
 * @brief Select chip select low
 *
 * @details This function does spi chip select low.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsLow()
{
  HAL_GPIO_WritePin(GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
}

/**
 *******************************************************************************
 * Function: adBmsCsHigh
 * @brief Select chip select High
 *
 * @details This function does spi chip select high.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsHigh()
{
  HAL_GPIO_WritePin(GPIO_PORT, CS_PIN, GPIO_PIN_SET);
}

/**
 *******************************************************************************
 * Function: spiWriteBytes
 * @brief Writes an array of bytes out of the SPI port.
 *
 * @details This function wakeup bms ic in IsoSpi mode send dumy byte data in spi line..
 *
 * @param [in]  size            Numberof bytes to be send on the SPI line
 *
 * @param [in]  *tx_Data    Tx data pointer 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteBytes
( 
uint16_t size,                     /*Option: Number of bytes to be written on the SPI port*/
uint8_t *tx_Data                       /*Array of bytes to be written on the SPI port*/
)
{
  HAL_SPI_Transmit(hspi, tx_Data, size, SPI_TIME_OUT); /* SPI1 , data, size, timeout */ 
}

/**
 *******************************************************************************
 * Function: spiWriteReadBytes
 * @brief Writes and read a set number of bytes using the SPI port.
 *
 * @details This function writes and read a set number of bytes using the SPI port.
 *
 * @param [in]  *tx_data    Tx data pointer
 *
 * @param [in]  *rx_data    Rx data pointer 
 *
 * @param [in]  size            Data size 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteReadBytes
(
uint8_t *tx_data,                   /*array of data to be written on SPI port*/
uint8_t *rx_data,                   /*Input: array that will store the data read by the SPI port*/
uint16_t size                           /*Option: number of bytes*/
)
{
  HAL_SPI_Transmit(hspi, tx_data, 4, SPI_TIME_OUT);
  HAL_SPI_Receive(hspi, rx_data, size, SPI_TIME_OUT);
}

/**
 *******************************************************************************
 * Function: spiReadBytes
 * @brief Read number of bytes using the SPI port.
 *
 * @details This function Read a set number of bytes using the SPI port.
 *
 * @param [in]  size            Data size 
 *
 * @param [in]  *rx_data    Rx data pointer
 * 
 * @return None
 *
 *******************************************************************************
*/
void spiReadBytes(uint16_t size, uint8_t *rx_data)
{   
  HAL_SPI_Receive(hspi, rx_data, size, SPI_TIME_OUT);
}

/**
 *******************************************************************************
 * Function: startTimer()
 * @brief Start timer 
 *
 * @details This function start the timer.
 *
 * @return None
 *
 *******************************************************************************
*/
void startTimer()
{   
  HAL_TIM_Base_Start(htim);
}

/**
 *******************************************************************************
 * Function: stopTimer()
 * @brief Stop timer 
 *
 * @details This function stop the timer.
 *
 * @return None
 *
 *******************************************************************************
*/
void stopTimer()
{   
  HAL_TIM_Base_Stop(htim);
}

/**
 *******************************************************************************
 * Function: getTimCount()
 * @brief Get Timer Count Value 
 *
 * @details This function return the timer count value.
 *
 * @return tim_count
 *
 *******************************************************************************
*/
uint32_t getTimCount()
{   
  uint32_t count = 0;
  count = __HAL_TIM_GetCounter(htim);
  __HAL_TIM_SetCounter(htim, 0);
  return(count);
}

#endif

/**
 *******************************************************************************
 * Function: adBmsWakeupIc
 * @brief Wakeup bms ic using chip select
 *
 * @details This function wakeup thr bms ic using chip select.
 *
 * @param [in]  total_ic    Total_ic
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsWakeupIc(uint8_t total_ic)
{
  for (uint8_t ic = 0; ic < total_ic; ic++)
  {
    adBmsCsLow();
    Delay_ms(WAKEUP_DELAY);
    adBmsCsHigh();
    Delay_ms(WAKEUP_DELAY);
  }
}

/** @}*/
/** @}*/