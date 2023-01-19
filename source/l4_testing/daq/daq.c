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
extern uint16_t my_counter;
extern uint16_t my_counter2;extern uint8_t charge_enable;
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=1, .bit_length=8, .read_var_a=&my_counter, .write_var_a=NULL, },
    {.is_read_only=0, .bit_length=12, .read_var_a=&my_counter2, .write_var_a=&my_counter2, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&charge_enable, .write_var_a=&charge_enable, },
    {.is_read_only=0, .bit_length=8, .read_var_a=&(config.blue_on), .write_var_a=&(config.blue_on), },
    {.is_read_only=0, .bit_length=8, .read_var_a=&(config.red_on), .write_var_a=&(config.red_on), },
    {.is_read_only=0, .bit_length=8, .read_var_a=&(config.green_on), .write_var_a=&(config.green_on), },
    {.is_read_only=0, .bit_length=32, .read_var_a=&(config.odometer), .write_var_a=&(config.odometer), },
    {.is_read_only=0, .bit_length=32, .read_var_a=&(config.charge_current), .write_var_a=&(config.charge_current), },
    {.is_read_only=0, .bit_length=32, .read_var_a=&(config.charge_voltage), .write_var_a=&(config.charge_voltage), },
    {.is_read_only=0, .bit_length=16, .read_var_a=&(config.trim), .write_var_a=&(config.trim), },
};
// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
config_t config = {
    .blue_on = 0,
    .red_on = 0,
    .green_on = 0,
    .odometer = 0,
    .charge_current = 0,
    .charge_voltage = 0,
    .trim = -12,
};
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a)
{
    // BEGIN AUTO INIT
    daqInitBase(tx_a, NUM_VARS, CAN1, ID_DAQ_RESPONSE_TEST_NODE, tracked_vars);
    mapMem((uint8_t *) &config, sizeof(config), "conf", 1);
    // END AUTO INIT
}

void daqPeriodic()
{
    daqPeriodicBase();
}

// BEGIN AUTO CALLBACK DEF
void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a)
// END AUTO CALLBACK DEF
{
    daq_command_callback(msg_header_a);
}