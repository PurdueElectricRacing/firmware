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

#include "common/can_library/faults_common.h"
#include "common/can_library/generated/PDU.h"
#include "common/phal/gpio.h"
#include "led.h"

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
    // High power switches
    auto_switches.current[SW_PUMP_1] = calcCurrent_HP(adc_readings.pump_1_imon);
    auto_switches.current[SW_PUMP_2] = calcCurrent_HP(adc_readings.pump_2_imon);
    auto_switches.current[SW_SDC]    = calcCurrent_HP(adc_readings.sdc_imon);
    auto_switches.current[SW_HXFAN]  = calcCurrent_HP(adc_readings.hxfan_imon);

    // Low power switches
    auto_switches.current[SW_FAN_1] = calcCurrent_LP(mux_readings[0]);
    auto_switches.current[SW_FAN_2] = calcCurrent_LP(mux_readings[1]);
    auto_switches.current[SW_FAN_3] = calcCurrent_LP(mux_readings[2]);
    auto_switches.current[SW_FAN_4] = calcCurrent_LP(mux_readings[3]);
    auto_switches.current[SW_DASH]  = calcCurrent_LP(adc_readings.dash_cs);
    auto_switches.current[SW_ABOX]  = calcCurrent_LP(adc_readings.abox_cs);
    auto_switches.current[SW_MAIN]  = calcCurrent_LP(adc_readings.main_cs);

    // Upstream CS
    calcCurrent_Total();
}

// Called periodically, Updates voltage for each rail in mV
void updateVoltage() {
    auto_switches.voltage.in_24v  = calcVoltage(adc_readings.v24_vs, LV_24V_R1, LV_24V_R2);
    auto_switches.voltage.out_5v  = calcVoltage(adc_readings.v5_vs, LV_5V_R1, LV_5V_R2);
    auto_switches.voltage.out_3v3 = calcVoltage(adc_readings.v3v3_vs, LV_3V3_R1, LV_3V3_R2);
    auto_switches.voltage.amk_24v =
        calcVoltage(MAX(mux_readings[4], mux_readings[5]), AMK_24V_R1, AMK_24V_R2);
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
        case SW_HXFAN:
            PHAL_writeGPIO(HXFAN_CTRL_GPIO_Port, HXFAN_CTRL_Pin, state);
            LED_control(LED_AUX_1, state);
            break;
        case SW_FAN_1:
            PHAL_writeGPIO(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin, state);
            LED_control(LED_FAN_1, state);
            break;
        case SW_FAN_2:
            PHAL_writeGPIO(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin, state);
            LED_control(LED_FAN_2, state);
            break;
        case SW_FAN_3:
            PHAL_writeGPIO(FAN_3_CTRL_GPIO_Port, FAN_3_CTRL_Pin, state);
            LED_control(LED_FAN_1, state);
            break;
        case SW_FAN_4:
            PHAL_writeGPIO(FAN_4_CTRL_GPIO_Port, FAN_4_CTRL_Pin, state);
            LED_control(LED_FAN_2, state);
            break;
        case SW_BLT:
            PHAL_writeGPIO(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin, state);
            LED_control(LED_BLT, state);
            break;
        case SW_CRIT_5V:
            LED_control(LED_5V_CRIT, state);
            break;
        case SW_MAIN:
            LED_control(LED_MAIN, state);
            break;
        case SW_ABOX:
            LED_control(LED_ABOX, state);
            break;
        case SW_DASH:
            LED_control(LED_DASH, state);
            break;
        case SW_TV:
            PHAL_writeGPIO(TV_CTRL_GPIO_Port, TV_CTRL_Pin, state);
            LED_control(LED_5V_NCRIT, state);
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
    uint8_t dash   = PHAL_readGPIO(DASH_NFLT_GPIO_Port, DASH_NFLT_Pin);
    uint8_t abox   = PHAL_readGPIO(ABOX_NFLT_GPIO_Port, ABOX_NFLT_Pin);
    uint8_t main   = PHAL_readGPIO(MAIN_NFLT_GPIO_Port, MAIN_NFLT_Pin);
    uint8_t daq    = PHAL_readGPIO(DAQ_NFLT_GPIO_Port, DAQ_NFLT_Pin);
    uint8_t vcrit  = PHAL_readGPIO(CRIT_5V_NFLT_GPIO_Port, CRIT_5V_NFLT_Pin);
    uint8_t vnc    = PHAL_readGPIO(TV_NFLT_GPIO_Port, TV_NFLT_Pin);
    uint8_t fan1   = PHAL_readGPIO(DLFR_NFLT_GPIO_Port, DLFR_NFLT_Pin);
    uint8_t fan2   = PHAL_readGPIO(DLBK_NFLT_GPIO_Port, DLBK_NFLT_Pin);
    uint8_t bullet = PHAL_readGPIO(BLT_NFLT_GPIO_Port, BLT_NFLT_Pin);

    static uint8_t dash_old   = 1;
    static uint8_t abox_old   = 1;
    static uint8_t main_old   = 1;
    static uint8_t daq_old    = 1;
    static uint8_t vcrit_old  = 1;
    static uint8_t vnc_old    = 1;
    static uint8_t fan1_old   = 1;
    static uint8_t fan2_old   = 1;
    static uint8_t bullet_old = 1;

    // Set Blink error for faulted switch
    if (!dash && dash_old) {
        LED_control(LED_DASH, LED_BLINK);
    }
    if (!abox && abox_old) {
        LED_control(LED_ABOX, LED_BLINK);
    }
    if (!main && main_old) {
        LED_control(LED_MAIN, LED_BLINK);
    }
    if (!daq && daq_old) {
        LED_control(LED_DAQ, LED_BLINK);
    }
    if (!vcrit && vcrit_old) {
        LED_control(LED_5V_CRIT, LED_BLINK);
    }
    if (!vnc && vnc_old) {
        LED_control(LED_5V_NCRIT, LED_BLINK);
    }
    if (!fan1 && fan1_old) {
        LED_control(LED_FAN_1, LED_BLINK);
    }
    if (!fan2 && fan2_old) {
        LED_control(LED_FAN_2, LED_BLINK);
    }
    if (!bullet && bullet_old) {
        LED_control(LED_BLT, LED_BLINK);
    }

    dash_old   = dash;
    abox_old   = abox;
    main_old   = main;
    daq_old    = daq;
    vcrit_old  = vcrit;
    vnc_old    = vnc;
    fan1_old   = fan1;
    fan2_old   = fan2;
    bullet_old = bullet;

    static uint8_t fault_num;
    // Set fault for dash/daq - this is too much for our 1ms window, so send each fault seperately
    switch (fault_num) {
        case 0:
            update_fault(FAULT_INDEX_PDU_DASH_RAIL, !dash);
            break;
        case 1:
            update_fault(FAULT_INDEX_PDU_ABOX_RAIL, !abox);
            break;
        case 2:
            update_fault(FAULT_INDEX_PDU_MAIN_RAIL, !main);
            break;
        case 3:
            update_fault(FAULT_INDEX_PDU_DAQ_RAIL, !daq);
            break;
        case 4:
            update_fault(FAULT_INDEX_PDU_V_CRIT, !vcrit);
            break;
        case 5:
            update_fault(FAULT_INDEX_PDU_V_NONCRIT, !vnc);
            break;
        case 6:
            update_fault(FAULT_INDEX_PDU_FAN1, !fan1);
            break;
        case 7:
            update_fault(FAULT_INDEX_PDU_FAN2, !fan2);
            break;
        case 8:
            update_fault(FAULT_INDEX_PDU_BULLET_RAIL, !bullet);
            fault_num = 0;
            break;
    }
}
