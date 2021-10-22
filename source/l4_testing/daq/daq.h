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
#include "common/eeprom/eeprom.h"
#include "can_parse.h"
// TODO: add templates
// TODO: define schema
// Make this match the node name within the daq_config.json
#define NODE_NAME "TEST_NODE"

typedef void (*read_func_ptr_t)(void* arg);
typedef void (*write_func_ptr_t)(void* arg);

// BEGIN AUTO VAR COUNT
#define NUM_VARS 5
// END AUTO VAR COUNT

#define EEPROM_ENABLED 1
#define EEPROM_SIZE    4000 // bytes
#define EEPROM_ADDR    0x50

#define DAQ_CMD_LENGTH    3 // bits
#define DAQ_CMD_MASK      0b111

#define DAQ_CMD_READ      0
#define DAQ_CMD_WRITE     1
#define DAQ_CMD_LOAD      2 
#define DAQ_CMD_SAVE      3 
#define DAQ_CMD_PUB_START 4 
#define DAQ_CMD_PUB_STOP  5

#define DAQ_ID_LENGTH     5 // bits
#define DAQ_ID_MASK       0b11111

// TODO: auto id
// BEGIN AUTO VAR IDs
#define DAQ_ID_TEST_VAR 0
#define DAQ_ID_TEST_VAR2 1
#define DAQ_ID_RED_ON 2
#define DAQ_ID_GREEN_ON 3
#define DAQ_ID_BLUE_ON 4
// END AUTO VAR IDs

typedef struct
{
    // read: either function returning var or pointer
    union {
        void* read_var_a;
        read_func_ptr_t read_func_a;
    };  
    // write: either function or pointer or none
    union {
        void* write_var_a;
        write_func_ptr_t write_func_a;
    };

    struct {
        uint8_t is_read_only:   1;
        uint8_t has_read_func:  1;
        uint8_t has_write_func: 1;
        uint8_t eeprom_enabled: 1;
    };

    uint8_t bit_length;
    char eeprom_label[NAME_SIZE];
    uint8_t eeprom_version;

} daq_variable_t;

extern daq_variable_t tracked_vars[NUM_VARS];


// TODO: document
bool daqInit();
void link_read_a(uint8_t id, void* addr);
void link_read_func(uint8_t id, read_func_ptr_t read_func);
void link_write_a(uint8_t id, void* addr);
void link_write_func(uint8_t id, write_func_ptr_t write_func);

extern void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a);

#endif