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

#include "common/phal/can.h"

// BEGIN AUTO VAR INCLUDES
#include "orion.h"
#include "tmu.h"
#include "main.h"
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=0, .bit_length=1, .read_var_a=&charge_request_user, .write_var_a=&charge_request_user, },
    {.is_read_only=0, .bit_length=16, .read_var_a=&user_charge_current_request, .write_var_a=&user_charge_current_request, },
    {.is_read_only=0, .bit_length=16, .read_var_a=&user_charge_voltage_request, .write_var_a=&user_charge_voltage_request, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&tmu_daq_override, .write_var_a=&tmu_daq_override, },
    {.is_read_only=0, .bit_length=4, .read_var_a=&tmu_daq_therm, .write_var_a=&tmu_daq_therm, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_1_1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_1_2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_2_1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_2_2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_3_1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_3_2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_4_1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_4_2, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_5_1, .write_var_a=NULL, },
    {.is_read_only=1, .bit_length=12, .read_var_a=&adc_readings.tmu_5_2, .write_var_a=NULL, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&bms_daq_override, .write_var_a=&bms_daq_override, },
    {.is_read_only=0, .bit_length=1, .read_var_a=&bms_daq_stat, .write_var_a=&bms_daq_stat, },
    {.is_read_only=1, .bit_length=32, .read_var_a=&CAN1->ESR, .write_var_a=NULL, },
};
// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a) {
    // BEGIN AUTO INIT
    uint8_t ret = daqInitBase(tx_a, NUM_VARS, CAN1, ID_DAQ_RESPONSE_A_BOX_VCAN, tracked_vars);
    return ret;
    // END AUTO INIT
}

void daqPeriodic() {
    daqPeriodicBase();
}

// BEGIN AUTO CALLBACK DEF
void daq_command_A_BOX_VCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a)
{
	daq_command_callback(msg_header_a);
}
void daq_command_A_BOX_CCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a)
{
	daq_command_callback(msg_header_a);
}
// END AUTO CALLBACK DEF
