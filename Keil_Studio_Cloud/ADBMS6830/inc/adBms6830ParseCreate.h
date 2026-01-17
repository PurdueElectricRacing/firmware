/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adBms6830ParseCreate.h
* @brief:   Data parse create helper function header file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/** @addtogroup BMS_DRIVER
*  @{
*
*/

/** @addtogroup PARSE_CREATE PARSE CREATE DATA
*  @{
*
*/
#ifndef __ADBMSPARSECREATE_H
#define __ADBMSPARSECREATE_H

#include "common.h"
#include "adBms6830Data.h"

uint16_t SetOverVoltageThreshold(float volt);
uint16_t SetUnderVoltageThreshold(float voltage);
uint8_t ConfigA_Flag(FLAG_D flag_d, CFGA_FLAG flag);
uint16_t ConfigA_Gpo(GPO gpo, CFGA_GPO stat);
uint16_t ConfigB_DccBit(DCC dcc, DCC_BIT dccbit);
void SetConfigB_DischargeTimeOutValue(uint8_t tIC, cell_asic *ic, DTRNG timer_rang, DCTO timeout_value);
void SetPwmDutyCycle(uint8_t tIC, cell_asic *ic, PWM_DUTY duty_cycle);
void adBms6830ParseConfiga(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseConfigb(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseConfig(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data);
void adBms6830ParseCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *cv_data);
void adBms6830ParseAverageCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *acv_data);
void adBms6830ParseSCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *scv_data);
void adBms6830ParseFCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *fcv_data);
void adBms6830ParseAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *aux_data);
void adBms6830ParseRAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *raux_data);
void adBms6830ParseStatusA(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseStatusB(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseStatusC(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseStatusD(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseStatusE(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParseStatus(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data);
void adBms6830ParseComm(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParsePwma(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParsePwmb(uint8_t tIC, cell_asic *ic, uint8_t *data);
void adBms6830ParsePwm(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data);
void adBms6830CreateConfiga(uint8_t tIC, cell_asic *ic);
void adBms6830CreateConfigb(uint8_t tIC, cell_asic *ic);
void adBms6830CreateClrflagData(uint8_t tIC, cell_asic *ic);
void adBms6830CreateComm(uint8_t tIC, cell_asic *ic);
void adBms6830CreatePwma(uint8_t tIC, cell_asic *ic);
void adBms6830CreatePwmb(uint8_t tIC, cell_asic *ic);
void adBms6830ParseSID(uint8_t tIC, cell_asic *ic, uint8_t *data);

#endif
/** @}*/
/** @}*/