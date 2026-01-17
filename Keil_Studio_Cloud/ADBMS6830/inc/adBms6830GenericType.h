/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adBms6830GenericType.h
* @brief:   Generic Type function header file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/** @addtogroup BMS_DRIVER
*  @{
*
*/

/** @addtogroup GENERIC_TYPE GENERIC TYPE
*  @{
*
*/
#ifndef __adBmsGenericType_H
#define __adBmsGenericType_H

#include "adbms_main.h"

/* Calculates and returns the CRC15Table */
uint16_t Pec15_Calc
( 
  uint8_t len, /* Number of bytes that will be used to calculate a PEC */
  uint8_t *data /* Array of data that will be used to calculate  a PEC */                                
);                                   
uint16_t pec10_calc(bool rx_cmd, int len, uint8_t *data);

void spiSendCmd(uint8_t tx_cmd[2]);

void spiReadData
( 
uint8_t tIC, 
uint8_t tx_cmd[2], 
uint8_t *rx_data,
uint8_t *pec_error,
uint8_t *cmd_cntr,
uint8_t regData_size
);
void spiWriteData
(
  uint8_t tIC, 
  uint8_t tx_cmd[2], 
  uint8_t *data
);
void adBmsReadData(uint8_t tIC, cell_asic *ic, uint8_t cmd_arg[2], TYPE type, GRP group);
void adBmsWriteData(uint8_t tIC, cell_asic *ic, uint8_t cmd_arg[2], TYPE type, GRP group);
uint32_t adBmsPollAdc(uint8_t tx_cmd[2]);

void adBms6830_Adcv
(
  RD rd,
  CONT cont,
  DCP dcp,
  RSTF rstf,
  OW_C_S owcs
);

void adBms6830_Adsv
(
  CONT cont,
  DCP dcp,
  OW_C_S owcs
);

void adBms6830_Adax
(
OW_AUX owaux,                           
PUP pup,
CH ch
);

void adBms6830_Adax2
(
  CH ch
);

#endif
/** @}*/
/** @}*/