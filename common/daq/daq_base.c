/**
 * @file daq_base.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Embedded DAQ protocol meant to communicate with a PC dashboard over CAN
 * @version 0.1
 * @date 2021-10-06
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "common/daq/daq_base.h"

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

// local function prototypes
static bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg);
static bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg);
static bool saveFile(daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg);
static bool loadFile(daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg);
static bool pubVarStart(uint8_t var_id, daq_rx_frame_reader_t* rx_msg);
static bool pubVarStop(uint8_t var_id);
static bool appendDataToFrame(daq_tx_frame_writer_t* tx_msg, uint64_t data, uint8_t bit_length);
static void flushDaqFrame(daq_tx_frame_writer_t* tx_msg);
static void sendDaqFrame(daq_tx_frame_writer_t tx_frame);
static void daqeQueuePop();
static void daqeQueuePush();
static void v_memcpy(volatile void *dest, volatile void *src, size_t len);

static q_handle_t* q_tx_can_a;
static uint8_t num_vars;
static uint32_t ext_id;
static CAN_TypeDef* can;
static daq_variable_t* tracked_vars;

bool daqInitBase(q_handle_t* tx_a, uint8_t num_variables_, CAN_TypeDef* hcan, uint32_t ext_id_,
             daq_variable_t* tracked_vars_)
{
    q_tx_can_a = tx_a;
    num_vars = num_variables_;
    ext_id = ext_id_;
    can = hcan;
    tracked_vars = tracked_vars_;

    // check all variables have been linked
    for(uint8_t i = 0; i < num_vars; i++)
    {
        if (tracked_vars[i].read_var_a == NULL ||
            (!tracked_vars[i].is_read_only &&
             tracked_vars[i].write_var_a == NULL))
        {
            return true;
        }
    }

    // mapMem(&config, sizeof(config), "conf", 1);

    return false;
}

// read variable
// returns true if error
static bool readVar(uint8_t var_id, daq_tx_frame_writer_t* tx_msg)
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
        v_memcpy(&temp_storage, tracked_vars[var_id].read_var_a, (tracked_vars[var_id].bit_length + 7) / 8);
    }

    // place data, based on error checking, should be within the same frame
    appendDataToFrame(tx_msg, temp_storage, tracked_vars[var_id].bit_length);

    return false;
}

// write variable
// returns true if error
static bool writeVar(uint8_t var_id, daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg)
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
        v_memcpy(tracked_vars[var_id].write_var_a, &temp_storage,
               (tracked_vars[var_id].bit_length + 7) / 8);
    }

    rx_msg->curr_bit += tracked_vars[var_id].bit_length;

    return false;
}

// returns true if error
static bool loadFile(daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg)
{
    char file_name[NAME_LEN];
    rx_msg->curr_bit += DAQ_ID_LENGTH;
    for (uint8_t i = 0; i < NAME_LEN; ++i)
    {
        file_name[i] = (*(rx_msg->raw_data_a) >> rx_msg->curr_bit) & 0xFF;
        rx_msg->curr_bit += 8;
    }

    requestLoad(file_name);
    return false;
}

// returns true if error
static bool saveFile(daq_rx_frame_reader_t* rx_msg, daq_tx_frame_writer_t* tx_msg)
{
    char file_name[NAME_LEN];
    rx_msg->curr_bit += DAQ_ID_LENGTH;
    for (uint8_t i = 0; i < NAME_LEN; ++i)
    {
        file_name[i] = (*(rx_msg->raw_data_a) >> rx_msg->curr_bit) & 0xFF;
        rx_msg->curr_bit += 8;
    }

    requestFlush(file_name);
    return false;
}

// add to publish list
// returns true if error
static bool pubVarStart(uint8_t var_id, daq_rx_frame_reader_t* rx_msg)
{
    rx_msg->curr_bit += DAQ_ID_LENGTH;

    // get period
    tracked_vars[var_id].pub_period = (*(rx_msg->raw_data_a) >> rx_msg->curr_bit) & 0xFF;
    rx_msg->curr_bit += 8;

    return false;
}

static bool pubVarStop(uint8_t var_id)
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
void daq_command_callback(CanMsgTypeDef_t* msg_header_a)
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
            case DAQ_CMD_LOAD: // FORMAT: 4 character file name
                loadFile(&rx_reader, &tx_writer);
                // loadFile increments the curr_bit internally
                break;
            case DAQ_CMD_SAVE:
                saveFile(&rx_reader, &tx_writer);
                // saveFile increments the curr_bit internally
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

static uint32_t daq_tick = 0;

// publish task, LIMIT TO 5ms minimum period (eeprom save constraint)
void daqPeriodicBase()
{
    daq_tx_frame_writer_t tx_msg = {.data={0}, .curr_bit=0};
    daq_tick += 1;

    // publish
    for (uint8_t i = 0; i < num_vars; i++)
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
                v_memcpy(&temp_storage, tracked_vars[i].read_var_a, (tracked_vars[i].bit_length + 7) / 8);
            }

            // place data, based on error checking, should be within the same frame
            appendDataToFrame(&tx_msg, temp_storage, tracked_vars[i].bit_length);
        }
    }

    flushDaqFrame(&tx_msg);
}

// add data to a tx frame
// returns true if errror (i.e. bit_length too long for one frame)
static bool appendDataToFrame(daq_tx_frame_writer_t* tx_msg, uint64_t data, uint8_t bit_length)
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
static void flushDaqFrame(daq_tx_frame_writer_t* tx_msg)
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
static void sendDaqFrame(daq_tx_frame_writer_t tx_frame)
{
    CanMsgTypeDef_t msg = {.IDE=1,
                           .Bus=can,
                           .ExtId=ext_id,
                           .DLC=(tx_frame.curr_bit + 7) / 8}; // rounding up
    v_memcpy(msg.Data, tx_frame.data, msg.DLC);
    qSendToBack(q_tx_can_a, &msg);
}

static void v_memcpy(volatile void *dest, volatile void *src, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        ((volatile uint8_t *) dest)[i] = ((volatile uint8_t *)src)[i];
}