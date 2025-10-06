/**
 * @file daq.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Embedded DAQ protocol meant to communicate with a PC dashboard over CAN
 * @version 0.1
 * @date 2021-10-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _DAQ_H_
#define _DAQ_H_

#include "can_parse.h"

// Make this match the node name within the daq_config.json
#define NODE_NAME "TEST_NODE"

#define DAQ_UPDATE_PERIOD 15 // ms

// BEGIN AUTO VAR COUNT
#define NUM_VARS 10
// END AUTO VAR COUNT

// BEGIN AUTO VAR IDs
#define DAQ_ID_TEST_VAR 0
#define DAQ_ID_TEST_VAR2 1
#define DAQ_ID_CHARGE_ENABLE 2
#define DAQ_ID_BLUE_ON 3
#define DAQ_ID_RED_ON 4
#define DAQ_ID_GREEN_ON 5
#define DAQ_ID_ODOMETER 6
#define DAQ_ID_CHARGE_CURRENT 7
#define DAQ_ID_CHARGE_VOLTAGE 8
#define DAQ_ID_TRIM 9
// END AUTO VAR IDs

// BEGIN AUTO FILE STRUCTS
typedef struct { 
    uint8_t   blue_on;
    uint8_t   red_on;
    uint8_t   green_on;
    float   odometer;
    float   charge_current;
    float   charge_voltage;
    int16_t   trim;
} __attribute__((packed)) config_t;

extern config_t config;

// END AUTO FILE STRUCTS

bool daqInit(q_handle_t* tx_a);
void daqPeriodic();

#endif