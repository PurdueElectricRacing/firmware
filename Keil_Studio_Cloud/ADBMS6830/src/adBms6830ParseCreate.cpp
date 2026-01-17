/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adBms6830ParseCreate.c
* @brief:   adBms6830 ParseCreate data helper functions
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/

/*! \addtogroup BMS DRIVER
*/

/*! @addtogroup PARSE_CREATE PARSE CREATE DATA
*  @{
*/

#include "common.h"
#include "adbms_main.h"
#ifdef MBED
extern Serial pc;
#endif

/**
 *******************************************************************************
 * Function: SetOverVoltageThreshold
 * @brief Set Over Voltage Threshold.
 *
 * @details This function Set Over Voltage Threshold.
 *
 * Parameters:
 *
 * @param [in]  voltage      Over voltage
 *
 * @return OverVoltage_value
 *
 *******************************************************************************
*/
uint16_t SetOverVoltageThreshold(float voltage)
{
  uint16_t vov_value;
  uint8_t rbits = 12;
  voltage = (voltage - 1.5);
  voltage = voltage / (16 * 0.000150);
  vov_value = (uint16_t )(voltage + 2 * (1 << (rbits - 1)));
  vov_value &= 0xFFF;
  return vov_value;
}

/**
 *******************************************************************************
 * Function: SetUnderVoltageThreshold
 * @brief Set Under Voltage Threshold.
 *
 * @details This function Set Under Voltage Threshold.
 *
 * Parameters:
 *
 * @param [in]  voltage      Under voltage
 *
 * @return UnderVoltage_value
 *
 *******************************************************************************
*/
uint16_t SetUnderVoltageThreshold(float voltage)
{
  uint16_t vuv_value;
  uint8_t rbits = 12;
  voltage = (voltage - 1.5);
  voltage = voltage / (16 * 0.000150);
  vuv_value = (uint16_t )(voltage + 2 * (1 << (rbits - 1)));
  vuv_value &= 0xFFF;
  return vuv_value;
}

/**
 *******************************************************************************
 * Function: ConfigA_Flag
 * @brief Config A Flag bit.
 *
 * @details This function Set configuration A flag bit.
 *
 * Parameters:
 *
 * @param [in]  flag_d      Enum type flag bit
 *
 * @param [in]  flag       Enum type set or clr flag
 *
 * @return Flagbit_Value
 *
 *******************************************************************************
*/
uint8_t ConfigA_Flag(FLAG_D flag_d, CFGA_FLAG flag)
{
  uint8_t flag_value;
  if(flag == FLAG_SET)
  {
    flag_value = (1 << flag_d);
  }
  else
  {
    flag_value = (0 << flag_d);
  }
  return(flag_value);
}

/**
 *******************************************************************************
 * Function: ConfigA_Gpo
 * @brief Config A Gpio Pull High/Low.
 *
 * @details This function Set configuration A gpio as pull high/Low.
 *
 * Parameters:
 *
 * @param [in]  gpo      Enum type GPO Pin
 *
 * @param [in]  stat     Enum type CFGAGPO set Low or High
 *
 * @return Gpio_value
 *
 *******************************************************************************
*/
uint16_t ConfigA_Gpo(GPO gpo, CFGA_GPO stat)
{
  uint16_t gpovalue;
  if(stat == GPO_SET)
  {
    gpovalue = (1 << gpo);
  }
  else
  {
    gpovalue = (0 << gpo);
  }
  return(gpovalue);
}

/**
 *******************************************************************************
 * Function: ConfigB_DccBit
 * @brief Config B DCC bit.
 *
 * @details This function configure config B DCC bit.
 *
 * Parameters:
 *
 * @param [in]  dcc      Enum type DCC bit
 *
 * @param [in]  dccbit   Enum type DCC bit set or clr.
 *
 * @return DccBit_value
 *
 *******************************************************************************
*/
uint16_t ConfigB_DccBit(DCC dcc, DCC_BIT dccbit)
{
  uint16_t dccvalue;
  if(dccbit == DCC_BIT_SET)
  {
    dccvalue = (1 << dcc);
  }
  else
  {
    dccvalue = (0 << dcc);
  }
  return(dccvalue);
}

/**
 *******************************************************************************
 * Function: SetConfig_B_DischargeTimeOutValue
 * @brief Set Config B Discharge Time Out Value.
 *
 * @details This function Set configuration B Discharge Time Out Value with short or long range.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                      cell_asic ic structure pointer
 *
 * @param [in]  timer_rang              Enum type time range
 *
 * @param [in]  timeout_value           Enum type time out value
 *
 * @return None
 *
 *******************************************************************************
*/
void SetConfigB_DischargeTimeOutValue(uint8_t tIC, cell_asic *ic, DTRNG timer_rang, DCTO timeout_value)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].tx_cfgb.dtrng = timer_rang;
    if(timer_rang == RANG_0_TO_63_MIN)
    {
      ic[curr_ic].tx_cfgb.dcto = timeout_value;
    }
    else if(timer_rang == RANG_0_TO_16_8_HR)
    {
      ic[curr_ic].tx_cfgb.dcto = timeout_value;
    }
  }
}

/**
 *******************************************************************************
 * Function: SetPwmDutyCycle
 * @brief Set PWM discharges duty cycle.
 *
 * @details This function Set PWM discharges duty cycle.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  duty_cycle              Enum type PWM duty cycle value
 *
 * @return None
 *
 *******************************************************************************
*/
void SetPwmDutyCycle(uint8_t tIC, cell_asic *ic, PWM_DUTY duty_cycle)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    for(uint8_t pwmc = 0; pwmc < PWMA; pwmc++)
    {
     ic[curr_ic].PwmA.pwma[pwmc] = duty_cycle;
    }

    for(uint8_t pwmc = 0; pwmc < PWMB; pwmc++)
    {
     ic[curr_ic].PwmB.pwmb[pwmc] = duty_cycle;
    }
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfiga
 * @brief Parse the recived Configuration register A data
 *
 * @details This function Parse the recived Configuration register A data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                      cell_asic ic structure pointer
 *
 * @param [in]  *data                    Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfiga(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].configa.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));

    ic[curr_ic].rx_cfga.cth = (ic[curr_ic].configa.rx_data[0] & 0x07);
    ic[curr_ic].rx_cfga.refon   = (ic[curr_ic].configa.rx_data[0] & 0x80) >> 7;

    ic[curr_ic].rx_cfga.flag_d  = (ic[curr_ic].configa.rx_data[1] & 0xFF);

    ic[curr_ic].rx_cfga.soakon   = (ic[curr_ic].configa.rx_data[2] & 0x80) >> 7;
    ic[curr_ic].rx_cfga.owrng    = (((ic[curr_ic].configa.rx_data[2] & 0x40) >> 6));
    ic[curr_ic].rx_cfga.owa    = ( (ic[curr_ic].configa.rx_data[2] & 0x38) >> 3);

    ic[curr_ic].rx_cfga.gpo        = ( (ic[curr_ic].configa.rx_data[3] & 0xFF)| ((ic[curr_ic].configa.rx_data[4] & 0x03) << 8) );

    ic[curr_ic].rx_cfga.snap   = ((ic[curr_ic].configa.rx_data[5] & 0x20) >> 5);
    ic[curr_ic].rx_cfga.mute_st   = ((ic[curr_ic].configa.rx_data[5] & 0x10) >> 4);
    ic[curr_ic].rx_cfga.comm_bk   = ((ic[curr_ic].configa.rx_data[5] & 0x08) >> 3);
    ic[curr_ic].rx_cfga.fc   = ((ic[curr_ic].configa.rx_data[5] & 0x07) >> 0);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfigb
 * @brief Parse the recived Configuration register B data
 *
 * @details This function Parse the recived Configuration register B data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfigb(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].configb.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));

    ic[curr_ic].rx_cfgb.vuv = ((ic[curr_ic].configb.rx_data[0])  | ((ic[curr_ic].configb.rx_data[1] & 0x0F) << 8));
    ic[curr_ic].rx_cfgb.vov  = (ic[curr_ic].configb.rx_data[2]<<4)+((ic[curr_ic].configb.rx_data[1] &0xF0)>>4)  ;
    ic[curr_ic].rx_cfgb.dtmen = (((ic[curr_ic].configb.rx_data[3] & 0x80) >> 7));
    ic[curr_ic].rx_cfgb.dtrng= ((ic[curr_ic].configb.rx_data[3] & 0x40) >> 6);
    ic[curr_ic].rx_cfgb.dcto   = ((ic[curr_ic].configb.rx_data[3] & 0x3F));
    ic[curr_ic].rx_cfgb.dcc = ((ic[curr_ic].configb.rx_data[4]) | ((ic[curr_ic].configb.rx_data[5] & 0xFF) << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfig
 * @brief Parse the recived Configuration register A & B data
 *
 * @details This function Parse the recived Configuration register A & B data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *data                   Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfig(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  switch (grp)
  {
  case A:
    adBms6830ParseConfiga(tIC, &ic[0], &data[0]);
    break;

  case B:
    adBms6830ParseConfigb(tIC, &ic[0], &data[0]);
    break;

  default:
    break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseCell
 * @brief Parse cell voltages
 *
 * @details This function Parse the recived cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *cv_data                Cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
/* Parse cell voltages */
void adBms6830ParseCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *cv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDCVALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse cell memory \n");
    #else
    printf(" Failed to allocate parse cell memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &cv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].cell.c_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].cell.c_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].cell.c_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].cell.c_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].cell.c_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
      ic[curr_ic].cell.c_codes[15] = (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].cell.c_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].cell.c_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].cell.c_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].cell.c_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].cell.c_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].cell.c_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].cell.c_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].cell.c_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].cell.c_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].cell.c_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].cell.c_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].cell.c_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].cell.c_codes[14] = (data[28] + (data[29] << 8));
      ic[curr_ic].cell.c_codes[15] = (data[30] + (data[31] << 8));
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseAverageCell
 * @brief Parse average cell voltages
 *
 * @details This function Parse the recived average cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *acv_data               Average cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseAverageCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *acv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDACALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse avg cell memory \n");
    #else
    printf(" Failed to allocate parse avg cell memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &acv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].acell.ac_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].acell.ac_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].acell.ac_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].acell.ac_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].acell.ac_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
      ic[curr_ic].acell.ac_codes[15] = (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].acell.ac_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].acell.ac_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].acell.ac_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].acell.ac_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].acell.ac_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].acell.ac_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].acell.ac_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].acell.ac_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].acell.ac_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].acell.ac_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].acell.ac_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].acell.ac_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].acell.ac_codes[14] = (data[28] + (data[29] << 8));
      ic[curr_ic].acell.ac_codes[15] = (data[30] + (data[31] << 8));
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseSCell
 * @brief Parse S cell voltages
 *
 * @details This function Parse the recived S cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *scv_data               S cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
/* Parse S cell voltages */
void adBms6830ParseSCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *scv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDSALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse scv memory \n");
    #else
    printf(" Failed to allocate parse scv memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &scv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].scell.sc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].scell.sc_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].scell.sc_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].scell.sc_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].scell.sc_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
      ic[curr_ic].scell.sc_codes[15] = (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].scell.sc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].scell.sc_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].scell.sc_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].scell.sc_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].scell.sc_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].scell.sc_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].scell.sc_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].scell.sc_codes[9] = (data[18] + (data[19] << 8));
      ic[curr_ic].scell.sc_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].scell.sc_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].scell.sc_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].scell.sc_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].scell.sc_codes[14] = (data[28] + (data[29] << 8));
      ic[curr_ic].scell.sc_codes[15] = (data[30] + (data[31] << 8));
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseFCell
 * @brief Parse Filtered cell voltages
 *
 * @details This function Parse the recived filtered cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *fcv_data               Filtered cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseFCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *fcv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDFCALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse fcell memory \n");
    #else
    printf(" Failed to allocate parse fcell memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &fcv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].fcell.fc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].fcell.fc_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].fcell.fc_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].fcell.fc_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].fcell.fc_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
      ic[curr_ic].fcell.fc_codes[15] = (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].fcell.fc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].fcell.fc_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].fcell.fc_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].fcell.fc_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].fcell.fc_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].fcell.fc_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].fcell.fc_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].fcell.fc_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].fcell.fc_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].fcell.fc_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].fcell.fc_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].fcell.fc_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].fcell.fc_codes[14] = (data[28] + (data[29] << 8));
      ic[curr_ic].fcell.fc_codes[15] = (data[30] + (data[31] << 8));
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseAux
 * @brief Parse aux voltages
 *
 * @details This function Parse the recived aux voltages register A,B,C and D data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *aux_data               Aux voltages data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *aux_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = (RDASALL_SIZE-44);}  /* RDASALL_SIZE 68 byte - (RAUX 20 byte + STATUS 24 byte) */
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse aux memory \n");
    #else
    printf(" Failed to allocate parse aux memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &aux_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Aux Register group A */
      ic[curr_ic].aux.a_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Aux Register group B */
      ic[curr_ic].aux.a_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Aux Register group C */
      ic[curr_ic].aux.a_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Aux Register group D */
      ic[curr_ic].aux.a_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[10] =  (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[11] =  (data[4] + (data[5] << 8));
      break;

   case ALL_GRP: /* Aux Register group ALL */
      ic[curr_ic].aux.a_codes[0]  = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[1]  = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[2]  = (data[4] + (data[5] << 8));
      ic[curr_ic].aux.a_codes[3]  = (data[6] + (data[7] << 8));
      ic[curr_ic].aux.a_codes[4]  = (data[8] + (data[9] << 8));
      ic[curr_ic].aux.a_codes[5]  = (data[10] + (data[11] << 8));
      ic[curr_ic].aux.a_codes[6]  = (data[12] + (data[13] << 8));
      ic[curr_ic].aux.a_codes[7]  = (data[14] + (data[15] << 8));
      ic[curr_ic].aux.a_codes[8]  = (data[16] + (data[17] << 8));
      ic[curr_ic].aux.a_codes[9]  = (data[18] + (data[19] << 8));
      ic[curr_ic].aux.a_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].aux.a_codes[11] = (data[22] + (data[23] << 8));
     break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseRAux
 * @brief Parse raux voltages
 *
 * @details This function Parse the recived raux voltages register A,B,C and D data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *raux_data              Raux voltages data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseRAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *raux_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = (RDASALL_SIZE-48);}  /* RDASALL_SIZE 68 byte - (AUX 24 byte + STATUS 24 byte) */
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    #ifdef MBED
    pc.printf(" Failed to allocate parse raux memory \n");
    #else
    printf(" Failed to allocate parse raux memory \n");
    #endif
    exit(0);
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &raux_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* RAux Register group A */
      ic[curr_ic].raux.ra_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* RAux Register group B */
      ic[curr_ic].raux.ra_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* RAux Register group C */
      ic[curr_ic].raux.ra_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* RAux Register group D */
      ic[curr_ic].raux.ra_codes[9] =  (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* RAux Register group ALL */
      ic[curr_ic].raux.ra_codes[0]  = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[1]  = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[2]  = (data[4] + (data[5] << 8));
      ic[curr_ic].raux.ra_codes[3]  = (data[6] + (data[7] << 8));
      ic[curr_ic].raux.ra_codes[4]  = (data[8] + (data[9] << 8));
      ic[curr_ic].raux.ra_codes[5]  = (data[10] + (data[11] << 8));
      ic[curr_ic].raux.ra_codes[6]  = (data[12] + (data[13] << 8));
      ic[curr_ic].raux.ra_codes[7]  = (data[14] + (data[15] << 8));
      ic[curr_ic].raux.ra_codes[8]  = (data[16] + (data[17] << 8));
      ic[curr_ic].raux.ra_codes[9]  = (data[18] + (data[19] << 8));
     break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusA
 * @brief Parse status A register data
 *
 * @details This function Parse the recived status A register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusA(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].stata.vref2   = (ic[curr_ic].stat.rx_data[0] | (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].stata.itmp = (ic[curr_ic].stat.rx_data[2] | (ic[curr_ic].stat.rx_data[3] << 8));
    ic[curr_ic].stata.vref3   = (ic[curr_ic].stat.rx_data[4] | (ic[curr_ic].stat.rx_data[5] << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusB
 * @brief Parse status B register data
 *
 * @details This function Parse the recived status B register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusB(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].statb.vd   = (ic[curr_ic].stat.rx_data[0] + (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].statb.va = (ic[curr_ic].stat.rx_data[2] + (ic[curr_ic].stat.rx_data[3] << 8));
    ic[curr_ic].statb.vr4k   = (ic[curr_ic].stat.rx_data[4] + (ic[curr_ic].stat.rx_data[5] << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusC
 * @brief Parse status C register data
 *
 * @details This function Parse the recived status C register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusC(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].statc.cs_flt   = (ic[curr_ic].stat.rx_data[0] + (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].statc.otp2_med = (ic[curr_ic].stat.rx_data[4] & 0x01);
    ic[curr_ic].statc.otp2_ed = ((ic[curr_ic].stat.rx_data[4] & 0x02) >> 1);
    ic[curr_ic].statc.otp1_med = ((ic[curr_ic].stat.rx_data[4] & 0x04) >> 2);
    ic[curr_ic].statc.otp1_ed = ((ic[curr_ic].stat.rx_data[4] & 0x08) >> 3);
    ic[curr_ic].statc.vd_uv  = ((ic[curr_ic].stat.rx_data[4] & 0x10) >> 4);
    ic[curr_ic].statc.vd_ov = ((ic[curr_ic].stat.rx_data[4] & 0x20) >> 5);
    ic[curr_ic].statc.va_uv = ((ic[curr_ic].stat.rx_data[4] & 0x40) >> 6);
    ic[curr_ic].statc.va_ov = ((ic[curr_ic].stat.rx_data[4] & 0x80) >> 7);
    ic[curr_ic].statc.oscchk = (ic[curr_ic].stat.rx_data[5] & 0x01);
    ic[curr_ic].statc.tmodchk = ((ic[curr_ic].stat.rx_data[5] & 0x02) >> 1);
    ic[curr_ic].statc.thsd = ((ic[curr_ic].stat.rx_data[5] & 0x04) >> 2);
    ic[curr_ic].statc.sleep = ((ic[curr_ic].stat.rx_data[5] & 0x08) >> 3);
    ic[curr_ic].statc.spiflt  = ((ic[curr_ic].stat.rx_data[5] & 0x10) >> 4);
    ic[curr_ic].statc.comp = ((ic[curr_ic].stat.rx_data[5] & 0x20) >> 5);
    ic[curr_ic].statc.vdel = ((ic[curr_ic].stat.rx_data[5] & 0x40) >> 6);
    ic[curr_ic].statc.vde = ((ic[curr_ic].stat.rx_data[5] & 0x80) >> 7);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusD
 * @brief Parse status D register data
 *
 * @details This function Parse the recived status D register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusD(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    /* uv, ov bits 1 to 4 status bits */
    ic[curr_ic].statd.c_uv[0] = (ic[curr_ic].stat.rx_data[0] & 0x01);
    ic[curr_ic].statd.c_ov[0] = ((ic[curr_ic].stat.rx_data[0] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[1] = ((ic[curr_ic].stat.rx_data[0] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[1] = ((ic[curr_ic].stat.rx_data[0] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[2] = ((ic[curr_ic].stat.rx_data[0] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[2] = ((ic[curr_ic].stat.rx_data[0] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[3] = ((ic[curr_ic].stat.rx_data[0] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[3] = ((ic[curr_ic].stat.rx_data[0] & 0x80) >> 7);
    /* uv, ov bits 5 to 8 status bits */
    ic[curr_ic].statd.c_uv[4] = (ic[curr_ic].stat.rx_data[1] & 0x01);
    ic[curr_ic].statd.c_ov[4] = ((ic[curr_ic].stat.rx_data[1] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[5] = ((ic[curr_ic].stat.rx_data[1] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[5] = ((ic[curr_ic].stat.rx_data[1] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[6] = ((ic[curr_ic].stat.rx_data[1] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[6] = ((ic[curr_ic].stat.rx_data[1] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[7] = ((ic[curr_ic].stat.rx_data[1] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[7] = ((ic[curr_ic].stat.rx_data[1] & 0x80) >> 7);
    /* uv, ov bits 9 to 12 status bits */
    ic[curr_ic].statd.c_uv[8] = (ic[curr_ic].stat.rx_data[2] & 0x01);
    ic[curr_ic].statd.c_ov[8] = ((ic[curr_ic].stat.rx_data[2] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[9] = ((ic[curr_ic].stat.rx_data[2] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[9] = ((ic[curr_ic].stat.rx_data[2] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[10] = ((ic[curr_ic].stat.rx_data[2] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[10] = ((ic[curr_ic].stat.rx_data[2] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[11] = ((ic[curr_ic].stat.rx_data[2] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[11] = ((ic[curr_ic].stat.rx_data[2] & 0x80) >> 7);
    /* uv, ov bits 13 to 16 status bits */
    ic[curr_ic].statd.c_uv[12] = (ic[curr_ic].stat.rx_data[3] & 0x01);
    ic[curr_ic].statd.c_ov[12] = ((ic[curr_ic].stat.rx_data[3] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[13] = ((ic[curr_ic].stat.rx_data[3] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[13] = ((ic[curr_ic].stat.rx_data[3] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[14] = ((ic[curr_ic].stat.rx_data[3] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[14] = ((ic[curr_ic].stat.rx_data[3] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[15] = ((ic[curr_ic].stat.rx_data[3] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[15] = ((ic[curr_ic].stat.rx_data[3] & 0x80) >> 7);
    /* ct and cts */
    ic[curr_ic].statd.cts = (ic[curr_ic].stat.rx_data[4] & 0x03);
    ic[curr_ic].statd.ct = ((ic[curr_ic].stat.rx_data[4] & 0xFC) >> 2);
    /* oc_cntr */
    ic[curr_ic].statd.oc_cntr = (ic[curr_ic].stat.rx_data[5] & 0xFF);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusE
 * @brief Parse status E register data
 *
 * @details This function Parse the recived status E register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusE(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].state.gpi   = ((ic[curr_ic].stat.rx_data[4] + ((ic[curr_ic].stat.rx_data[5] & 0x03) << 8)));
    ic[curr_ic].state.rev = ((ic[curr_ic].stat.rx_data[5] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatus
 * @brief Parse status register data
 *
 * @details This function Parse the recived status register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatus(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  uint8_t statc[RX_DATA], state[RX_DATA];
  switch (grp)
  {
    case A: /* Status Register group A */
      adBms6830ParseStatusA(tIC, &ic[0], &data[0]);
      break;

    case B: /* Status Register group B */
      adBms6830ParseStatusB(tIC, &ic[0], &data[0]);
      break;

    case C: /* Status Register group C */
      adBms6830ParseStatusC(tIC, &ic[0], &data[0]);
      break;

    case D: /* Status Register group D */
      adBms6830ParseStatusD(tIC, &ic[0], &data[0]);
      break;

    case E: /* Status Register group E */
      adBms6830ParseStatusE(tIC, &ic[0], &data[0]);
      break;

    case ALL_GRP: /* Status Register group ALL */
      /* Status A base address data[0] index */
      adBms6830ParseStatusA(tIC, &ic[0], &data[0]);
      /* Status B base address data[6] index */
      adBms6830ParseStatusB(tIC, &ic[0], &data[6]);
      /* Status C base address data[12] index */
      statc[0] = data[12];
      statc[1] = data[13];
      statc[4] = data[14];
      statc[5] = data[15];
      adBms6830ParseStatusC(tIC, &ic[0], &statc[0]);
      /* Status D base address data[16] index */
      adBms6830ParseStatusD(tIC, &ic[0], &data[16]);
      /* Status E base address data[22] index */
      state[4] = data[22];
      state[5] = data[23];
      adBms6830ParseStatusE(tIC, &ic[0], &state[0]);
     break;

    default:
      break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseComm
 * @brief Parse comm register
 *
 * @details This function Parse the recived comm register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseComm(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].com.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].comm.icomm[0] = ((ic[curr_ic].com.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].comm.fcomm[0] = (ic[curr_ic].com.rx_data[0] & 0x0F);
    ic[curr_ic].comm.data[0] = (ic[curr_ic].com.rx_data[1]);
    ic[curr_ic].comm.icomm[1] = ((ic[curr_ic].com.rx_data[2] & 0xF0) >> 4);
    ic[curr_ic].comm.data[1] = (ic[curr_ic].com.rx_data[3]);
    ic[curr_ic].comm.fcomm[1] = (ic[curr_ic].com.rx_data[2] & 0x0F);
    ic[curr_ic].comm.icomm[2] = ((ic[curr_ic].com.rx_data[4] & 0xF0) >> 4);
    ic[curr_ic].comm.data[2] = (ic[curr_ic].com.rx_data[5]);
    ic[curr_ic].comm.fcomm[2] = (ic[curr_ic].com.rx_data[4] & 0x0F);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseSID
 * @brief Parse SID register
 *
 * @details This function Parse the recived sid register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseSID(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].rsid.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].sid.sid[0] = ic[curr_ic].rsid.rx_data[0];
    ic[curr_ic].sid.sid[1] = ic[curr_ic].rsid.rx_data[1];
    ic[curr_ic].sid.sid[2] = ic[curr_ic].rsid.rx_data[2];
    ic[curr_ic].sid.sid[3] = ic[curr_ic].rsid.rx_data[3];
    ic[curr_ic].sid.sid[4] = ic[curr_ic].rsid.rx_data[4];
    ic[curr_ic].sid.sid[5] = ic[curr_ic].rsid.rx_data[5];
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParsePwma
 * @brief Parse PWMA register
 *
 * @details This function Parse the recived pwma register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwma(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].pwma.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].PwmA.pwma[0] = (ic[curr_ic].pwma.rx_data[0] & 0x0F);
    ic[curr_ic].PwmA.pwma[1] = ((ic[curr_ic].pwma.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[2] = (ic[curr_ic].pwma.rx_data[1] & 0x0F);
    ic[curr_ic].PwmA.pwma[3] = ((ic[curr_ic].pwma.rx_data[1] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[4] = (ic[curr_ic].pwma.rx_data[2] & 0x0F);
    ic[curr_ic].PwmA.pwma[5] = ((ic[curr_ic].pwma.rx_data[2] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[6] = (ic[curr_ic].pwma.rx_data[3] & 0x0F);
    ic[curr_ic].PwmA.pwma[7] = ((ic[curr_ic].pwma.rx_data[3] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[8] = (ic[curr_ic].pwma.rx_data[4] & 0x0F);
    ic[curr_ic].PwmA.pwma[9] = ((ic[curr_ic].pwma.rx_data[4] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[10] = (ic[curr_ic].pwma.rx_data[5] & 0x0F);
    ic[curr_ic].PwmA.pwma[11] = ((ic[curr_ic].pwma.rx_data[5] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParsePwmb
 * @brief Parse PWMB register
 *
 * @details This function Parse the recived pwmb register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwmb(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].pwmb.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].PwmB.pwmb[0] = (ic[curr_ic].pwmb.rx_data[0] & 0x0F);
    ic[curr_ic].PwmB.pwmb[1] = ((ic[curr_ic].pwmb.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].PwmB.pwmb[2] = (ic[curr_ic].pwmb.rx_data[1] & 0x0F);
    ic[curr_ic].PwmB.pwmb[3] = ((ic[curr_ic].pwmb.rx_data[1] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParsePwm
 * @brief Parse PWM register
 *
 * @details This function Parse the recived pwm register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwm(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  switch (grp)
  {
    case A:
      adBms6830ParsePwma(tIC, &ic[0], &data[0]);
      break;

    case B:
      adBms6830ParsePwmb(tIC, &ic[0], &data[0]);
      break;

    default:
      break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateConfiga
 * @brief Create the configation A write buffer
 *
 * @details This function create the configation A write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateConfiga(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].configa.tx_data[0] = (((ic[curr_ic].tx_cfga.refon & 0x01) << 7) | (ic[curr_ic].tx_cfga.cth & 0x07));
    ic[curr_ic].configa.tx_data[1] = (ic[curr_ic].tx_cfga.flag_d & 0xFF);
    ic[curr_ic].configa.tx_data[2] = (((ic[curr_ic].tx_cfga.soakon & 0x01) << 7) | ((ic[curr_ic].tx_cfga.owrng & 0x01) << 6) | ((ic[curr_ic].tx_cfga.owa & 0x07) << 3));
    ic[curr_ic].configa.tx_data[3] = ((ic[curr_ic].tx_cfga.gpo & 0x00FF));
    ic[curr_ic].configa.tx_data[4] = ((ic[curr_ic].tx_cfga.gpo & 0x0300)>>8);
    ic[curr_ic].configa.tx_data[5] = (((ic[curr_ic].tx_cfga.snap & 0x01) << 5) | ((ic[curr_ic].tx_cfga.mute_st & 0x01) << 4) | ((ic[curr_ic].tx_cfga.comm_bk & 0x01) << 3) | (ic[curr_ic].tx_cfga.fc & 0x07));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateConfigb
 * @brief Create the configation B write buffer
 *
 * @details This function create the configation B write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateConfigb(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].configb.tx_data[0] = ((ic[curr_ic].tx_cfgb.vuv ));
    ic[curr_ic].configb.tx_data[1] = (((ic[curr_ic].tx_cfgb.vov & 0x000F) << 4) | ((ic[curr_ic].tx_cfgb.vuv ) >> 8));
    ic[curr_ic].configb.tx_data[2] = ((ic[curr_ic].tx_cfgb.vov >>4)&0x0FF);
    ic[curr_ic].configb.tx_data[3] = (((ic[curr_ic].tx_cfgb.dtmen & 0x01) << 7) | ((ic[curr_ic].tx_cfgb.dtrng & 0x01) << 6) | ((ic[curr_ic].tx_cfgb.dcto & 0x3F) << 0));
    ic[curr_ic].configb.tx_data[4] = ((ic[curr_ic].tx_cfgb.dcc & 0xFF));
    ic[curr_ic].configb.tx_data[5] = ((ic[curr_ic].tx_cfgb.dcc >>8 ));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateClrflagData
 * @brief Create the clear flag write buffer
 *
 * @details This function create the clear flag write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateClrflagData(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].clrflag.tx_data[0] = (ic[curr_ic].clflag.cl_csflt & 0x00FF);
    ic[curr_ic].clrflag.tx_data[1] = ((ic[curr_ic].clflag.cl_csflt & 0xFF00) >> 8);
    ic[curr_ic].clrflag.tx_data[2] = 0x00;
    ic[curr_ic].clrflag.tx_data[3] = 0x00;
    ic[curr_ic].clrflag.tx_data[4] = ((ic[curr_ic].clflag.cl_vaov << 7) | (ic[curr_ic].clflag.cl_vauv << 6) | (ic[curr_ic].clflag.cl_vdov << 5) | (ic[curr_ic].clflag.cl_vduv << 4)
                                      |(ic[curr_ic].clflag.cl_ced << 3)| (ic[curr_ic].clflag.cl_cmed << 2) | (ic[curr_ic].clflag.cl_sed << 1) | (ic[curr_ic].clflag.cl_smed));
    ic[curr_ic].clrflag.tx_data[5] = ((ic[curr_ic].clflag.cl_vde << 7) | (ic[curr_ic].clflag.cl_vdel << 6) | (ic[curr_ic].clflag.cl_spiflt << 4) |(ic[curr_ic].clflag.cl_sleep << 3)
                                      | (ic[curr_ic].clflag.cl_thsd << 2) | (ic[curr_ic].clflag.cl_tmode << 1) | (ic[curr_ic].clflag.cl_oscchk));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateComm
 * @brief Create the configation comm write buffer
 *
 * @details This function create the configation comm write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateComm(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].com.tx_data[0] = ((ic[curr_ic].comm.icomm[0] & 0x0F)  << 4  | (ic[curr_ic].comm.fcomm[0]   & 0x0F));
    ic[curr_ic].com.tx_data[1] = ((ic[curr_ic].comm.data[0] ));
    ic[curr_ic].com.tx_data[2] = ((ic[curr_ic].comm.icomm[1] & 0x0F)  << 4 ) | (ic[curr_ic].comm.fcomm[1]   & 0x0F);
    ic[curr_ic].com.tx_data[3] = ((ic[curr_ic].comm.data[1]));
    ic[curr_ic].com.tx_data[4] = ((ic[curr_ic].comm.icomm[2] & 0x0F)  << 4  | (ic[curr_ic].comm.fcomm[2]   & 0x0F));
    ic[curr_ic].com.tx_data[5] = ((ic[curr_ic].comm.data[2]));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreatePwma
 * @brief Create the configation pwma write buffer
 *
 * @details This function create the configation pwma write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreatePwma(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].pwma.tx_data[0] = ((ic[curr_ic].PwmA.pwma[1] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[0] & 0x0F));
    ic[curr_ic].pwma.tx_data[1] = ((ic[curr_ic].PwmA.pwma[3] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[2] & 0x0F));
    ic[curr_ic].pwma.tx_data[2] = ((ic[curr_ic].PwmA.pwma[5] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[4] & 0x0F));
    ic[curr_ic].pwma.tx_data[3] = ((ic[curr_ic].PwmA.pwma[7] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[6] & 0x0F));
    ic[curr_ic].pwma.tx_data[4] = ((ic[curr_ic].PwmA.pwma[9] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[8] & 0x0F));
    ic[curr_ic].pwma.tx_data[5] = ((ic[curr_ic].PwmA.pwma[11] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[10] & 0x0F));
  }
}
/**
 *******************************************************************************
 * Function: adBms6830CreatePwmb
 * @brief Create the configation pwmb write buffer
 *
 * @details This function create the configation pwmb write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreatePwmb(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].pwmb.tx_data[0] = ((ic[curr_ic].PwmB.pwmb[1] & 0x0F) << 4 | (ic[curr_ic].PwmB.pwmb[0] & 0x0F));
    ic[curr_ic].pwmb.tx_data[1] = ((ic[curr_ic].PwmB.pwmb[3] & 0x0F) << 4 | (ic[curr_ic].PwmB.pwmb[2] & 0x0F));
  }
}
/** @}*/
/** @}*/