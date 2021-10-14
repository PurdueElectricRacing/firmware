/**
 * @file daq.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  //TODO: brief
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

// Make this match the node name within the daq_config.json
#define NODE_NAME "TEST_NODE"

typedef void (*read_func_ptr_t)(void* arg);
typedef void (*write_func_ptr_t)(void* arg);

// BEGIN AUTO VAR COUNT
#define NUM_VARS 2
// END AUTO VAR COUNT

#define DAQ_COMMAND_LENGTH 2 // bits
#define DAQ_COMMAND_READ  0
#define DAQ_COMMAND_WRITE 1
#define DAQ_COMMAND_SAVE  2
#define DAQ_COMMAND_PUB   3

#define DAQ_ID_LENGTH 6 // bits

// TODO: auto id
// BEGIN AUTO VAR IDs
#define DAQ_ID_TEST_VAR 0
#define DAQ_ID_TEST_VAR2 1
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
        uint8_t is_read_only: 1;
        uint8_t has_read_func: 1;
        uint8_t has_write_func: 1;
    };

    uint8_t bit_length;

} daq_variable_t;

extern daq_variable_t tracked_vars[NUM_VARS];


// TODO: document
void link_read_a(uint8_t id, void* addr);
void link_read_func(uint8_t id, read_func_ptr_t read_func);
void link_write_a(uint8_t id, void* addr);
void link_write_func(uint8_t id, write_func_ptr_t write_func);

extern void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a);

#endif