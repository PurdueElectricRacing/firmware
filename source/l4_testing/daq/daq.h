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

#include <string.h>
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/eeprom_spi/eeprom_spi.h"
#include "can_parse.h"

typedef void (*read_func_ptr_t)(void* arg);
typedef void (*write_func_ptr_t)(void* arg);

// Make this match the node name within the daq_config.json
#define NODE_NAME "TEST_NODE"

// BEGIN AUTO VAR COUNT
#define NUM_VARS 10
// END AUTO VAR COUNT

#define DAQ_UPDATE_PERIOD 15 // ms

#define DAQ_CMD_LENGTH    3 // bits
#define DAQ_CMD_MASK      0b111

typedef enum
{
    DAQ_CMD_READ      = 0,
    DAQ_CMD_WRITE     = 1,
    DAQ_CMD_LOAD      = 2,
    DAQ_CMD_SAVE      = 3, 
    DAQ_CMD_PUB_START = 4, 
    DAQ_CMD_PUB_STOP  = 5
} Daq_Cmd_t;

typedef enum
{
    DAQ_RPLY_READ        = 0,
    DAQ_RPLY_SAVE        = 1,
    DAQ_RPLY_READ_ERROR  = 2,
    DAQ_RPLY_WRITE_ERROR = 3,
    DAQ_RPLY_SAVE_ERROR  = 4,
    DAQ_RPLY_LOAD_ERROR  = 5,
    DAQ_RPLY_PUB         = 6
} Daq_Rply_t;

#define DAQ_ID_LENGTH     5 // bits
#define DAQ_ID_MASK       0b11111

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

typedef struct
{
    union {
        void* read_var_a;
        read_func_ptr_t read_func_a;
    };  
    // write: either function or pointer or none
    union {
        void* write_var_a;
        write_func_ptr_t write_func_a;
    };
    // flags
    struct {
        uint8_t is_read_only:   1;
        uint8_t has_read_func:  1;
        uint8_t has_write_func: 1;
    };
    uint8_t bit_length;
    // publishing
    uint8_t pub_period; // in terms of 15ms (i.e. 1 == 15ms period)
} daq_variable_t;

extern daq_variable_t tracked_vars[NUM_VARS];

// BEGIN AUTO FILE STRUCTS
typedef struct { __attribute__((packed))
    uint8_t   blue_on;
    uint8_t   red_on;
    uint8_t   green_on;
    float   odometer;
    float   charge_current;
    float   charge_voltage;
    int16_t   trim;
} config_t;

extern config_t config;

// END AUTO FILE STRUCTS

/**
 * @brief Call after linking read and write functions / addresses
 *        If the eeprom is enabled, configures the eeprom to contain
 *        the variables and loads default values if applicable
 * 
 * @param tx_a Address of the tx CAN buffer to send responses to
 * 
 * @return Returns false if the operation was successful
 */
bool daqInit(q_handle_t* tx_a);

/**
 * @brief Call periodically at DAQ_UPDATE_PERIOD
 *        Processes pending eeprom save commands 
 *        and publishes live variables
 */
void daqPeriodic();

/**
 * @brief              Callback for receiving a daq message,
 *                     automatically called by can_parse.c
 * 
 * @param msg_header_a Rx CAN message
 */
extern void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a);

#endif