/**
 * @file auto_switch.h
 * @author Gavin Zyonse (gzyonse@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2023-11-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _AUTO_SWITCH_H_
#define _AUTO_SWITCH_H_

#include <stdint.h>
#include <source/pdu/main.h>

// Static variables
#define SHUNT_R 10000000

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
    SW_BLT,
    // 5V switches
    SW_CRIT_5V,
    SW_NCRIT_5V,
    SW_DAQ,
    SW_FAN_5V,

    // Number of switches
    NUM_SWITCHES
} switches_t;

// Structures
typedef struct {
    float in_24v;
    float out_5v;
    float out_3v3;
} voltage_t;

typedef struct {
    float fault_status[NUM_SWITCHES];
    float current[NUM_SWITCHES];
    voltage_t voltage;
} auto_switch_t;

extern auto_switch_t auto_switch;

// Function definitions
uint8_t faultStatus();
void getFaults();
void getCurrent();
void getVoltage();
void enableSwitch();

#endif