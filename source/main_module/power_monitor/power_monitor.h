/**
 * @file power_monitor.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Monitor voltage and current present on the board
 * @version 0.1
 * @date 2023-02-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _POWER_MONITOR_H_
#define _POWER_MONITOR_H_

#include <stdint.h>

/* VOLTAGE SENSE */
/* R1 is top, R2 is bottom in voltage divider */
#define LV_24V_R1  47000 // Ohms
#define LV_24V_R2  3300  // Ohms
#define LV_12V_R1  15800 // Ohms
#define LV_12V_R2  3300  // Ohms
#define LV_5V_R1   4300  // Ohms
#define LV_5V_R2   3300  // Ohms
#define LV_3V3_R1  4300  // Ohms
#define LV_3V3_R2  10000 // Ohms

#define LV_24V_CAL (1015)        // V_actual / V_measured * 1000
#define LV_12V_CAL (LV_24V_CAL)
#define LV_5V_CAL  (LV_24V_CAL)
#define LV_3V3_CAL (LV_24V_CAL)

/* CURRENT SENSE */
#define LV_I_SENSE_GAIN 100 // V/V
#define LV_I_SENSE_R    2   // mOhm

/* INTENERAL TEMPERATURE SENSOR */
#define TS_CAL1_ADDR   ((uint16_t*) 0x1FFF75A8UL)
#define TS_CAL2_ADDR   ((uint16_t*) 0x1FFF75CAUL)
#define TS_CAL1_TEMP   ((int32_t) 30)      // C +/-5
#define TS_CAL2_TEMP   ((int32_t) 130)     // C +/-5
#define TS_CAL_ADC_REF (3000UL)            // mV

typedef struct
{
    uint16_t lv_24_v_sense_mV;
    uint16_t lv_24_i_sense_mA;
    uint16_t lv_12_v_sense_mV;
    uint16_t lv_5_v_sense_mV;
    uint16_t lv_5_i_sense_mA;
    uint16_t lv_3v3_v_sense_mV;
    uint8_t  lv_3v3_power_good;
    int16_t  mcu_temp;
} PowerMonitor_t;

extern PowerMonitor_t power_monitor;

void initPowerMonitor();
void updatePowerMonitor();

#endif