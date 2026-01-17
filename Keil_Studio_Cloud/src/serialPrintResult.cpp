/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    serialPrintResult.c
* @brief:   Print IO terminal functions
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! \addtogroup PRINT RESULT
*  @{
*/

/*! @addtogroup RESULT PRINT
*  @{
*/
#include "common.h"
#include "serialPrintResult.h"

#ifdef MBED
extern Serial pc;
/**
 *******************************************************************************
 * Function: printWriteConfig
 * @brief Print write config A/B result.
 *
 * @details This function Print write config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printWriteConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        pc.printf("Write Config A:\n");
        pc.printf("0x%X, ", IC[ic].configa.tx_data[0]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[1]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[2]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[3]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[4]);
        pc.printf("0x%X\n\n", IC[ic].configa.tx_data[5]);
      }
      else if(grp == B)
      {
        pc.printf("Write Config B:\n");
        pc.printf("0x%X, ", IC[ic].configb.tx_data[0]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[1]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[2]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[3]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[4]);
        pc.printf("0x%X\n\n", IC[ic].configb.tx_data[5]);
      }
      else if(grp == ALL_GRP)
      {
        pc.printf("Write Config A:\n");
        pc.printf("0x%X, ", IC[ic].configa.tx_data[0]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[1]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[2]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[3]);
        pc.printf("0x%X, ", IC[ic].configa.tx_data[4]);
        pc.printf("0x%X\n\n", IC[ic].configa.tx_data[5]);

        pc.printf("Write Config B:\n");
        pc.printf("0x%X, ", IC[ic].configb.tx_data[0]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[1]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[2]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[3]);
        pc.printf("0x%X, ", IC[ic].configb.tx_data[4]);
        pc.printf("0x%X\n\n", IC[ic].configb.tx_data[5]);
      }
      else{ pc.printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printReadConfig
 * @brief Print read config A/B result.
 *
 * @details This function Print read config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printReadConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        pc.printf("Read Config A:\n");
        pc.printf("REFON:0x%X, ", IC[ic].rx_cfga.refon);
        pc.printf("CTH:0x%X\n", IC[ic].rx_cfga.cth & 0x07);
        pc.printf("FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        pc.printf("FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        pc.printf("FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        pc.printf("FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        pc.printf("FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        pc.printf("FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        pc.printf("FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        pc.printf("FLAG_D[7]:0x%X\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        pc.printf("OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        pc.printf("OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        pc.printf("SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        pc.printf("GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        pc.printf("FC:0x%X, ", (IC[ic].rx_cfga.fc));
        pc.printf("COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        pc.printf("MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        pc.printf("SNAP:0x%X\n\n", (IC[ic].rx_cfga.snap));
        pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == B)
      {
        pc.printf("Read Config B:\n");
        pc.printf("VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        pc.printf("VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        pc.printf("DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        pc.printf("DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        pc.printf("DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        pc.printf("DCC:0x%X\n\n", IC[ic].rx_cfgb.dcc);
        pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == ALL_GRP)
      {
        pc.printf("Read Config A:\n");
        pc.printf("REFON:0x%X, ", IC[ic].rx_cfga.refon);
        pc.printf("CTH:0x%X\n", IC[ic].rx_cfga.cth & 0x07);
        pc.printf("FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        pc.printf("FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        pc.printf("FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        pc.printf("FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        pc.printf("FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        pc.printf("FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        pc.printf("FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        pc.printf("FLAG_D[7]:0x%X\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        pc.printf("OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        pc.printf("OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        pc.printf("SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        pc.printf("GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        pc.printf("FC:0x%X, ", (IC[ic].rx_cfga.fc));
        pc.printf("COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        pc.printf("MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        pc.printf("SNAP:0x%X\n\n", (IC[ic].rx_cfga.snap));

        pc.printf("Read Config B:\n");
        pc.printf("VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        pc.printf("VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        pc.printf("DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        pc.printf("DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        pc.printf("DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        pc.printf("DCC:0x%X\n\n", IC[ic].rx_cfgb.dcc);
        pc.printf("CCount:%d,", IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n", IC[ic].cccrc.cfgr_pec);
      }
      else{ pc.printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printVoltages
 * @brief Print Voltages.
 *
 * @details This function Print Voltages into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC    Total IC
 *
 * @param [in]  *IC    cell_asic stucture pointer
 *
 * @param [in]  type    Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printVoltages(uint8_t tIC, cell_asic *IC, TYPE type)
{
  float voltage;
  int16_t temp;
  uint8_t channel;
  if((type == Cell) || (type == AvgCell) || (type == F_volt) || (type == S_volt))
  {
    channel = CELL;
  }
  else if (type == Aux){ channel = AUX;}
  else if (type == RAux){channel = RAUX;}
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:",(ic+1));
    for(uint8_t index = 0; index < channel; index++)
    {
      if(type == Cell){ temp = IC[ic].cell.c_codes[index]; }
      else if(type == AvgCell){ temp = IC[ic].acell.ac_codes[index]; }
      else if(type == F_volt){ temp = IC[ic].fcell.fc_codes[index]; }
      else if(type == S_volt){ temp = IC[ic].scell.sc_codes[index]; }
      else if(type == Aux){ temp = IC[ic].aux.a_codes[index]; }
      else if(type == RAux){ temp = IC[ic].raux.ra_codes[index]; }
      voltage = getVoltage(temp);
      if(type == Cell)
      {
        pc.printf("C%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.cell_pec);
        }
      }
      else if(type == AvgCell)
      {
        pc.printf("AC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.acell_pec);
        }
      }
      else if(type == F_volt)
      {
        pc.printf("FC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.fcell_pec);
        }
      }
      else if(type == S_volt)
      {
        pc.printf("S%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.scell_pec);
        }
      }
      else if(type == Aux)
      {
        if(index <= 9)
        {
          pc.printf("AUX%d=%fV,",(index+1), voltage);
        }
        else if(index == 10)
        {
          pc.printf("VMV:%fV,",(20 * voltage));
        }
        else if(index == 11)
        {
          pc.printf("V+:%fV,",(20 * voltage));
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.aux_pec);
        }
      }
      else if(type == RAux)
      {
        pc.printf("RAUX%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          pc.printf("PECError:%d",IC[ic].cccrc.raux_pec);
        }
      }
      else{ pc.printf("Wrong Register Group Select\n"); }
    }
    pc.printf("\n\n");
  }
}

/**
 *******************************************************************************
 * Function: PrintStatus
 * @brief Print status reg. result.
 *
 * @details This function Print status result into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printStatus(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  float voltage;
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Status)
    {
      if(grp == A)
      {
        pc.printf("Status A:\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        pc.printf("VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        pc.printf("VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        pc.printf("ITMP:%f°C\n", (voltage/0.0075)-273);

        pc.printf("CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == B)
      {
        pc.printf("Status B:\n");
        voltage = getVoltage(IC[ic].statb.va);
        pc.printf("VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        pc.printf("VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        pc.printf("VR4K:%fV\n", voltage);

        pc.printf("CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == C)
      {
        pc.printf("Status C:\n");
        pc.printf("CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        pc.printf("OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        pc.printf("OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        pc.printf("OTP1_MED:0x%X ", IC[ic].statc.otp1_med);
        pc.printf("OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        pc.printf("VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        pc.printf("VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        pc.printf("VA_UV:0x%X, ", IC[ic].statc.va_uv);
        pc.printf("VA_OV:0x%X\n", IC[ic].statc.va_ov);

        pc.printf("OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        pc.printf("TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        pc.printf("THSD:0x%X, ", IC[ic].statc.thsd);
        pc.printf("SLEEP:0x%X, ", IC[ic].statc.sleep);
        pc.printf("SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        pc.printf("COMP:0x%X, ", IC[ic].statc.comp);
        pc.printf("VDEL:0x%X, ", IC[ic].statc.vdel);
        pc.printf("VDE:0x%X\n", IC[ic].statc.vde);

        pc.printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == D)
      {
        pc.printf("Status D:\n");
        pc.printf("C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        pc.printf("C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        pc.printf("C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        pc.printf("C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        pc.printf("C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        pc.printf("C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        pc.printf("C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        pc.printf("C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        pc.printf("C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        pc.printf("C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        pc.printf("C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        pc.printf("C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        pc.printf("C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        pc.printf("C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        pc.printf("C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
        pc.printf("C16UV:0x%X\n", IC[ic].statd.c_uv[15]);

        pc.printf("C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        pc.printf("C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        pc.printf("C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        pc.printf("C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        pc.printf("C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        pc.printf("C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        pc.printf("C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        pc.printf("C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        pc.printf("C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        pc.printf("C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        pc.printf("C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        pc.printf("C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        pc.printf("C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        pc.printf("C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        pc.printf("C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
        pc.printf("C16OV:0x%X\n", IC[ic].statd.c_ov[15]);

        pc.printf("CTS:0x%X, ", IC[ic].statd.cts);
        pc.printf("CT:0x%X, ", IC[ic].statd.ct);
        pc.printf("OC_CNTR:0x%X\n", IC[ic].statd.oc_cntr);

        pc.printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == E)
      {
        pc.printf("Status E:\n");
        pc.printf("GPI:0x%X, ", IC[ic].state.gpi);
        pc.printf("REV_ID:0x%X\n", IC[ic].state.rev);

        pc.printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == ALL_GRP)
      {
        pc.printf("Status A:\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        pc.printf("VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        pc.printf("VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        pc.printf("ITMP:%f°C\n\n", (voltage/0.0075)-273);

        pc.printf("Status B:\n");
        voltage = getVoltage(IC[ic].statb.va);
        pc.printf("VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        pc.printf("VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        pc.printf("VR4K:%fV\n\n", voltage);

        pc.printf("Status C:\n");
        pc.printf("CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        pc.printf("OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        pc.printf("OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        pc.printf("OTP1_MED:0x%X, ", IC[ic].statc.otp1_med);
        pc.printf("OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        pc.printf("VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        pc.printf("VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        pc.printf("VA_UV:0x%X, ", IC[ic].statc.va_uv);
        pc.printf("VA_OV:0x%X\n", IC[ic].statc.va_ov);

        pc.printf("OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        pc.printf("TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        pc.printf("THSD:0x%X, ", IC[ic].statc.thsd);
        pc.printf("SLEEP:0x%X, ", IC[ic].statc.sleep);
        pc.printf("SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        pc.printf("COMP:0x%X, ", IC[ic].statc.comp);
        pc.printf("VDEL:0x%X, ", IC[ic].statc.vdel);
        pc.printf("VDE:0x%X\n\n", IC[ic].statc.vde);

        pc.printf("Status D:\n");
        pc.printf("C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        pc.printf("C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        pc.printf("C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        pc.printf("C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        pc.printf("C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        pc.printf("C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        pc.printf("C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        pc.printf("C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        pc.printf("C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        pc.printf("C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        pc.printf("C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        pc.printf("C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        pc.printf("C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        pc.printf("C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        pc.printf("C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
        pc.printf("C16UV:0x%X\n", IC[ic].statd.c_uv[15]);

        pc.printf("C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        pc.printf("C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        pc.printf("C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        pc.printf("C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        pc.printf("C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        pc.printf("C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        pc.printf("C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        pc.printf("C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        pc.printf("C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        pc.printf("C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        pc.printf("C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        pc.printf("C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        pc.printf("C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        pc.printf("C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        pc.printf("C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
        pc.printf("C16OV:0x%X\n", IC[ic].statd.c_ov[15]);

        pc.printf("CTS:0x%X, ", IC[ic].statd.cts);
        pc.printf("CT:0x%X\n\n", IC[ic].statd.ct);

        pc.printf("Status E:\n");
        pc.printf("GPI:0x%X, ", IC[ic].state.gpi);
        pc.printf("REV_ID:0x%X\n\n", IC[ic].state.rev);

        pc.printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else{ pc.printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: PrintDeviceSID
 * @brief Print Device SID.
 *
 * @details This function Print Device SID into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @return None
 *
 *******************************************************************************
*/
void printDeviceSID(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Sid)
    {
        pc.printf("Read Device SID:\n");
        pc.printf("0x%X, ", IC[ic].sid.sid[0]);
        pc.printf("0x%X, ", IC[ic].sid.sid[1]);
        pc.printf("0x%X, ", IC[ic].sid.sid[2]);
        pc.printf("0x%X, ", IC[ic].sid.sid[3]);
        pc.printf("0x%X, ", IC[ic].sid.sid[4]);
        pc.printf("0x%X, ", IC[ic].sid.sid[5]);
        pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        pc.printf("PECError:%d\n\n",IC[ic].cccrc.sid_pec);
     }
     else{ pc.printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printWritePwmDutyCycle
* @brief Print Write Pwm Duty Cycle.
*
* @details This function Print write pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printWritePwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(grp == A)
    {
      pc.printf("Write Pwma Duty Cycle:\n");
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[0]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[1]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[2]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[3]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[4]);
      pc.printf("0x%X\n\n", IC[ic].pwma.tx_data[5]);
    }
    else if(grp == B)
    {
      pc.printf("Write Pwmb Duty Cycle:\n");
      pc.printf("0x%X, ", IC[ic].pwmb.tx_data[0]);
      pc.printf("0x%X\n\n", IC[ic].pwmb.tx_data[1]);
    }
    else if(grp == ALL_GRP)
    {
      pc.printf("Write Pwma Duty Cycle:\n");
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[0]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[1]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[2]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[3]);
      pc.printf("0x%X, ", IC[ic].pwma.tx_data[4]);
      pc.printf("0x%X\n", IC[ic].pwma.tx_data[5]);

      pc.printf("Write Pwmb Duty Cycle:\n");
      pc.printf("0x%X, ", IC[ic].pwmb.tx_data[0]);
      pc.printf("0x%X\n\n", IC[ic].pwmb.tx_data[1]);
    }
    else{ pc.printf("Wrong Register Group Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadPwmDutyCycle
* @brief Print Read Pwm Duty Cycle.
*
* @details This function print read pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadPwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(grp == A)
    {
      pc.printf("Read PWMA Duty Cycle:\n");
      pc.printf("PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      pc.printf("PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      pc.printf("PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      pc.printf("PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      pc.printf("PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      pc.printf("PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      pc.printf("PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      pc.printf("PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      pc.printf("PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      pc.printf("PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      pc.printf("PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      pc.printf("PWM12:0x%X, ", IC[ic].PwmA.pwma[11]);
      pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      pc.printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == B)
    {
      pc.printf("Read PWMB Duty Cycle:\n");
      pc.printf("PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      pc.printf("PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      pc.printf("PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      pc.printf("PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      pc.printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == ALL_GRP)
    {
      pc.printf("Read PWMA Duty Cycle:\n");
      pc.printf("PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      pc.printf("PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      pc.printf("PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      pc.printf("PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      pc.printf("PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      pc.printf("PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      pc.printf("PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      pc.printf("PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      pc.printf("PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      pc.printf("PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      pc.printf("PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      pc.printf("PWM12:0x%X\n", IC[ic].PwmA.pwma[11]);

      pc.printf("Read PWMB Duty Cycle:\n");
      pc.printf("PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      pc.printf("PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      pc.printf("PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      pc.printf("PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      pc.printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else{ pc.printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printWriteCommData
* @brief Print Write Comm data.
*
* @details This function Print write comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printWriteCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Comm)
    {
      pc.printf("Write Comm Data:\n");
      pc.printf("0x%X, ", IC[ic].com.tx_data[0]);
      pc.printf("0x%X, ", IC[ic].com.tx_data[1]);
      pc.printf("0x%X, ", IC[ic].com.tx_data[2]);
      pc.printf("0x%X, ", IC[ic].com.tx_data[3]);
      pc.printf("0x%X, ", IC[ic].com.tx_data[4]);
      pc.printf("0x%X\n\n", IC[ic].com.tx_data[5]);
    }
    else{ pc.printf("Wrong Register Group Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadCommData
* @brief Print Read Comm Data.
*
* @details This function print read comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    pc.printf("IC%d:\n",(ic+1));
    if(type == Comm)
    {
      pc.printf("Read Comm Data:\n");
      pc.printf("ICOM0:0x%X, ", IC[ic].comm.icomm[0]);
      pc.printf("ICOM1:0x%X, ", IC[ic].comm.icomm[1]);
      pc.printf("ICOM2:0x%X\n", IC[ic].comm.icomm[2]);
      pc.printf("FCOM0:0x%X, ", IC[ic].comm.fcomm[0]);
      pc.printf("FCOM1:0x%X, ", IC[ic].comm.fcomm[1]);
      pc.printf("FCOM2:0x%X\n", IC[ic].comm.fcomm[2]);
      pc.printf("DATA0:0x%X, ", IC[ic].comm.data[0]);
      pc.printf("DATA1:0x%X, ", IC[ic].comm.data[1]);
      pc.printf("DATA2:0x%X\n", IC[ic].comm.data[2]);
      pc.printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      pc.printf("PECError:%d\n\n",IC[ic].cccrc.comm_pec);
    }
    else{ pc.printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printDiagnosticTestResult
* @brief Print diagnostic test result.
*
* @details This function Print diagnostic test result (PASS,FAIL) into console terminal.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  TEST     Enum type diagnostic test
*
* @return None
*
*******************************************************************************
*/
void printDiagnosticTestResult(uint8_t tIC, cell_asic *IC, DIAGNOSTIC_TYPE type)
{
  if(type == OSC_MISMATCH)
  {
    pc.printf("OSC Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.osc_mismatch);
    }
    pc.printf("\n\n");
  }

  else if(type == SUPPLY_ERROR)
  {
    pc.printf("Force Supply Error Detection Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.supply_error);
    }
    pc.printf("\n\n");
  }

  else if(type == THSD)
  {
    pc.printf("Thsd Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.thsd);
    }
    pc.printf("\n\n");
  }

  else if(type == FUSE_ED)
  {
    pc.printf("Fuse_ed Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_ed);
    }
    pc.printf("\n\n");
  }

  else if(type == FUSE_MED)
  {
    pc.printf("Fuse_med Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_med);
    }
    pc.printf("\n\n");
  }

  else if(type == TMODCHK)
  {
    pc.printf("TMODCHK Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.tmodchk);
    }
    pc.printf("\n\n");
  }
  else{pc.printf("Wrong Diagnostic Selected\n");}
}

/**
*******************************************************************************
* Function: diagnosticResultPrint
* @brief Print diagnostic (PASS/FAIL) result.
*
* @details This function print diagnostic (PASS/FAIL) result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void diagnosticTestResultPrint(uint8_t result)
{
  if(result == 1)
  {
    pc.printf("PASS\n");
  }
  else
  {
    pc.printf("FAIL\n");
  }
}

/**
*******************************************************************************
* Function: printOpenWireTestResult
* @brief Print open wire test result.
*
* @details This function print open wire test result.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printOpenWireTestResult(uint8_t tIC, cell_asic *IC, TYPE type)
{
  if(type == Cell)
  {
    pc.printf("Cell Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        pc.printf("CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cell_ow[cell]);
      }
      pc.printf("\n\n");
    }
  }
  else if(type == S_volt)
  {
    pc.printf("Cell redundant Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        pc.printf("CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cellred_ow[cell]);
      }
      pc.printf("\n\n");
    }
  }
  else if(type == Aux)
  {
    pc.printf("Aux Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      pc.printf("IC%d:\n",(ic+1));
      for(uint8_t gpio = 0; gpio < (AUX-2); gpio++)
      {
        pc.printf("GPIO%d:",(gpio+1));
        openWireResultPrint(IC[ic].diag_result.aux_ow[gpio]);
      }
      pc.printf("\n\n");
    }
  }
  else{pc.printf("Wrong Resistor Type Selected\n");}
}

/**
*******************************************************************************
* Function: openWireResultPrint
* @brief Print open wire (OPEN/CLOSE) result.
*
* @details This function print open wire result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void openWireResultPrint(uint8_t result)
{
  if(result == 1)
  {
    pc.printf(" OPEN\n");
  }
  else
  {
    pc.printf(" CLOSE\n");
  }
}

/**
 *******************************************************************************
 * Function: printPollAdcConvTime
 * @brief Print Poll adc conversion Time.
 *
 * @details This function print poll adc conversion Time.
 *
 * @return None
 *
 *******************************************************************************
*/
void printPollAdcConvTime(int count)
{
  pc.printf("Adc Conversion Time = %fms\n", (float)(count/1000.0));
}

/**
 *******************************************************************************
 * Function: printMenu
 * @brief Print Command Menu.
 *
 * @details This function print all command menu.
 *
 * @return None
 *
 *******************************************************************************
*/
void printMenu()
{
  pc.printf("List of ADBMS6830 Command:\n");
  pc.printf("Write and Read Configuration: 1      \n");
  pc.printf("Read Configuration: 2                \n");
  pc.printf("Start Cell Voltage Conversion: 3     \n");
  pc.printf("Read Cell Voltages: 4                \n");
  pc.printf("Start S-Voltage Conversion: 5        \n");
  pc.printf("Read S-Voltages: 6                   \n");
  pc.printf("Start Avg Cell Voltage Conversion: 7 \n");
  pc.printf("Read Avg Cell Voltages: 8            \n");
  pc.printf("Start F-Cell Voltage Conversion: 9   \n");
  pc.printf("Read F-Cell Voltages: 10             \n");
  pc.printf("Start Aux Voltage Conversion: 11     \n");
  pc.printf("Read Aux Voltages: 12                \n");
  pc.printf("Start RAux Voltage Conversion: 13    \n");
  pc.printf("Read RAux Voltages: 14               \n");
  pc.printf("Read Status Registers: 15            \n");
  pc.printf("Loop Measurements: 16                \n");
  pc.printf("Clear Cell registers: 17             \n");
  pc.printf("Clear Aux registers: 18              \n");
  pc.printf("Clear Spin registers: 19             \n");
  pc.printf("Clear Fcell registers: 20            \n");
  pc.printf("Write Configuration: 21 \n");
  pc.printf("\n");
  pc.printf("Print '0' for menu\n");
  pc.printf("Please enter command: \n");
  pc.printf("\n\n");
}
#else

/**
 *******************************************************************************
 * Function: printWriteConfig
 * @brief Print write config A/B result.
 *
 * @details This function Print write config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printWriteConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        printf("Write Config A:\n");
        printf("0x%X, ", IC[ic].configa.tx_data[0]);
        printf("0x%X, ", IC[ic].configa.tx_data[1]);
        printf("0x%X, ", IC[ic].configa.tx_data[2]);
        printf("0x%X, ", IC[ic].configa.tx_data[3]);
        printf("0x%X, ", IC[ic].configa.tx_data[4]);
        printf("0x%X\n\n", IC[ic].configa.tx_data[5]);
      }
      else if(grp == B)
      {
        printf("Write Config B:\n");
        printf("0x%X, ", IC[ic].configb.tx_data[0]);
        printf("0x%X, ", IC[ic].configb.tx_data[1]);
        printf("0x%X, ", IC[ic].configb.tx_data[2]);
        printf("0x%X, ", IC[ic].configb.tx_data[3]);
        printf("0x%X, ", IC[ic].configb.tx_data[4]);
        printf("0x%X\n\n", IC[ic].configb.tx_data[5]);
      }
      else if(grp == ALL_GRP)
      {
        printf("Write Config A:\n");
        printf("0x%X, ", IC[ic].configa.tx_data[0]);
        printf("0x%X, ", IC[ic].configa.tx_data[1]);
        printf("0x%X, ", IC[ic].configa.tx_data[2]);
        printf("0x%X, ", IC[ic].configa.tx_data[3]);
        printf("0x%X, ", IC[ic].configa.tx_data[4]);
        printf("0x%X\n\n", IC[ic].configa.tx_data[5]);

        printf("Write Config B:\n");
        printf("0x%X, ", IC[ic].configb.tx_data[0]);
        printf("0x%X, ", IC[ic].configb.tx_data[1]);
        printf("0x%X, ", IC[ic].configb.tx_data[2]);
        printf("0x%X, ", IC[ic].configb.tx_data[3]);
        printf("0x%X, ", IC[ic].configb.tx_data[4]);
        printf("0x%X\n\n", IC[ic].configb.tx_data[5]);
      }
      else{ printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printReadConfig
 * @brief Print read config result.
 *
 * @details This function Print read config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printReadConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        printf("Read Config A:\n");
        printf("REFON:0x%X, ", IC[ic].rx_cfga.refon);
        printf("CTH:0x%X\n", IC[ic].rx_cfga.cth & 0x07);
        printf("FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        printf("FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        printf("FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        printf("FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        printf("FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        printf("FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        printf("FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        printf("FLAG_D[7]:0x%X\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        printf("OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        printf("OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        printf("SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        printf("GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        printf("FC:0x%X, ", (IC[ic].rx_cfga.fc));
        printf("COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        printf("MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        printf("SNAP:0x%X\n\n", (IC[ic].rx_cfga.snap));
        printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == B)
      {
        printf("Read Config B:\n");
        printf("VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        printf("VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        printf("DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        printf("DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        printf("DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        printf("DCC:0x%X\n\n", IC[ic].rx_cfgb.dcc);
        printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == ALL_GRP)
      {
        printf("Read Config A:\n");
        printf("REFON:0x%X, ", IC[ic].rx_cfga.refon);
        printf("CTH:0x%X\n", IC[ic].rx_cfga.cth & 0x07);
        printf("FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        printf("FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        printf("FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        printf("FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        printf("FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        printf("FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        printf("FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        printf("FLAG_D[7]:0x%X\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        printf("OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        printf("OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        printf("SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        printf("GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        printf("FC:0x%X, ", (IC[ic].rx_cfga.fc));
        printf("COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        printf("MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        printf("SNAP:0x%X\n\n", (IC[ic].rx_cfga.snap));

        printf("Read Config B:\n");
        printf("VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        printf("VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        printf("DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        printf("DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        printf("DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        printf("DCC:0x%X\n\n", IC[ic].rx_cfgb.dcc);
        printf("CCount:%d,", IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n", IC[ic].cccrc.cfgr_pec);
      }
      else{ printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printVoltages
 * @brief Print Voltages.
 *
 * @details This function Print Voltages into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC    Total IC
 *
 * @param [in]  *IC    cell_asic stucture pointer
 *
 * @param [in]  type    Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printVoltages(uint8_t tIC, cell_asic *IC, TYPE type)
{
  float voltage;
  int16_t temp;
  uint8_t channel;
  if((type == Cell) || (type == AvgCell) || (type == F_volt) || (type == S_volt))
  {
    channel = CELL;
  }
  else if (type == Aux){ channel = AUX;}
  else if (type == RAux){channel = RAUX;}
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:",(ic+1));
    for(uint8_t index = 0; index < channel; index++)
    {
      if(type == Cell){ temp = IC[ic].cell.c_codes[index]; }
      else if(type == AvgCell){ temp = IC[ic].acell.ac_codes[index]; }
      else if(type == F_volt){ temp = IC[ic].fcell.fc_codes[index]; }
      else if(type == S_volt){ temp = IC[ic].scell.sc_codes[index]; }
      else if(type == Aux){ temp = IC[ic].aux.a_codes[index]; }
      else if(type == RAux){ temp = IC[ic].raux.ra_codes[index]; }
      voltage = getVoltage(temp);
      if(type == Cell)
      {
        printf("C%d=%fV,",(index+1), voltage);
        
        if(index == (channel-1))
        {
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.cell_pec);
        }
      }
      else if(type == AvgCell)
      {
        printf("AC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.acell_pec);
        }
      }
      else if(type == F_volt)
      {
        printf("FC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.fcell_pec);
        }
      }
      else if(type == S_volt)
      {
        printf("S%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.scell_pec);
        }
      }
      else if(type == Aux)
      {
        if(index <= 9)
        {
          printf("AUX%d=%fV,",(index+1), voltage);
        }
        else if(index == 10)
        {
          printf("VMV:%fV,",(20 * voltage));
        }
        else if(index == 11)
        {
          printf("V+:%fV,",(20 * voltage));
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.aux_pec);
        }
      }
      else if(type == RAux)
      {
        printf("RAUX%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
          printf("PECError:%d",IC[ic].cccrc.raux_pec);
        }
      }
      else{ printf("Wrong Register Group Select\n"); }
    }
    printf("\n\n");
  }
}

/**
 *******************************************************************************
 * Function: PrintStatus
 * @brief Print status reg. result.
 *
 * @details This function Print status result into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printStatus(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  float voltage;
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Status)
    {
      if(grp == A)
      {
        printf("Status A:\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        printf("VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        printf("VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        printf("ITMP:%f°C\n", (voltage/0.0075)-273);

        printf("CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == B)
      {
        printf("Status B:\n");
        voltage = getVoltage(IC[ic].statb.va);
        printf("VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        printf("VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        printf("VR4K:%fV\n", voltage);

        printf("CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == C)
      {
        printf("Status C:\n");
        printf("CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        printf("OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        printf("OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        printf("OTP1_MED:0x%X ", IC[ic].statc.otp1_med);
        printf("OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        printf("VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        printf("VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        printf("VA_UV:0x%X, ", IC[ic].statc.va_uv);
        printf("VA_OV:0x%X\n", IC[ic].statc.va_ov);

        printf("OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        printf("TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        printf("THSD:0x%X, ", IC[ic].statc.thsd);
        printf("SLEEP:0x%X, ", IC[ic].statc.sleep);
        printf("SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        printf("COMP:0x%X, ", IC[ic].statc.comp);
        printf("VDEL:0x%X, ", IC[ic].statc.vdel);
        printf("VDE:0x%X\n", IC[ic].statc.vde);

        printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == D)
      {
        printf("Status D:\n");
        printf("C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        printf("C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        printf("C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        printf("C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        printf("C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        printf("C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        printf("C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        printf("C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        printf("C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        printf("C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        printf("C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        printf("C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        printf("C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        printf("C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        printf("C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
        printf("C16UV:0x%X\n", IC[ic].statd.c_uv[15]);

        printf("C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        printf("C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        printf("C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        printf("C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        printf("C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        printf("C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        printf("C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        printf("C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        printf("C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        printf("C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        printf("C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        printf("C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        printf("C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        printf("C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        printf("C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
        printf("C16OV:0x%X\n", IC[ic].statd.c_ov[15]);

        printf("CTS:0x%X, ", IC[ic].statd.cts);
        printf("CT:0x%X, ", IC[ic].statd.ct);
        printf("OC_CNTR:0x%X\n", IC[ic].statd.oc_cntr);

        printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == E)
      {
        printf("Status E:\n");
        printf("GPI:0x%X, ", IC[ic].state.gpi);
        printf("REV_ID:0x%X\n", IC[ic].state.rev);

        printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == ALL_GRP)
      {
        printf("Status A:\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        printf("VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        printf("VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        printf("ITMP:%f°C\n\n", (voltage/0.0075)-273);

        printf("Status B:\n");
        voltage = getVoltage(IC[ic].statb.va);
        printf("VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        printf("VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        printf("VR4K:%fV\n\n", voltage);

        printf("Status C:\n");
        printf("CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        printf("OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        printf("OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        printf("OTP1_MED:0x%X, ", IC[ic].statc.otp1_med);
        printf("OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        printf("VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        printf("VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        printf("VA_UV:0x%X, ", IC[ic].statc.va_uv);
        printf("VA_OV:0x%X\n", IC[ic].statc.va_ov);

        printf("OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        printf("TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        printf("THSD:0x%X, ", IC[ic].statc.thsd);
        printf("SLEEP:0x%X, ", IC[ic].statc.sleep);
        printf("SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        printf("COMP:0x%X, ", IC[ic].statc.comp);
        printf("VDEL:0x%X, ", IC[ic].statc.vdel);
        printf("VDE:0x%X\n\n", IC[ic].statc.vde);

        printf("Status D:\n");
        printf("C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        printf("C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        printf("C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        printf("C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        printf("C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        printf("C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        printf("C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        printf("C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        printf("C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        printf("C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        printf("C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        printf("C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        printf("C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        printf("C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        printf("C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
        printf("C16UV:0x%X\n", IC[ic].statd.c_uv[15]);

        printf("C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        printf("C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        printf("C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        printf("C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        printf("C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        printf("C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        printf("C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        printf("C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        printf("C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        printf("C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        printf("C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        printf("C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        printf("C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        printf("C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        printf("C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
        printf("C16OV:0x%X\n", IC[ic].statd.c_ov[15]);

        printf("CTS:0x%X, ", IC[ic].statd.cts);
        printf("CT:0x%X\n\n", IC[ic].statd.ct);

        printf("Status E:\n");
        printf("GPI:0x%X, ", IC[ic].state.gpi);
        printf("REV_ID:0x%X\n\n", IC[ic].state.rev);

        printf("CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n", IC[ic].cccrc.stat_pec);
      }
      else{ printf("Wrong Register Group Select\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: PrintDeviceSID
 * @brief Print Device SID.
 *
 * @details This function Print Device SID into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @return None
 *
 *******************************************************************************
*/
void printDeviceSID(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Sid)
    {
        printf("Read Device SID:\n");
        printf("0x%X, ", IC[ic].sid.sid[0]);
        printf("0x%X, ", IC[ic].sid.sid[1]);
        printf("0x%X, ", IC[ic].sid.sid[2]);
        printf("0x%X, ", IC[ic].sid.sid[3]);
        printf("0x%X, ", IC[ic].sid.sid[4]);
        printf("0x%X, ", IC[ic].sid.sid[5]);
        printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
        printf("PECError:%d\n\n",IC[ic].cccrc.sid_pec);
     }
     else{ printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printWritePwmDutyCycle
* @brief Print Write Pwm Duty Cycle.
*
* @details This function Print write pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printWritePwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(grp == A)
    {
      printf("Write Pwma Duty Cycle:\n");
      printf("0x%X, ", IC[ic].pwma.tx_data[0]);
      printf("0x%X, ", IC[ic].pwma.tx_data[1]);
      printf("0x%X, ", IC[ic].pwma.tx_data[2]);
      printf("0x%X, ", IC[ic].pwma.tx_data[3]);
      printf("0x%X, ", IC[ic].pwma.tx_data[4]);
      printf("0x%X\n\n", IC[ic].pwma.tx_data[5]);
    }
    else if(grp == B)
    {
      printf("Write Pwmb Duty Cycle:\n");
      printf("0x%X, ", IC[ic].pwmb.tx_data[0]);
      printf("0x%X\n\n", IC[ic].pwmb.tx_data[1]);
    }
    else if(grp == ALL_GRP)
    {
      printf("Write Pwma Duty Cycle:\n");
      printf("0x%X, ", IC[ic].pwma.tx_data[0]);
      printf("0x%X, ", IC[ic].pwma.tx_data[1]);
      printf("0x%X, ", IC[ic].pwma.tx_data[2]);
      printf("0x%X, ", IC[ic].pwma.tx_data[3]);
      printf("0x%X, ", IC[ic].pwma.tx_data[4]);
      printf("0x%X\n", IC[ic].pwma.tx_data[5]);

      printf("Write Pwmb Duty Cycle:\n");
      printf("0x%X, ", IC[ic].pwmb.tx_data[0]);
      printf("0x%X\n\n", IC[ic].pwmb.tx_data[1]);
    }
    else{ printf("Wrong Register Group Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadPwmDutyCycle
* @brief Print Read Pwm Duty Cycle.
*
* @details This function print read pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadPwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(grp == A)
    {
      printf("Read PWMA Duty Cycle:\n");
      printf("PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      printf("PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      printf("PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      printf("PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      printf("PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      printf("PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      printf("PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      printf("PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      printf("PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      printf("PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      printf("PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      printf("PWM12:0x%X, ", IC[ic].PwmA.pwma[11]);
      printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == B)
    {
      printf("Read PWMB Duty Cycle:\n");
      printf("PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      printf("PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      printf("PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      printf("PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == ALL_GRP)
    {
      printf("Read PWMA Duty Cycle:\n");
      printf("PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      printf("PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      printf("PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      printf("PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      printf("PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      printf("PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      printf("PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      printf("PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      printf("PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      printf("PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      printf("PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      printf("PWM12:0x%X\n", IC[ic].PwmA.pwma[11]);

      printf("Read PWMB Duty Cycle:\n");
      printf("PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      printf("PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      printf("PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      printf("PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      printf("PECError:%d\n\n",IC[ic].cccrc.pwm_pec);
    }
    else{ printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printWriteCommData
* @brief Print Write Comm data.
*
* @details This function Print write comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printWriteCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Comm)
    {
      printf("Write Comm Data:\n");
      printf("0x%X, ", IC[ic].com.tx_data[0]);
      printf("0x%X, ", IC[ic].com.tx_data[1]);
      printf("0x%X, ", IC[ic].com.tx_data[2]);
      printf("0x%X, ", IC[ic].com.tx_data[3]);
      printf("0x%X, ", IC[ic].com.tx_data[4]);
      printf("0x%X\n\n", IC[ic].com.tx_data[5]);
    }
    else{ printf("Wrong Register Group Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadCommData
* @brief Print Read Comm Data.
*
* @details This function print read comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    printf("IC%d:\n",(ic+1));
    if(type == Comm)
    {
      printf("Read Comm Data:\n");
      printf("ICOM0:0x%X, ", IC[ic].comm.icomm[0]);
      printf("ICOM1:0x%X, ", IC[ic].comm.icomm[1]);
      printf("ICOM2:0x%X\n", IC[ic].comm.icomm[2]);
      printf("FCOM0:0x%X, ", IC[ic].comm.fcomm[0]);
      printf("FCOM1:0x%X, ", IC[ic].comm.fcomm[1]);
      printf("FCOM2:0x%X\n", IC[ic].comm.fcomm[2]);
      printf("DATA0:0x%X, ", IC[ic].comm.data[0]);
      printf("DATA1:0x%X, ", IC[ic].comm.data[1]);
      printf("DATA2:0x%X\n", IC[ic].comm.data[2]);
      printf("CCount:%d,",IC[ic].cccrc.cmd_cntr);
      printf("PECError:%d\n\n",IC[ic].cccrc.comm_pec);
    }
    else{ printf("Wrong Register Type Select\n"); }
  }
}

/**
*******************************************************************************
* Function: printDiagnosticTestResult
* @brief Print diagnostic test result.
*
* @details This function Print diagnostic test result (PASS,FAIL) into console terminal.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  TEST     Enum type diagnostic test
*
* @return None
*
*******************************************************************************
*/
void printDiagnosticTestResult(uint8_t tIC, cell_asic *IC, DIAGNOSTIC_TYPE type)
{
  if(type == OSC_MISMATCH)
  {
    printf("OSC Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.osc_mismatch);
    }
    printf("\n\n");
  }

  else if(type == SUPPLY_ERROR)
  {
    printf("Force Supply Error Detection Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.supply_error);
    }
    printf("\n\n");
  }

  else if(type == THSD)
  {
    printf("Thsd Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.thsd);
    }
    printf("\n\n");
  }

  else if(type == FUSE_ED)
  {
    printf("Fuse_ed Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_ed);
    }
    printf("\n\n");
  }

  else if(type == FUSE_MED)
  {
    printf("Fuse_med Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_med);
    }
    printf("\n\n");
  }

  else if(type == TMODCHK)
  {
    printf("TMODCHK Diagnostic Test:\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.tmodchk);
    }
    printf("\n\n");
  }
  else{printf("Wrong Diagnostic Selected\n");}
}

/**
*******************************************************************************
* Function: diagnosticResultPrint
* @brief Print diagnostic (PASS/FAIL) result.
*
* @details This function print diagnostic (PASS/FAIL) result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void diagnosticTestResultPrint(uint8_t result)
{
  if(result == 1)
  {
    printf("PASS\n");
  }
  else
  {
    printf("FAIL\n");
  }
}

/**
*******************************************************************************
* Function: printOpenWireTestResult
* @brief Print open wire test result.
*
* @details This function print open wire test result.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printOpenWireTestResult(uint8_t tIC, cell_asic *IC, TYPE type)
{
  if(type == Cell)
  {
    printf("Cell Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        printf("CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cell_ow[cell]);
      }
      printf("\n\n");
    }
  }
  else if(type == S_volt)
  {
    printf("Cell redundant Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        printf("CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cellred_ow[cell]);
      }
      printf("\n\n");
    }
  }
  else if(type == Aux)
  {
    printf("Aux Open Wire Test\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      printf("IC%d:\n",(ic+1));
      for(uint8_t gpio = 0; gpio < (AUX-2); gpio++)
      {
        printf("GPIO%d:",(gpio+1));
        openWireResultPrint(IC[ic].diag_result.aux_ow[gpio]);
      }
      printf("\n\n");
    }
  }
  else{printf("Wrong Resistor Type Selected\n");}
}

/**
*******************************************************************************
* Function: openWireResultPrint
* @brief Print open wire (OPEN/CLOSE) result.
*
* @details This function print open wire result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void openWireResultPrint(uint8_t result)
{
  if(result == 1)
  {
    printf(" OPEN\n");
  }
  else
  {
    printf(" CLOSE\n");
  }
}

/**
 *******************************************************************************
 * Function: printPollAdcConvTime
 * @brief Print Poll adc conversion Time.
 *
 * @details This function print poll adc conversion Time.
 *
 * @return None
 *
 *******************************************************************************
*/
void printPollAdcConvTime(int count)
{
  printf("Adc Conversion Time = %fms\n", (float)(count/64000.0));
}

/**
 *******************************************************************************
 * Function: printMenu
 * @brief Print Command Menu.
 *
 * @details This function print all command menu.
 *
 * @return None
 *
 *******************************************************************************
*/
void printMenu()
{
  printf("List of ADBMS6830 Command:\n");
  printf("Write and Read Configuration: 1 \n");
  printf("Read Configuration: 2 \n");
  printf("Start Cell Voltage Conversion: 3 \n");
  printf("Read Cell Voltages: 4 \n");
  printf("Start S-Voltage Conversion: 5 \n");
  printf("Read S-Voltages: 6 \n");
  printf("Start Avg Cell Voltage Conversion: 7 \n");
  printf("Read Avg Cell Voltages: 8 \n");
  printf("Start F-Cell Voltage Conversion: 9 \n");
  printf("Read F-Cell Voltages: 10 \n");
  printf("Start Aux Voltage Conversion: 11 \n");
  printf("Read Aux Voltages: 12 \n");
  printf("Start RAux Voltage Conversion: 13 \n");
  printf("Read RAux Voltages: 14 \n");
  printf("Read Status Registers: 15 \n");
  printf("Loop Measurements: 16 \n");
  printf("Clear Cell registers: 17 \n");
  printf("Clear Aux registers: 18 \n");
  printf("Clear Spin registers: 19 \n");
  printf("Clear Fcell registers: 20 \n");
  printf("Write Configuration: 21 \n");

  printf("\n");
  printf("Print '0' for menu\n");
  printf("Please enter command: \n");
  printf("\n\n");
}
#endif

/**
 *******************************************************************************
 * Function: getVoltage
 * @brief Get voltages with multiplication factor.
 *
 * @details This function calculate the voltage.
 *
 * Parameters:
 * @param [in]  data    voltages(uint16_t)
 *
 * @return voltage(float)
 *
 *******************************************************************************
*/
float getVoltage(int data)
{
    float voltage_float; //voltage in Volts
    voltage_float = ((data + 10000) * 0.000150);
    return voltage_float;
}

/** @}*/
/** @}*/