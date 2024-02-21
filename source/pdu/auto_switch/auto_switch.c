/**
 * @file auto_switch.c
 * @author Gavin Zyonse (gzyonse@purdue.edu)
 * @brief 
 * @version 1.0
 * @date 2023-11-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "auto_switch.h"
#include "common/phal_F4_F7/gpio/gpio.h"

// Initialize struct
auto_switches_t auto_switches;

// Static function declarations
void updateCurrent();
void updateVoltage();
uint16_t calcCurrent_HP(uint16_t);
uint16_t calcCurrent_LP(uint16_t);
void calcCurrent_Total();
uint16_t calcVoltage(uint16_t, int, int);

// Called periodically, Calculates current through each switch in mA
void updateCurrent() {
    // High power switches
    auto_switches.current[SW_PUMP_1] = calcCurrent_HP(adc_readings.pump_1_imon);
    auto_switches.current[SW_PUMP_2] = calcCurrent_HP(adc_readings.pump_2_imon);
    auto_switches.current[SW_SDC] = calcCurrent_HP(adc_readings.sdc_imon);
    auto_switches.current[SW_AUX] = calcCurrent_HP(adc_readings.aux_hp_imon);

    // Low power switches
    auto_switches.current[SW_FAN_1] = calcCurrent_LP(adc_readings.fan_1_cs);
    auto_switches.current[SW_FAN_2] = calcCurrent_LP(adc_readings.fan_2_cs);
    auto_switches.current[SW_DASH] = calcCurrent_LP(adc_readings.dash_cs);
    auto_switches.current[SW_ABOX] = calcCurrent_LP(adc_readings.abox_cs);
    auto_switches.current[SW_MAIN] = calcCurrent_LP(adc_readings.main_cs);

    // Upstream CS
    calcCurrent_Total();
}

// Called periodically, Updates voltage for each rail in mV
void updateVoltage() {
    auto_switches.voltage.in_24v = calcVoltage(adc_readings.lv_24_v_sense, LV_24V_R1, LV_24V_R2);
    auto_switches.voltage.out_5v = calcVoltage(adc_readings.lv_5_v_sense, LV_5V_R1, LV_5V_R2);
    auto_switches.voltage.out_3v3 = calcVoltage(adc_readings.lv_3v3_v_sense, LV_3V3_R1, LV_3V3_R2);
}

// Current helper functions
uint16_t calcCurrent_HP(uint16_t current) {
    current = current * ADC_REF_mV / ADC_MAX;  // Convert to mV
    current = current * (HP_CS_R1 + HP_CS_R2) / HP_CS_R2;
    current = current * HP_CS_R3 / (HP_CS_R1 + HP_CS_R2);
    return current;
}

uint16_t calcCurrent_LP(uint16_t current) {
    current = current * ADC_REF_mV / ADC_MAX;  // Convert to mA
    return current;
}

// CS signals for upstream 24V and 5V (total current through each)
void calcCurrent_Total() {
    // 24V current
    uint16_t current = adc_readings.lv_24_i_sense;
    current = current * ADC_REF_mV / ADC_MAX;  // Convert to mV
    current = current / HP_CS_R_SENSE / CS_GAIN;
    auto_switches.current[CS_24V] = current;

    // 5V current
    current = adc_readings.lv_5_i_sense;
    current = current * ADC_REF_mV / ADC_MAX;  // Convert to mA
    auto_switches.current[CS_5V] = current;
}

// Converts ADC voltage reading to mV
uint16_t calcVoltage(uint16_t voltage, int r1, int r2) {
    // Compensate for voltage divider
    voltage = voltage * (r1 + r2) / r2;
    // Scale for 12-bit adc at 3.3v
    voltage = voltage * ADC_REF_mV / ADC_MAX;
    return voltage;
}

// Enable or disable switches by name
void setSwitch(uint16_t auto_switch_enum, uint16_t state) {
    switch (auto_switch_enum) {
        case SW_PUMP_1:
            PHAL_writeGPIO(PUMP_1_CTRL_GPIO_Port, PUMP_1_CTRL_Pin, state);
            break;
        case SW_PUMP_2:
            PHAL_writeGPIO(PUMP_2_CTRL_GPIO_Port, PUMP_2_CTRL_Pin, state);
            break;
        case SW_SDC:
            PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, state);
            break;
        case SW_AUX:
            PHAL_writeGPIO(AUX_HP_CTRL_GPIO_Port, AUX_HP_CTRL_Pin, state);
            break;
        case SW_FAN_1:
            PHAL_writeGPIO(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin, state);
            break;
        case SW_FAN_2:
            PHAL_writeGPIO(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin, state);
            break;
        case SW_BLT:
            PHAL_writeGPIO(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin, state);
            break;
        case SW_CRIT_5V:
            PHAL_writeGPIO(CRIT_5V_CTRL_GPIO_Port, CRIT_5V_CTRL_Pin, state);
            break;
        case SW_NCRIT_5V:
            PHAL_writeGPIO(NCRIT_5V_CTRL_GPIO_Port, NCRIT_5V_CTRL_Pin, state);
            break;
        case SW_DAQ:
            PHAL_writeGPIO(DAQ_CTRL_GPIO_Port, DAQ_CTRL_Pin, state);
            break;
        case SW_FAN_5V:
            PHAL_writeGPIO(FAN_5V_CTRL_GPIO_Port, FAN_5V_CTRL_Pin, state);
            break;
    }
}

// Get switch state and return it
bool getSwitchStatus(switches_t auto_switch_enum) {
    bool status;
    switch (auto_switch_enum) {
        case SW_PUMP_1:
            status = PHAL_readGPIO(PUMP_1_CTRL_GPIO_Port, PUMP_1_CTRL_Pin);
            break;
        case SW_PUMP_2:
            status = PHAL_readGPIO(PUMP_2_CTRL_GPIO_Port, PUMP_2_CTRL_Pin);
            break;
        case SW_SDC:
            status = PHAL_readGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin);
            break;
        case SW_AUX:
            status = PHAL_readGPIO(AUX_HP_CTRL_GPIO_Port, AUX_HP_CTRL_Pin);
            break;
        case SW_FAN_1:
            status = PHAL_readGPIO(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin);
            break;
        case SW_FAN_2:
            status = PHAL_readGPIO(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin);
            break;
        case SW_BLT:
            status = PHAL_readGPIO(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin);
            break;
        case SW_CRIT_5V:
            status = PHAL_readGPIO(CRIT_5V_CTRL_GPIO_Port, CRIT_5V_CTRL_Pin);
            break;
        case SW_NCRIT_5V:
            status = PHAL_readGPIO(NCRIT_5V_CTRL_GPIO_Port, NCRIT_5V_CTRL_Pin);
            break;
        case SW_DAQ:
            status = PHAL_readGPIO(DAQ_CTRL_GPIO_Port, DAQ_CTRL_Pin);
            break;
        case SW_FAN_5V:
            status = PHAL_readGPIO(FAN_5V_CTRL_GPIO_Port, FAN_5V_CTRL_Pin);
            break;
    }

    return status;
}

void autoSwitchPeriodic() {
    updateCurrent();
    updateVoltage();
}
