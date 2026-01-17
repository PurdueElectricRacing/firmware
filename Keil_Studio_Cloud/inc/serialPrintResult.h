/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    serialPrintResult.h
* @brief:   Print IO terminal functions
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! @addtogroup RESULT_PRINT
*  @{
*
*/

/*! @addtogroup RESULT PRINT I/O
*  @{
*
*/

#ifndef __RESULT_H
#define __RESULT_H

#include "adbms_main.h"

void printWriteConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp);
void printReadConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp);
void printVoltages(uint8_t tIC, cell_asic *IC, TYPE type);
void printStatus(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp);
void printDeviceSID(uint8_t tIC, cell_asic *IC, TYPE type);
void printWritePwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp);
void printReadPwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp);
void printWriteCommData(uint8_t tIC, cell_asic *IC, TYPE type);
void printReadCommData(uint8_t tIC, cell_asic *IC, TYPE type);
void printDiagnosticTestResult(uint8_t tIC, cell_asic *IC, DIAGNOSTIC_TYPE type);
void diagnosticTestResultPrint(uint8_t result);
void printOpenWireTestResult(uint8_t tIC, cell_asic *IC, TYPE type);
void openWireResultPrint(uint8_t result);
float getVoltage(int data);
void printPollAdcConvTime(int count);
void printMenu();

#endif 
/** @}*/
/** @}*/