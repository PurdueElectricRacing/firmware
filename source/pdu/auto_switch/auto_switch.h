/**
 * @file auto_switch.h
 * @author Gavin Zyonse (gzyonse@purdue.edu)
 * @brief
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef _AUTO_SWITCH_H_
#define _AUTO_SWITCH_H_

#include <stdint.h>
#include <stdbool.h>
#include <source/pdu/main.h>

// Static variables
#define SHUNT_R 10000000
#define ADC_MAX 4095

// Voltage sense resistors
#define LV_24V_R1  47000 // Ohms
#define LV_24V_R2  3400  // Ohms
#define LV_5V_R1   4300  // Ohms
#define LV_5V_R2   3400  // Ohms
#define LV_3V3_R1  4300  // Ohms
#define LV_3V3_R2  10000 // Ohms

// HP Current sense resistors
#define HP_CS_R1 180 // Ohms
#define HP_CS_R2 330 // Ohms
#define HP_CS_R3 500 // Ohms

// Upstream Current sense
#define HP_CS_R_SENSE 0.002 // Ohms
#define CS_GAIN 100


// Enumeration
typedef enum {
    // High power switches
    SW_PUMP_1,
    SW_PUMP_2,
    SW_SDC,
    SW_AUX,

    // Low power switches
    SW_FAN_1,
    SW_FAN_2,
    SW_DASH,
    SW_ABOX,
    SW_MAIN,

    // Not actually switches
    CS_24V,
    CS_5V,

    // Number of switches with CS signals (used for array bounds)
    // If switch has a current senese circuit, PLACE IT ABOVE THIS COMMENT
    CS_SWITCH_COUNT,

    // Low power switches (no CS)
    SW_BLT,
    // 5V switches (no CS)
    SW_CRIT_5V,
    SW_NCRIT_5V,
    SW_DAQ,
    SW_FAN_5V
} switches_t;

// Structures
typedef struct {
    uint16_t in_24v;
    uint16_t out_5v;
    uint16_t out_3v3;
} voltage_t;  // Voltage in mV

typedef struct {
    uint16_t current[CS_SWITCH_COUNT];  // Current in mA
    voltage_t voltage;
} auto_switches_t;

extern auto_switches_t auto_switches;

// Function declarations
/**
 * @brief Enable or disable a switch
 *
 * @param auto_switch_enum Switch to enable or disable
 * @param state 1 to enable, 0 to disable
*/
void setSwitch(switches_t auto_switch_enum, bool state);

/**
 * @brief Check if a switch is enabled
 *
 * @param auto_switch_enum Switch to check
 * @return 1 if enabled, 0 if disabled
*/
bool getSwitchStatus(switches_t auto_switch_enum);

/**
 * @brief Combined function that calls updateVoltage()
 *        and updateCurrent()
 *
 *        Voltage on each rail is stored at:
 *        auto_switches.voltage.in_24v
 *        auto_switches.voltage.out_5v
 *        auto_switches.voltage.out_3v3
 *
 *        Current through each switch is stored at:
 *        auto_switches.current[auto_switch_enum]
*/
void autoSwitchPeriodic();

/**
 * @brief Checks the status of various auto switches to ensure rails are ok. Sets fault and activates blink indicator if not..
 *
 */
void checkSwitchFaults();

#endif
