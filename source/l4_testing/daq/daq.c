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
#include "daq.h"

typedef struct
{
    union {
    uint8_t data[8];
    uint64_t raw_data; // remember little endian
    };
    uint8_t curr_bit;
} daq_tx_frame_writer_t;

typedef struct
{
    uint64_t* raw_data_a;
    uint8_t curr_bit;
} daq_rx_frame_reader_t;

// TODO: has read / write func defined based on when you link the variable
// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=1, .bit_length=8},
    {.is_read_only=0, .bit_length=12},
};
// END AUTO VAR DEFS

// read variable
// returns true if error
bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
{
    if (tracked_vars[var_id].bit_length + 8 > 64 - tx_msg->curr_bit)
    {
        return true; // not enough space
    }
    // add var id
    tx_msg->raw_data |= ((uint64_t) var_id << tx_msg->curr_bit);
    tx_msg->curr_bit += 8;
    // TODO: add error flag?

    // get data
    uint64_t temp_storage = 0;
    if (tracked_vars[var_id].has_read_func)
    {
       tracked_vars[var_id].read_func_a(&temp_storage);
    }
    else
    {
        memcpy(&temp_storage, tracked_vars[var_id].read_var_a, (tracked_vars[var_id].bit_length + 7) / 8);
    }
    // place data
    tx_msg->raw_data |= (temp_storage << tx_msg->curr_bit);
    tx_msg->curr_bit += tracked_vars[var_id].bit_length;

    return false;
}

// TODO: be careful with bit shifts more than 32 (ensure thing being shifted is a uint64)

// write variable
// returns true if error
bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg)
{
    if (tracked_vars[var_id].is_read_only)
    {
        return true; // can't write to read only variable
        // TODO: add error flag?
    }

    rx_msg->curr_bit += DAQ_ID_LENGTH;

    // get data
    uint64_t temp_storage = (*(rx_msg->raw_data_a) >> rx_msg->curr_bit) & 
                            ~(0xFFFFFFFFFFFFFFFF << (tracked_vars[var_id].bit_length));
    // place data
    if (tracked_vars[var_id].has_write_func)
    {
        tracked_vars[var_id].write_func_a(&temp_storage);
    }
    else
    {
        memcpy(tracked_vars[var_id].write_var_a, &temp_storage, 
               (tracked_vars[var_id].bit_length + 7) / 8);
    }

    rx_msg->curr_bit += tracked_vars[var_id].bit_length;

    return false;
}

// send tx frame
void sendDaqFrame(daq_tx_frame_writer_t tx_frame)
{
    CanMsgTypeDef_t msg = {.IDE=1, .ExtId=ID_DAQ_RESPONSE_TEST_NODE, .DLC=(tx_frame.curr_bit + 7) / 8}; // rounding up
    memcpy(msg.Data, tx_frame.data, msg.DLC);
    PHAL_txCANMessage(&msg);
}

/**
 * process daq request
 * 2 bit command type: 0-read, 1-write, 2-save, 3-publish
 * followed by 6 bit var id
 * write is followed by variable data, length pre-specified for each var id
 * publish is followed by publish rate (in Hz), 0 = stop publish 
 */
void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a)
{

    daq_tx_frame_writer_t tx_writer = {.data={0}, .curr_bit=0};
    daq_rx_frame_reader_t rx_reader = {.curr_bit=0};
    rx_reader.raw_data_a = (uint64_t *) msg_header_a->Data;

    while (rx_reader.curr_bit <= msg_header_a->DLC * 8 - DAQ_COMMAND_LENGTH - DAQ_ID_LENGTH)
    {
        uint8_t command = (*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & 0x3;
        rx_reader.curr_bit += DAQ_COMMAND_LENGTH;
        switch(command)
        {
            case DAQ_COMMAND_READ:
                if(readVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & 0x3F, &tx_writer))
                {
                    // frame full, send and get a new one
                    sendDaqFrame(tx_writer);
                    tx_writer.curr_bit = 0;
                    tx_writer.raw_data = 0;
                    if (readVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & 0x3F, &tx_writer))
                    {
                        // TODO: error handling (var too large for one frame)
                        while (true) {
                            __asm__("nop");
                        }
                    }
                }
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_COMMAND_WRITE:
                if(writeVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & 0x3F, &rx_reader))
                {
                    // TODO: error handling (write to read only var)
                    while (true) {
                        __asm__("nop");
                    }
                }
                // writeVar increments the curr_bit internally
                break;
            case DAQ_COMMAND_SAVE:
                // TODO: write var to eeprom
                break;
            case DAQ_COMMAND_PUB:
                // TODO: start or stop publish
                break;
            default:
                __asm__("nop"); // Do nothing so we can place a breakpoint
        }
    }
    if (tx_writer.curr_bit > 0)
    {
        sendDaqFrame(tx_writer);
    }
}


// save to eeprom

// add to publish list

// stop publish

// publish task

void link_read_a(uint8_t id, void* addr)
{
    tracked_vars[id].read_var_a=addr;
    tracked_vars[id].has_read_func=0;
}

void link_read_func(uint8_t id, read_func_ptr_t read_func)
{
    tracked_vars[id].read_func_a=read_func;
    tracked_vars[id].has_read_func=1;
}

void link_write_a(uint8_t id, void* addr)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_var_a=addr;
        tracked_vars[id].has_write_func=0;
    }
}

void link_write_func(uint8_t id, write_func_ptr_t write_func)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_func_a=write_func;
        tracked_vars[id].has_write_func=1;
    }
}