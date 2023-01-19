/**
 * @file daq.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2023-01-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "daq.h"
#include "common/daq/daq_base.h"
#include "common/phal_L4/can/can.h"

// BEGIN AUTO VAR INCLUDES
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {};
// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a)
{
    // BEGIN AUTO INIT
    daqInitBase(tx_a, NUM_VARS, hcan, ext_id, tracked_vars);
    // END AUTO INIT
}

void daqPeriodic()
{
    daqPeriodicBase();
}

// BEGIN AUTO CALLBACK DEF
void daq_command_DASHBOARD_CALLBACK(CanMsgTypeDef_t* msg_header_a)
// END AUTO CALLBACK DEF
{
    daq_command_callback(msg_header_a);
}