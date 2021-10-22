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

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
    {.is_read_only=1, .bit_length=8, .eeprom_enabled=0},
    {.is_read_only=0, .bit_length=12, .eeprom_enabled=1, .eeprom_label="tv2", .eeprom_version=0},
    {.is_read_only=0, .bit_length=1, .eeprom_enabled=0},
    {.is_read_only=0, .bit_length=1, .eeprom_enabled=0},
    {.is_read_only=0, .bit_length=1, .eeprom_enabled=0},
};
// END AUTO VAR DEFS

uint64_t eeprom_data_buffer = 0; // used for saving and loading data from eeprom
                                 // useful because it allows for the linked read and write
                                 // functions or addresses to be used

// function prototypes
bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg);
bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg);
bool saveVar(uint8_t var_id);
bool loadVar(uint8_t var_id);
void sendDaqFrame(daq_tx_frame_writer_t tx_frame);

bool daqInit()
{
    // setup eeprom
    if(EEPROM_ENABLED)
    {
        eepromInitialize(EEPROM_SIZE, EEPROM_ADDR);
        for(int i = 0; i < NUM_VARS; i++)
        {
            if (tracked_vars[i].eeprom_enabled)
            {
                if (!eepromLinkStruct(&eeprom_data_buffer, (tracked_vars[i].bit_length + 7) / 8,
                                 tracked_vars[i].eeprom_label, tracked_vars[i].eeprom_version, 0))
                {
                    // structure recognized, okay to load var
                    loadVar(i);
                }
                else
                {
                    // first time setting up var, put default into memory
                    saveVar(i);
                }
            }
        }
        eepromCleanHeaders();
    }
    
}

// read variable
// returns true if error
bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
{
    if (tracked_vars[var_id].bit_length + DAQ_CMD_LENGTH + DAQ_ID_LENGTH > 64 - tx_msg->curr_bit)
    {
        return true; // not enough space
    }
    // add var id
    tx_msg->raw_data |= ((uint64_t) var_id << tx_msg->curr_bit) << DAQ_CMD_LENGTH;
    tx_msg->curr_bit += DAQ_CMD_LENGTH + DAQ_ID_LENGTH;
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

// save to eeprom
bool saveVar(uint8_t var_id)
{
    if(!EEPROM_ENABLED || !tracked_vars[var_id].eeprom_enabled) return true;

    // load data into eeprom buffer
    eeprom_data_buffer = 0;
    if (tracked_vars[var_id].has_read_func)
    {
       tracked_vars[var_id].read_func_a(&eeprom_data_buffer);
    }
    else
    {
        memcpy(&eeprom_data_buffer, tracked_vars[var_id].read_var_a, (tracked_vars[var_id].bit_length + 7) / 8);
    }

    // move data from eeprom buffer into eeprom
    if(eepromSaveStruct(tracked_vars[var_id].eeprom_label)) return true;

    return false;
}

// load from eeprom
bool loadVar(uint8_t var_id)
{
    if(!EEPROM_ENABLED || !tracked_vars[var_id].eeprom_enabled || tracked_vars[var_id].is_read_only) return true;

    // pull data from eeprom into eeprom data buffer
    if(eepromLoadStruct(tracked_vars[var_id].eeprom_label)) return true;

    // constrain to bit length
    eeprom_data_buffer &= ~(0xFFFFFFFFFFFFFFFF << (tracked_vars[var_id].bit_length));

    // place data
    if (tracked_vars[var_id].has_write_func)
    {
        tracked_vars[var_id].write_func_a(&eeprom_data_buffer);
    }
    else
    {
        memcpy(tracked_vars[var_id].write_var_a, &eeprom_data_buffer, 
               (tracked_vars[var_id].bit_length + 7) / 8);
    }

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
 * 3 bit command type: read, write, load, save, start pub, stop pub
 * followed by 5 bit var id
 * write is followed by variable data, length pre-specified for each var id
 * publish is followed by publish rate (in Hz), 0 = stop publish 
 */
void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a)
{

    daq_tx_frame_writer_t tx_writer = {.data={0}, .curr_bit=0};
    daq_rx_frame_reader_t rx_reader = {.curr_bit=0};
    rx_reader.raw_data_a = (uint64_t *) msg_header_a->Data;

    while (rx_reader.curr_bit <= msg_header_a->DLC * 8 - DAQ_CMD_LENGTH - DAQ_ID_LENGTH)
    {
        uint8_t command = (*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_CMD_MASK;
        rx_reader.curr_bit += DAQ_CMD_LENGTH;
        switch(command)
        {
            case DAQ_CMD_READ:
                if (readVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &tx_writer))
                {
                    // frame full, send and get a new one
                    sendDaqFrame(tx_writer);
                    tx_writer.curr_bit = 0;
                    tx_writer.raw_data = 0;
                    if (readVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &tx_writer))
                    {
                        // TODO: error handling (var too large for one frame)
                        while (true) {
                            __asm__("nop");
                        }
                    }
                }
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_WRITE:
                if (writeVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &rx_reader))
                {
                    // TODO: error handling (write to read only var)
                    while (true) {
                        __asm__("nop");
                    }
                }
                // writeVar increments the curr_bit internally
                break;
            case DAQ_CMD_LOAD:
                if (loadVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK))
                {
                    // TODO: error handling (eeprom load error)
                    while (true) {
                        __asm__("nop");
                    }
                }
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_SAVE:
                if (saveVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK))
                {
                    // TODO: error handling (eeprom load error)
                    while (true) {
                        __asm__("nop");
                    }
                }
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_PUB_START:
                // TODO: start publish
                break;
            case DAQ_CMD_PUB_STOP:
                // TODO: stop publish
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



// add to publish list

// stop publish

// publish task

void link_read_a(uint8_t id, void* addr)
{
    tracked_vars[id].read_var_a = addr;
    tracked_vars[id].has_read_func = 0;
}

void link_read_func(uint8_t id, read_func_ptr_t read_func)
{
    tracked_vars[id].read_func_a = read_func;
    tracked_vars[id].has_read_func = 1;
}

void link_write_a(uint8_t id, void* addr)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_var_a = addr;
        tracked_vars[id].has_write_func = 0;
    }
}

void link_write_func(uint8_t id, write_func_ptr_t write_func)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_func_a = write_func;
        tracked_vars[id].has_write_func = 1;
    }
}