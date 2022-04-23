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

// used for creating a tx can frame
typedef struct
{
    union {
        uint8_t data[8];
        uint64_t raw_data; // remember little endian
    };
    uint8_t curr_bit;
} daq_tx_frame_writer_t;

// used for reading a rx can frame
typedef struct
{
    uint64_t* raw_data_a;
    uint8_t curr_bit;
} daq_rx_frame_reader_t;

// represents an eeprom save command
typedef struct
{
    uint8_t var_id;
    uint64_t data;
} e_save_request_t;

// ring buffer of eeprom save commands
typedef struct
{
    uint64_t data_buffer; // used for saving and loading data from eeprom
    e_save_request_t cmd_buffer[DAQ_SAVE_QUEUE_SIZE];
    uint8_t read_idx;
    uint8_t write_idx;
    uint8_t queue_current;
    uint8_t data_buff_lock; // prevents load during save process
} daq_eeprom_status_t;

daq_eeprom_status_t daq_e_stat = {.read_idx=0, .write_idx=0, .data_buff_lock=0, .queue_current=0};

// BEGIN AUTO VAR DEFS
daq_variable_t tracked_vars[NUM_VARS] = {
};
// END AUTO VAR DEFS

// function prototypes
bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg);
bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg);
bool saveVarRequest(uint8_t var_id, daq_tx_frame_writer_t* tx_msg);
bool loadVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg);
bool pubVarStart(uint8_t var_id, daq_rx_frame_reader_t* rx_msg);
bool pubVarStop(uint8_t var_id);
bool appendDataToFrame(daq_tx_frame_writer_t* tx_msg, uint64_t data, uint8_t bit_length);
void flushDaqFrame(daq_tx_frame_writer_t* tx_msg);
void sendDaqFrame(daq_tx_frame_writer_t tx_frame);
void daqeQueuePop();
void daqeQueuePush();

q_handle_t* q_tx_can_a;

bool daqInit(q_handle_t* tx_a, I2C_TypeDef *i2c)
{
    q_tx_can_a = tx_a;
    // check all variables have been linked
    for(uint8_t i = 0; i < NUM_VARS; i++)
    {
        if (tracked_vars[i].read_var_a == NULL ||
            (!tracked_vars[i].is_read_only &&
             tracked_vars[i].write_var_a == NULL))
        {
            return true;
        }
    }

    // setup eeprom
    if (EEPROM_ENABLED)
    {
        if (!i2c) return true;
        eepromInitialize(EEPROM_SIZE, EEPROM_ADDR, i2c);
        for(uint8_t i = 0; i < NUM_VARS; i++)
        {
            if (tracked_vars[i].eeprom_enabled)
            {
                if (!eepromLinkStruct(&daq_e_stat.data_buffer, (tracked_vars[i].bit_length + 7) / 8,
                                 tracked_vars[i].eeprom_label, tracked_vars[i].eeprom_version, 0))
                {
                    // structure recognized, okay to load var
                    // pull data from eeprom into eeprom data buffer
                    if (eepromLoadStruct(tracked_vars[i].eeprom_label)) return true;
                    // constrain to bit length
                    daq_e_stat.data_buffer &= ~(0xFFFFFFFFFFFFFFFF << (tracked_vars[i].bit_length));
                    // place data
                    if (tracked_vars[i].has_write_func)
                    {
                        tracked_vars[i].write_func_a(&daq_e_stat.data_buffer);
                    }
                    else
                    {
                        memcpy(tracked_vars[i].write_var_a, &daq_e_stat.data_buffer, 
                            (tracked_vars[i].bit_length + 7) / 8);
                    }
                }
                else
                {
                    // first time setting up var, put default into memory (blocking i2c command)
                    if (tracked_vars[i].has_read_func)
                    {
                        tracked_vars[i].read_func_a(&daq_e_stat.cmd_buffer[daq_e_stat.write_idx].data);
                    }
                    else
                    {
                        memcpy(&daq_e_stat.cmd_buffer[daq_e_stat.write_idx].data, tracked_vars[i].read_var_a, (tracked_vars[i].bit_length + 7) / 8);
                    }
                    eepromSaveStructBlocking(tracked_vars[i].eeprom_label);
                }
            }
        }
        eepromCleanHeaders();
    }

    return false;
}

// read variable
// returns true if error
bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
{
    if (tracked_vars[var_id].bit_length + DAQ_CMD_LENGTH + DAQ_ID_LENGTH > 64)
    {
        // var too large
        appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_READ_ERROR,
                        DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
        return true;
    }
    if (tracked_vars[var_id].bit_length + DAQ_CMD_LENGTH + DAQ_ID_LENGTH > 64 - tx_msg->curr_bit)
    {
        // make room so entire message will fit
        flushDaqFrame(tx_msg);
    }
    // add var id
    appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_READ, 
                      DAQ_CMD_LENGTH + DAQ_ID_LENGTH);

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

    // place data, based on error checking, should be within the same frame
    appendDataToFrame(tx_msg, temp_storage, tracked_vars[var_id].bit_length);

    return false;
}

// write variable
// returns true if error
bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg)
{
    if (tracked_vars[var_id].is_read_only)
    {
        // can't write to read only variable
        appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_WRITE_ERROR,
                        DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
        return true; 
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
// returns true if error
bool saveVarRequest(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
{
    if (!EEPROM_ENABLED || !tracked_vars[var_id].eeprom_enabled || daq_e_stat.queue_current >= DAQ_SAVE_QUEUE_SIZE)
    {
        appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_SAVE_ERROR,
                        DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
        return true;
    }

    daq_e_stat.cmd_buffer[daq_e_stat.write_idx].var_id = var_id;

    // load data into save command
    if (tracked_vars[var_id].has_read_func)
    {
       tracked_vars[var_id].read_func_a(&daq_e_stat.cmd_buffer[daq_e_stat.write_idx].data);
    }
    else
    {
        memcpy(&daq_e_stat.cmd_buffer[daq_e_stat.write_idx].data, tracked_vars[var_id].read_var_a, (tracked_vars[var_id].bit_length + 7) / 8);
    }

    daqeQueuePush();

    return false;
}

// load from eeprom
// returns true if error
bool loadVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
{
    if (!EEPROM_ENABLED || !tracked_vars[var_id].eeprom_enabled || tracked_vars[var_id].is_read_only ||
       daq_e_stat.queue_current > 0 || daq_e_stat.data_buff_lock)
    {
        appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_LOAD_ERROR,
                        DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
        return true;
    }

    daq_e_stat.data_buff_lock = 1;

    // pull data from eeprom into eeprom data buffer
    if (eepromLoadStruct(tracked_vars[var_id].eeprom_label)){
        appendDataToFrame(tx_msg, (var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_LOAD_ERROR,
                        DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
        daq_e_stat.data_buff_lock = 0;
        return true;
    } 

    // constrain to bit length
    daq_e_stat.data_buffer &= ~(0xFFFFFFFFFFFFFFFF << (tracked_vars[var_id].bit_length));

    // place data
    if (tracked_vars[var_id].has_write_func)
    {
        tracked_vars[var_id].write_func_a(&daq_e_stat.data_buffer);
    }
    else
    {
        memcpy(tracked_vars[var_id].write_var_a, &daq_e_stat.data_buffer, 
               (tracked_vars[var_id].bit_length + 7) / 8);
    }

    daq_e_stat.data_buff_lock = 0;

    return false;
}

// add to publish list
// returns true if error
bool pubVarStart(uint8_t var_id, daq_rx_frame_reader_t* rx_msg)
{
    rx_msg->curr_bit += DAQ_ID_LENGTH;

    // get period
    tracked_vars[var_id].pub_period = (*(rx_msg->raw_data_a) >> rx_msg->curr_bit) & 0xFF;
    rx_msg->curr_bit += 8;

    return false;
}

bool pubVarStop(uint8_t var_id)
{
    tracked_vars[var_id].pub_period = 0;
    return false;
}


/**
 * process daq request
 * 3 bit command type: read, write, load, save, start pub, stop pub
 * followed by 5 bit var id
 * write is followed by variable data, length pre-specified for each var id
 * publish is followed by publish rate (in Hz)
 */
// BEGIN AUTO CALLBACK DEF
void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a)
// END AUTO CALLBACK DEF
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
                readVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &tx_writer);
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_WRITE:
                writeVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &rx_reader, &tx_writer);
                // writeVar increments the curr_bit internally
                break;
            case DAQ_CMD_LOAD:
                loadVar((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &tx_writer);
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_SAVE:
                saveVarRequest((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &tx_writer);
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            case DAQ_CMD_PUB_START:
                pubVarStart((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK, &rx_reader);
                // pubVarStart increments the curr_bit internally
                break;
            case DAQ_CMD_PUB_STOP:
                pubVarStop((*(rx_reader.raw_data_a) >> rx_reader.curr_bit) & DAQ_ID_MASK);
                rx_reader.curr_bit += DAQ_ID_LENGTH;
                break;
            default:
                __asm__("nop"); // Do nothing so we can place a breakpoint
        }
    }
    flushDaqFrame(&tx_writer);
}

uint32_t daq_tick = 0;

// publish task, LIMIT TO 5ms minimum period (eeprom save constraint)
void daqPeriodic()
{
    daq_tx_frame_writer_t tx_msg = {.data={0}, .curr_bit=0};
    daq_tick += 1;

    // save command handling
    if (daq_e_stat.queue_current > 0)
    {
        // queue contains command, process
        if (!e_wr_stat.has_write_started && !daq_e_stat.data_buff_lock)
        {
            // write has not started: acquire lock, load eeprom buffer
            daq_e_stat.data_buff_lock = 1;
            daq_e_stat.data_buffer = daq_e_stat.cmd_buffer[daq_e_stat.read_idx].data;
        }
        eepromSaveStructPeriodic(tracked_vars[daq_e_stat.cmd_buffer[daq_e_stat.read_idx].var_id].eeprom_label);
        if (e_wr_stat.is_write_complete)
        {
            // respond with save complete message
            appendDataToFrame(&tx_msg, (daq_e_stat.cmd_buffer[daq_e_stat.read_idx].var_id << DAQ_CMD_LENGTH) | DAQ_RPLY_SAVE,
                            DAQ_CMD_LENGTH + DAQ_ID_LENGTH);
            // done with lock, pop off buff
            daq_e_stat.data_buff_lock = 0;
            daqeQueuePop();
        }
    }

    // publish
    for (uint8_t i = 0; i < NUM_VARS; i++)
    {
        if (tracked_vars[i].pub_period != 0 && 
            daq_tick % tracked_vars[i].pub_period == 0)
        {
            // read var and publish

            if (tracked_vars[i].bit_length + DAQ_CMD_LENGTH + DAQ_ID_LENGTH > 64 - tx_msg.curr_bit)
            {
                // make room so entire message will fit
                flushDaqFrame(&tx_msg);
            }

            // add var id
            appendDataToFrame(&tx_msg, (i << DAQ_CMD_LENGTH) | DAQ_RPLY_PUB, 
                            DAQ_CMD_LENGTH + DAQ_ID_LENGTH);

            // get data
            uint64_t temp_storage = 0;
            if (tracked_vars[i].has_read_func)
            {
            tracked_vars[i].read_func_a(&temp_storage);
            }
            else
            {
                memcpy(&temp_storage, tracked_vars[i].read_var_a, (tracked_vars[i].bit_length + 7) / 8);
            }

            // place data, based on error checking, should be within the same frame
            appendDataToFrame(&tx_msg, temp_storage, tracked_vars[i].bit_length);
        }
    }

    flushDaqFrame(&tx_msg);
}

// add data to a tx frame
// returns true if errror (i.e. bit_length too long for one frame)
bool appendDataToFrame(daq_tx_frame_writer_t* tx_msg, uint64_t data, uint8_t bit_length)
{
    // data too long?
    if (bit_length > 64)
    {
        return true;
    }
    // is there enough room?
    if (bit_length > 64 - tx_msg->curr_bit)
    {
        flushDaqFrame(tx_msg);
    }

    tx_msg->raw_data |= ((data & ~(0xFFFFFFFFFFFFFFFF << bit_length)) << tx_msg->curr_bit);
    tx_msg->curr_bit += bit_length;

    return false;
}

// flush tx writer if not empty
void flushDaqFrame(daq_tx_frame_writer_t* tx_msg)
{
    if (tx_msg->curr_bit == 0)
    {
        return;
    }

    sendDaqFrame(*tx_msg);
    tx_msg->curr_bit = 0;
    tx_msg->curr_bit = 0;
    tx_msg->raw_data = 0;
}

// send tx frame
void sendDaqFrame(daq_tx_frame_writer_t tx_frame)
{
    CanMsgTypeDef_t msg = {.IDE=1, 
    // BEGIN AUTO DAQ SEND DEF
                           .Bus=CAN1,
                           .ExtId=ID_DAQ_RESPONSE_MAIN_MODULE,
    // END AUTO DAQ SEND DEF
                           .DLC=(tx_frame.curr_bit + 7) / 8}; // rounding up
    memcpy(msg.Data, tx_frame.data, msg.DLC);
    qSendToBack(q_tx_can_a, &msg);
}

// call after read op
void daqeQueuePop()
{
    daq_e_stat.read_idx += 1;
    if (daq_e_stat.read_idx >= DAQ_SAVE_QUEUE_SIZE)
    {
        daq_e_stat.read_idx = 0;
    }
    daq_e_stat.queue_current--;
}

// call after write op
void daqeQueuePush()
{
    if (daq_e_stat.queue_current < DAQ_SAVE_QUEUE_SIZE)
    {
        daq_e_stat.write_idx += 1;
        if (daq_e_stat.write_idx >= DAQ_SAVE_QUEUE_SIZE)
        {
            daq_e_stat.write_idx = 0;
        }
        daq_e_stat.queue_current++;
    }
}

void linkReada(uint8_t id, void* addr)
{
    tracked_vars[id].read_var_a = addr;
    tracked_vars[id].has_read_func = 0;
}

void linkReadFunc(uint8_t id, read_func_ptr_t read_func)
{
    tracked_vars[id].read_func_a = read_func;
    tracked_vars[id].has_read_func = 1;
}

void linkWritea(uint8_t id, void* addr)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_var_a = addr;
        tracked_vars[id].has_write_func = 0;
    }
}

void linkWriteFunc(uint8_t id, write_func_ptr_t write_func)
{
    if (!tracked_vars[id].is_read_only)
    {
        tracked_vars[id].write_func_a = write_func;
        tracked_vars[id].has_write_func = 1;
    }
}