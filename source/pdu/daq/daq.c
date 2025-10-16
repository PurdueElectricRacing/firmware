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
#include "common/phal/can.h"

// BEGIN AUTO VAR INCLUDES
#include "main.h"
// END AUTO VAR INCLUDES

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {
        .is_read_only = 1,
        .bit_length   = 12,
        .read_var_a   = &adc_readings.lv_24_v_sense,
        .write_var_a  = NULL,
    },
    {
        .is_read_only = 1,
        .bit_length   = 12,
        .read_var_a   = &adc_readings.lv_5_v_sense,
        .write_var_a  = NULL,
    },
    {
        .is_read_only = 1,
        .bit_length   = 12,
        .read_var_a   = &adc_readings.lv_3v3_v_sense,
        .write_var_a  = NULL,
    },
    {
        .is_read_only = 1,
        .bit_length   = 32,
        .read_var_a   = &CAN1->ESR,
        .write_var_a  = NULL,
    },
};

// END AUTO VAR DEFS

// BEGIN AUTO FILE DEFAULTS
// END AUTO FILE DEFAULTS

bool daqInit(q_handle_t* tx_a) {
    // BEGIN AUTO INIT
    uint8_t ret = daqInitBase(tx_a, NUM_VARS, CAN1, ID_DAQ_RESPONSE_PDU_VCAN, tracked_vars);
    return ret;
    // END AUTO INIT
}

void daqPeriodic() {
    daqPeriodicBase();
}

// BEGIN AUTO CALLBACK DEF
void daq_command_PDU_VCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a) {
    daq_command_callback(msg_header_a);
}

// END AUTO CALLBACK DEF
