/**
 * @file can_parse.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief Parsing of CAN messages using auto-generated structures with bit-fields
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _CAN_PARSE_H_
#define _CAN_PARSE_H_

#include "common/queue/queue.h"
#include "common/phal_L4/can/can.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "TORQUE_VECTOR"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_TORQUE_REQUEST 0x4000049
#define ID_BITSTREAM_FLASH_STATUS 0x1909
#define ID_BITSTREAM_DATA 0x1000193f
#define ID_BITSTREAM_REQUEST 0x1000197f
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_TORQUE_REQUEST 6
#define DLC_BITSTREAM_FLASH_STATUS 1
#define DLC_BITSTREAM_DATA 8
#define DLC_BITSTREAM_REQUEST 5
/* END AUTO DLC DEFS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t front_left: 12;
        uint64_t front_right: 12;
        uint64_t rear_left: 12;
        uint64_t rear_right: 12;
    }torque_request;
    struct {
        uint64_t flash_success: 1;
        uint64_t flash_timeout_rx: 1;
    }bitstream_flash_status;
    struct {
        uint64_t word_0: 32;
        uint64_t word_1: 32;
    }bitstream_data;
    struct {
        uint64_t download_request: 1;
        uint64_t download_size: 32;
    }bitstream_request;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint32_t word_0;
        uint32_t word_1;
    } bitstream_data;
    struct {
        uint8_t download_request;
        uint32_t download_size;
    } bitstream_request;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void bitstream_request_CALLBACK(CanParsedData_t* msg_data_a);
/* END AUTO EXTERN CALLBACK */

/* BEGIN AUTO EXTERN RX IRQ */
extern void bitstream_data_IRQ(CanParsedData_t* msg_data_a);
/* END AUTO EXTERN RX IRQ */

/**
 * @brief Setup queue and message filtering
 * 
 * @param q_rx_can RX buffer of CAN messages
 */
void initCANParse(q_handle_t* q_rx_can_a);

/**
 * @brief Pull message off of rx buffer,
 *        update can_data struct,
 *        check for stale messages
 */
void canRxUpdate();

/**
 * @brief Process any rx message callbacks from the CAN Rx IRQ
 * 
 * @param rx rx data from message just recieved
 */
void canProcessRxIRQs(CanMsgTypeDef_t* rx);

#endif