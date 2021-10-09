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
#define NODE_NAME "TEST_NODE"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_TEST_MSG 0x1400004c
#define ID_THROTTLE_BRAKE 0x1400028b
#define ID_WHEEL_SPEEDS 0x1400028a
#define ID_MOTOR_CURRENT 0x400008a
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_TEST_MSG 2
#define DLC_THROTTLE_BRAKE 4
#define DLC_WHEEL_SPEEDS 4
#define DLC_MOTOR_CURRENT 1
/* END AUTO DLC DEFS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_THROTTLE_BRAKE 5
#define UP_WHEEL_SPEEDS 15
#define UP_MOTOR_CURRENT 5
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t test_sig: 16;
    }test_msg;
    struct {
        uint64_t raw_throttle: 16;
        uint64_t raw_brake: 16;
    }throttle_brake;
    struct {
        uint64_t fl_speed: 8;
        uint64_t fr_speed: 8;
        uint64_t bl_speed: 8;
        uint64_t br_speed: 8;
    }wheel_speeds;
    struct {
        uint64_t current: 8;
    }motor_current;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t raw_throttle;
        uint16_t raw_brake;
        uint8_t stale;
        uint32_t last_rx;
    } throttle_brake;
    struct {
        uint8_t fl_speed;
        uint8_t fr_speed;
        uint8_t bl_speed;
        uint8_t br_speed;
        uint8_t stale;
        uint32_t last_rx;
    } wheel_speeds;
    struct {
        uint8_t current;
        uint8_t stale;
        uint32_t last_rx;
    } motor_current;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
/* END AUTO EXTERN CALLBACK */

/* BEGIN AUTO EXTERN RX IRQ */
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