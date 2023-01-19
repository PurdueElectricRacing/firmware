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
#include "pedals.h"
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=1, .bit_length=12, .read_var_a=&raw_pedals.t1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&raw_pedals.t2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&raw_pedals.b1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&raw_pedals.b2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&raw_pedals.b3, .write_var_a=NULL, },
};
// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a)
{
    // BEGIN AUTO INIT
    daqInitBase(tx_a, NUM_VARS, CAN1, ID_DAQ_RESPONSE_DASHBOARD, tracked_vars);
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