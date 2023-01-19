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
#define NODE_NAME "Driveline"

#define DAQ_UPDATE_PERIOD 15 // ms

// BEGIN AUTO VAR COUNT
#define NUM_VARS 0
// END AUTO VAR COUNT

// BEGIN AUTO VAR IDs
// END AUTO VAR IDs

// BEGIN AUTO FILE STRUCTS
// END AUTO FILE STRUCTS

bool daqInit(q_handle_t* tx_a);
void daqPeriodic();

#endif