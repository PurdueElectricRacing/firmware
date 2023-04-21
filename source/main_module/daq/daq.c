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
#include "cooling.h"
#include"main.h"
#include"car.h"
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=1, .bit_length=8, .read_var_a=&cooling.dt_liters_p_min_x10, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=8, .read_var_a=&cooling.bat_liters_p_min_x10, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=1, .read_var_a=&cooling.dt_flow_error, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=1, .read_var_a=&cooling.dt_temp_error, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=1, .read_var_a=&cooling.bat_flow_error, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=1, .read_var_a=&cooling.bat_temp_error, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=1, .has_read_func=1, .read_func_a=(read_func_ptr_t)calibrateSteeringAngle, .write_var_a=NULL, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&cooling.daq_override, .write_var_a=&cooling.daq_override, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&cooling.out_daq_req.dt_pump, .write_var_a=&cooling.out_daq_req.dt_pump, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&cooling.out_daq_req.bat_pump, .write_var_a=&cooling.out_daq_req.bat_pump, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&cooling.out_daq_req.bat_pump_aux, .write_var_a=&cooling.out_daq_req.bat_pump_aux, },
    {.is_read_only=0, .bit_length=8, .read_var_a=&cooling.out_daq_req.dt_fan_power, .write_var_a=&cooling.out_daq_req.dt_fan_power, },
    {.is_read_only=0, .bit_length=8, .read_var_a=&cooling.out_daq_req.bat_fan_power, .write_var_a=&cooling.out_daq_req.bat_fan_power, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&daq_buzzer, .write_var_a=&daq_buzzer, },
};
// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a)
{
    // BEGIN AUTO INIT
    uint8_t ret = daqInitBase(tx_a, NUM_VARS, CAN1, ID_DAQ_RESPONSE_MAIN_MODULE, tracked_vars);
    return ret;
    // END AUTO INIT
}

void daqPeriodic()
{
    daqPeriodicBase();
}

// BEGIN AUTO CALLBACK DEF
void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a)
// END AUTO CALLBACK DEF
{
    daq_command_callback(msg_header_a);
}