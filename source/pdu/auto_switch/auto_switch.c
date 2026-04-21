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

#include "can_library/faults_common.h"
#include "can_library/generated/PDU.h"
#include "common/phal/gpio.h"
#include "led.h"
#include "main.h"

// Initialize struct
auto_switches_t auto_switches;
static uint16_t mux_readings[6];
static uint8_t mux_index;

// Static function declarations
static void updateCurrent();
static void updateVoltage();
static uint16_t calcCurrent_HP(uint16_t);
static uint16_t calcCurrent_LP(uint16_t);
static void calcCurrent_Total();
static uint16_t calcVoltage(uint16_t, int, int);
static void setMuxChannel(uint8_t channel);
static void updateMux();

// Called periodically, Calculates current through each switch in mA
void updateCurrent() {
    // High power switches (direct ADC)
    auto_switches.current[SW_PUMP_1] = calcCurrent_HP(adc_readings.pump_1_imon);
    auto_switches.current[SW_PUMP_2] = calcCurrent_HP(adc_readings.pump_2_imon);
    auto_switches.current[SW_SDC]    = calcCurrent_HP(adc_readings.sdc_imon);
    auto_switches.current[SW_HXFAN]  = calcCurrent_HP(adc_readings.hxfan_imon);

    // High power switches (mux-sensed)
    auto_switches.current[SW_FAN_1] = calcCurrent_HP(mux_readings[0]);
    auto_switches.current[SW_FAN_2] = calcCurrent_HP(mux_readings[1]);
    auto_switches.current[SW_FAN_3] = calcCurrent_HP(mux_readings[2]);
    auto_switches.current[SW_FAN_4] = calcCurrent_HP(mux_readings[3]);
    auto_switches.current[SW_AMK1]  = calcCurrent_HP(mux_readings[4]);
    auto_switches.current[SW_AMK2]  = calcCurrent_HP(mux_readings[5]);

    // Low power switches
    auto_switches.current[SW_DASH] = calcCurrent_LP(adc_readings.dash_cs);
    auto_switches.current[SW_ABOX] = calcCurrent_LP(adc_readings.abox_cs);
    auto_switches.current[SW_MAIN] = calcCurrent_LP(adc_readings.main_cs);
    auto_switches.current[SW_DLFR] = calcCurrent_LP(adc_readings.dlfr_cs);
    auto_switches.current[SW_DLBK] = calcCurrent_LP(adc_readings.dlbk_cs);

    // Upstream CS
    calcCurrent_Total();
}

// Called periodically, Updates voltage for each rail in mV
void updateVoltage() {
    auto_switches.voltage.in_24v  = calcVoltage(adc_readings.v24_vs, LV_24V_R1, LV_24V_R2);
    auto_switches.voltage.out_5v  = calcVoltage(adc_readings.v5_vs, LV_5V_R1, LV_5V_R2);
    auto_switches.voltage.out_3v3 = calcVoltage(adc_readings.v3v3_vs, LV_3V3_R1, LV_3V3_R2);
}

static void updateMux() {
    mux_readings[mux_index] = adc_readings.mux_out;
    mux_index               = (uint8_t)((mux_index + 1U) % 6U);
    setMuxChannel(mux_index);
}

uint16_t getMuxReading(uint8_t channel) {
    if (channel >= 6U) {
        return 0;
    }

    return mux_readings[channel];
}

static void setMuxChannel(uint8_t channel) {
    PHAL_writeGPIO(MUX_CTRL_A_GPIO_Port, MUX_CTRL_A_Pin, (channel & 0x01U) != 0U);
    PHAL_writeGPIO(MUX_CTRL_B_GPIO_Port, MUX_CTRL_B_Pin, (channel & 0x02U) != 0U);
    PHAL_writeGPIO(MUX_CTRL_C_GPIO_Port, MUX_CTRL_C_Pin, (channel & 0x04U) != 0U);
}

// Current helper functions
uint16_t calcCurrent_HP(uint16_t current) {
    current = current * ADC_REF_mV / ADC_MAX; // Convert to mV
    current = current * (HP_CS_R1 + HP_CS_R2) / HP_CS_R2;
    current = current * HP_CS_R3 / (HP_CS_R1 + HP_CS_R2);
    return current;
}

uint16_t calcCurrent_LP(uint16_t current) {
    current = current * ADC_REF_mV / ADC_MAX; // Convert to mA
    return current;
}

// CS signals for upstream 24V and 5V (total current through each)
void calcCurrent_Total() {
    // 24V current
    uint16_t current              = adc_readings.v24_cs;
    current                       = current * ADC_REF_mV / ADC_MAX; // Convert to mV
    current                       = current / HP_CS_R_SENSE / CS_GAIN;
    auto_switches.current[CS_24V] = current;

    // 5V current
    current                      = adc_readings.v5_cs;
    current                      = current * ADC_REF_mV / ADC_MAX; // Convert to mA
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
void setSwitch(switches_t auto_switch_enum, bool state) {
    switch (auto_switch_enum) {
        case CS_24V:
        case CS_5V:
        case CS_SWITCH_COUNT:
            return;
        case SW_PUMP_1:
            PHAL_writeGPIO(PUMP_1_CTRL_GPIO_Port, PUMP_1_CTRL_Pin, state);
            LED_control(LED_PUMP_1, state);
            break;
        case SW_PUMP_2:
            PHAL_writeGPIO(PUMP_2_CTRL_GPIO_Port, PUMP_2_CTRL_Pin, state);
            LED_control(LED_PUMP_2, state);
            break;
        case SW_SDC:
            // NoToggle switch (always on)
            LED_control(LED_SDC, state);
            break;
        case SW_AMK1:
        case SW_AMK2:
            // NoToggle switches (always on, no LED on driver)
            break;
        case SW_HXFAN:
            PHAL_writeGPIO(HXFAN_CTRL_GPIO_Port, HXFAN_CTRL_Pin, state);
            LED_control(LED_HXFAN, state);
            break;
        case SW_FAN_1:
            PHAL_writeGPIO(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin, state);
            break;
        case SW_FAN_2:
            PHAL_writeGPIO(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin, state);
            break;
        case SW_FAN_3:
            PHAL_writeGPIO(FAN_3_CTRL_GPIO_Port, FAN_3_CTRL_Pin, state);
            break;
        case SW_FAN_4:
            PHAL_writeGPIO(FAN_4_CTRL_GPIO_Port, FAN_4_CTRL_Pin, state);
            break;
        case SW_BLT:
            PHAL_writeGPIO(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin, state);
            LED_control(LED_BLT, state);
            break;
        case SW_CRIT_5V:
            PHAL_writeGPIO(CRIT_5V_CTRL_GPIO_Port, CRIT_5V_CTRL_Pin, state);
            LED_control(LED_5V_CRIT, state);
            break;
        case SW_MAIN:
            LED_control(LED_MAIN, state);
            break;
        case SW_DLFR:
            PHAL_writeGPIO(DLFR_CTRL_GPIO_Port, DLFR_CTRL_Pin, state);
            LED_control(LED_DLFR, state);
            break;
        case SW_DLBK:
            PHAL_writeGPIO(DLBK_CTRL_GPIO_Port, DLBK_CTRL_Pin, state);
            LED_control(LED_DLBK, state);
            break;
        case SW_ABOX:
            LED_control(LED_ABOX, state);
            break;
        case SW_DASH:
            LED_control(LED_DASH, state);
            break;
        case SW_TV:
            PHAL_writeGPIO(TV_CTRL_GPIO_Port, TV_CTRL_Pin, state);
            LED_control(LED_TV, state);
            break;
        case SW_DAQ:
            // NoToggle switch (always on)
            LED_control(LED_DAQ, state);
            break;
        case SW_FAN_5V:
            PHAL_writeGPIO(FAN_5V_CTRL_GPIO_Port, FAN_5V_CTRL_Pin, state);
            LED_control(LED_5V_FAN, state);
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
        case SW_HXFAN:
            status = PHAL_readGPIO(HXFAN_CTRL_GPIO_Port, HXFAN_CTRL_Pin);
            break;
        case SW_FAN_1:
            status = PHAL_readGPIO(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin);
            break;
        case SW_FAN_2:
            status = PHAL_readGPIO(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin);
            break;
        case SW_FAN_3:
            status = PHAL_readGPIO(FAN_3_CTRL_GPIO_Port, FAN_3_CTRL_Pin);
            break;
        case SW_FAN_4:
            status = PHAL_readGPIO(FAN_4_CTRL_GPIO_Port, FAN_4_CTRL_Pin);
            break;
        case SW_BLT:
            status = PHAL_readGPIO(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin);
            break;
        case SW_CRIT_5V:
            status = PHAL_readGPIO(CRIT_5V_CTRL_GPIO_Port, CRIT_5V_CTRL_Pin);
            break;
        case SW_TV:
            status = PHAL_readGPIO(TV_CTRL_GPIO_Port, TV_CTRL_Pin);
            break;
        case SW_FAN_5V:
            status = PHAL_readGPIO(FAN_5V_CTRL_GPIO_Port, FAN_5V_CTRL_Pin);
            break;
        case SW_DLFR:
            status = PHAL_readGPIO(DLFR_CTRL_GPIO_Port, DLFR_CTRL_Pin);
            break;
        case SW_DLBK:
            status = PHAL_readGPIO(DLBK_CTRL_GPIO_Port, DLBK_CTRL_Pin);
            break;
        default:
            status = 1; // Non-controllable switches are always on
    }

    return status;
}

void autoSwitchPeriodic() {
    updateMux();
    updateCurrent();
    updateVoltage();
}

void checkSwitchFaults() {
    // Get status of all switches
    uint8_t dash_faulted   = !PHAL_readGPIO(DASH_NFLT_GPIO_Port, DASH_NFLT_Pin);
    uint8_t abox_faulted   = !PHAL_readGPIO(ABOX_NFLT_GPIO_Port, ABOX_NFLT_Pin);
    uint8_t main_faulted   = !PHAL_readGPIO(MAIN_NFLT_GPIO_Port, MAIN_NFLT_Pin);
    uint8_t vcrit_faulted  = !PHAL_readGPIO(CRIT_5V_NFLT_GPIO_Port, CRIT_5V_NFLT_Pin);
    uint8_t vnc_faulted    = !PHAL_readGPIO(TV_NFLT_GPIO_Port, TV_NFLT_Pin);
    uint8_t dlfr_faulted   = !PHAL_readGPIO(DLFR_NFLT_GPIO_Port, DLFR_NFLT_Pin);
    uint8_t dlbk_faulted   = !PHAL_readGPIO(DLBK_NFLT_GPIO_Port, DLBK_NFLT_Pin);
    uint8_t bullet_faulted = !PHAL_readGPIO(BLT_NFLT_GPIO_Port, BLT_NFLT_Pin);
    uint8_t fan5v_faulted  = !PHAL_readGPIO(FAN_5V_NFLT_GPIO_Port, FAN_5V_NFLT_Pin);
    (void) fan5v_faulted; // not used for now

    static uint8_t fault_num = 0;
    // Set fault - this is too much for our 1ms window, so send each fault separately
    switch (fault_num++) {
        case 0:
            update_fault(FAULT_ID_DASH_RAIL, dash_faulted);
            LED_control(
                LED_DASH,
                is_latched(FAULT_ID_DASH_RAIL) ? LED_BLINK : LED_OFF
            );
            break;
        case 1:
            update_fault(FAULT_ID_ABOX_RAIL, abox_faulted);
            LED_control(
                LED_ABOX,
                is_latched(FAULT_ID_ABOX_RAIL) ? LED_BLINK : LED_OFF
            );
            break;
        case 2:
            update_fault(FAULT_ID_MAIN_RAIL, main_faulted);
            LED_control(
                LED_MAIN,
                is_latched(FAULT_ID_MAIN_RAIL) ? LED_BLINK : LED_OFF
            );
            break;
        case 3:
            update_fault(FAULT_ID_V_CRIT, vcrit_faulted);
            LED_control(
                LED_5V_CRIT,
                is_latched(FAULT_ID_V_CRIT) ? LED_BLINK : LED_OFF
            );
            break;
        case 4:
            update_fault(FAULT_ID_V_NONCRIT, vnc_faulted);
            LED_control(
                LED_TV,
                is_latched(FAULT_ID_V_NONCRIT) ? LED_BLINK : LED_OFF
            );
            break;
        case 5:
            update_fault(FAULT_ID_FRONT_DRIVELINE_RAIL, dlfr_faulted);
            LED_control(
                LED_DLFR,
                is_latched(FAULT_ID_FRONT_DRIVELINE_RAIL) ? LED_BLINK : LED_OFF
            );
            break;
        case 6:
            update_fault(FAULT_ID_REAR_DRIVELINE_RAIL, dlbk_faulted);
            LED_control(
                LED_DLBK,
                is_latched(FAULT_ID_REAR_DRIVELINE_RAIL) ? LED_BLINK : LED_OFF
            );
            break;
        case 7:
            update_fault(FAULT_ID_BULLET_RAIL, bullet_faulted);
            LED_control(
                LED_BLT,
                is_latched(FAULT_ID_BULLET_RAIL) ? LED_BLINK : LED_OFF
            );
            fault_num = 0;
            break;
    }
}
