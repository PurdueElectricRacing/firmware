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
#include "can_parse.h"
#include "common/eeprom/eeprom.h"

typedef void (*read_func_ptr_t)(void* arg);
typedef void (*write_func_ptr_t)(void* arg);

// Make this match the node name within the daq_config.json
#define NODE_NAME "Main_Module"

// BEGIN AUTO VAR COUNT
#define NUM_VARS 6
// END AUTO VAR COUNT

#define DAQ_UPDATE_PERIOD 15 // ms

#define EEPROM_ENABLED 0

#define EEPROM_SIZE    4000 // bytes
#define EEPROM_ADDR    0x50
#define DAQ_SAVE_QUEUE_SIZE 8

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
#define DAQ_ID_DT_LITERS_P_MIN_X10 0
#define DAQ_ID_BAT_LITERS_P_MIN_X10 1
#define DAQ_ID_DT_FLOW_ERROR 2
#define DAQ_ID_DT_TEMP_ERROR 3
#define DAQ_ID_BAT_FLOW_ERROR 4
#define DAQ_ID_BAT_TEMP_ERROR 5
// END AUTO VAR IDs

typedef struct
{
    // read: either function or pointer
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
        uint8_t eeprom_enabled: 1;
    };
    uint8_t bit_length;
    // eeprom configuration
    char eeprom_label[NAME_SIZE];
    uint8_t eeprom_version;
    // publishing
    uint8_t pub_period; // in terms of 15ms (i.e. 1 == 15ms period)
} daq_variable_t;

extern daq_variable_t tracked_vars[NUM_VARS];

/**
 * @brief Call after linking read and write functions / addresses
 *        If the eeprom is enabled, configures the eeprom to contain
 *        the variables and loads default values if applicable
 * 
 * @param tx_a Address of the tx CAN buffer to send responses to
 * @param i2c  I2C Peripheral for eeprom, leave NULL if eeprom disabled
 * 
 * @return Returns false if the operation was successful
 */
bool daqInit(q_handle_t* tx_a, I2C_TypeDef *i2c);

/**
 * @brief Call periodically at DAQ_UPDATE_PERIOD
 *        Processes pending eeprom save commands 
 *        and publishes live variables
 */
void daqPeriodic();

/**
 * @brief      Links an address to read a variable at
 * 
 * @param id   Variable id
 * @param addr Address to read the variable at
 */
void linkReada(uint8_t id, void* addr);

/**
 * @brief           Links a function to read a variable
 * 
 * @param id        Variable id
 * @param read_func Read function of type read_func_ptr_t
 */
void linkReadFunc(uint8_t id, read_func_ptr_t read_func);

/**
 * @brief      Links an address to write to a variable
 * 
 * @param id   Variable id
 * @param addr Address to write to the variable
 */
void linkWritea(uint8_t id, void* addr);

/**
 * @brief            Links a function to write to a variable
 * 
 * @param id         Variable id 
 * @param write_func Write function of type write_func_ptr_t 
 */
void linkWriteFunc(uint8_t id, write_func_ptr_t write_func);

/**
 * @brief              Callback for receiving a daq message,
 *                     automatically called by can_parse.c
 * 
 * @param msg_header_a Rx CAN message
 */
extern void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a);

#endif