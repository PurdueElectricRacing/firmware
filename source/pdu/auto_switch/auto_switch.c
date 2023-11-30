/**
 * @file auto_switch.c
 * @author Gavin Zyonse (gzyonse@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2023-11-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "auto_switch.h"

uint8_t faultStatus() {
    return 0;
}

void getFaults() {
    
}

void getCurrent() {
    // High power switches
    auto_switch.current[SW_PUMP_1] = getCurrent_HP(adc_readings.pump_1_imon);
    auto_switch.current[SW_PUMP_2] = getCurrent_HP(adc_readings.pump_2_imon);
    auto_switch.current[SW_SDC] = getCurrent_HP(adc_readings.sdc_imon);
    auto_switch.current[SW_AUX] = getCurrent_HP(adc_readings.aux_hp_imon);

    // Low power switches
    auto_switch.current[SW_FAN_1] = getCurrent_LP(adc_readings.fan_1_cs);
    auto_switch.current[SW_FAN_2] = getCurrent_LP(adc_readings.fan_2_cs);
    auto_switch.current[SW_DASH] = getCurrent_LP(adc_readings.dash_cs);
    auto_switch.current[SW_ABOX] = getCurrent_LP(adc_readings.abox_cs);
    auto_switch.current[SW_MAIN] = getCurrent_LP(adc_readings.main_cs);
    // TODO: bullet switch doesn't have cs signal

    // 5V switches
    /*
    auto_switch.current[SW_CRIT_5V] =;
    auto_switch.current[SW_NCRIT_5V] =;
    auto_switch.current[SW_DAQ] =;
    auto_switch.current[SW_FAN_5V] =;
    */
}

// Updates voltage for all switches and stores values in struct
void getVoltage() {
    auto_switch.voltage.in_24v = adc_readings.lv_24_v_sense * (3300 / (47000 + 3300));
    auto_switch.voltage.out_5v = adc_readings.lv_5_v_sense * (3300 / (4300 + 3300));
    auto_switch.voltage.out_3v3 = adc_readings.lv_3v3_v_sense * (10000 / (4300 + 10000));
}

// Current helper functions
float getCurrent_HP(uint16_t adc_reading) {
    uint16_t current = adc_reading;
    return current;
}

float getCurrent_LP(uint16_t adc_reading) {
    // There's probably some math to be done here
    uint16_t current = adc_reading;
    return current;
}

float getCurrent_5V() {
    return 0;
}

void enableSwitch() {

}