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
#define NODE_NAME "Main_Module"

#define DAQ_UPDATE_PERIOD 15 // ms

// BEGIN AUTO VAR COUNT
#define NUM_VARS 7
// END AUTO VAR COUNT

// BEGIN AUTO VAR IDs
#define DAQ_ID_DT_LITERS_P_MIN_X10 0
#define DAQ_ID_BAT_LITERS_P_MIN_X10 1
#define DAQ_ID_DT_FLOW_ERROR 2
#define DAQ_ID_DT_TEMP_ERROR 3
#define DAQ_ID_BAT_FLOW_ERROR 4
#define DAQ_ID_BAT_TEMP_ERROR 5
#define DAQ_ID_CAL_STEER_ANGLE 6
// END AUTO VAR IDs

// BEGIN AUTO FILE STRUCTS
// END AUTO FILE STRUCTS

bool daqInit(q_handle_t* tx_a);
void daqPeriodic();

#endif