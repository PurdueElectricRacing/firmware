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
#define NODE_NAME "Dashboard"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_RAW_THROTTLE_BRAKE 0x14000285
#define ID_START_BUTTON 0x4000005
#define ID_MAIN_STATUS 0x4001900
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_RAW_THROTTLE_BRAKE 8
#define DLC_START_BUTTON 1
#define DLC_MAIN_STATUS 3
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_RAW_THROTTLE_BRAKE(queue, throttle0_, throttle1_, brake0_, brake1_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_THROTTLE_BRAKE, .DLC=DLC_RAW_THROTTLE_BRAKE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_throttle_brake.throttle0 = throttle0_;\
        data_a->raw_throttle_brake.throttle1 = throttle1_;\
        data_a->raw_throttle_brake.brake0 = brake0_;\
        data_a->raw_throttle_brake.brake1 = brake1_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_START_BUTTON(queue, start_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_START_BUTTON, .DLC=DLC_START_BUTTON, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->start_button.start = start_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_MAIN_STATUS 25
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t throttle0: 16;
        uint64_t throttle1: 16;
        uint64_t brake0: 16;
        uint64_t brake1: 16;
    } raw_throttle_brake;
    struct {
        uint64_t start: 1;
    } start_button;
    struct {
        uint64_t car_state: 8;
        uint64_t apps_state: 8;
        uint64_t precharge_state: 1;
    } main_status;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t car_state;
        uint8_t apps_state;
        uint8_t precharge_state;
        uint8_t stale;
        uint32_t last_rx;
    } main_status;
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