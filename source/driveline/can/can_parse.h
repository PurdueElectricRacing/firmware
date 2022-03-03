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
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "Driveline"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_FRONT_WHEEL_DATA 0x4000002
#define ID_REAR_WHEEL_DATA 0x4000042
#define ID_FRONT_MOTOR_CURRENTS 0xc000282
#define ID_REAR_MOTOR_CURRENTS 0xc0002c2
#define ID_TORQUE_REQUEST 0x4000041
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_FRONT_WHEEL_DATA 8
#define DLC_REAR_WHEEL_DATA 8
#define DLC_FRONT_MOTOR_CURRENTS 4
#define DLC_REAR_MOTOR_CURRENTS 4
#define DLC_TORQUE_REQUEST 6
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_FRONT_WHEEL_DATA(queue, left_speed_, right_speed_, left_normal_, right_normal_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_WHEEL_DATA, .DLC=DLC_FRONT_WHEEL_DATA, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_wheel_data.left_speed = left_speed_;\
        data_a->front_wheel_data.right_speed = right_speed_;\
        data_a->front_wheel_data.left_normal = left_normal_;\
        data_a->front_wheel_data.right_normal = right_normal_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_WHEEL_DATA(queue, left_speed_, right_speed_, left_normal_, right_normal_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_WHEEL_DATA, .DLC=DLC_REAR_WHEEL_DATA, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_wheel_data.left_speed = left_speed_;\
        data_a->rear_wheel_data.right_speed = right_speed_;\
        data_a->rear_wheel_data.left_normal = left_normal_;\
        data_a->rear_wheel_data.right_normal = right_normal_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FRONT_MOTOR_CURRENTS(queue, left_, right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_MOTOR_CURRENTS, .DLC=DLC_FRONT_MOTOR_CURRENTS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_motor_currents.left = left_;\
        data_a->front_motor_currents.right = right_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_MOTOR_CURRENTS(queue, left_, right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_CURRENTS, .DLC=DLC_REAR_MOTOR_CURRENTS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_currents.left = left_;\
        data_a->rear_motor_currents.right = right_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_TORQUE_REQUEST 15
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t left_speed: 16;
        uint64_t right_speed: 16;
        uint64_t left_normal: 16;
        uint64_t right_normal: 16;
    } front_wheel_data;
    struct {
        uint64_t left_speed: 16;
        uint64_t right_speed: 16;
        uint64_t left_normal: 16;
        uint64_t right_normal: 16;
    } rear_wheel_data;
    struct {
        uint64_t left: 16;
        uint64_t right: 16;
    } front_motor_currents;
    struct {
        uint64_t left: 16;
        uint64_t right: 16;
    } rear_motor_currents;
    struct {
        uint64_t front_left: 12;
        uint64_t front_right: 12;
        uint64_t rear_left: 12;
        uint64_t rear_right: 12;
    } torque_request;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t front_left;
        uint16_t front_right;
        uint16_t rear_left;
        uint16_t rear_right;
        uint8_t stale;
        uint32_t last_rx;
    } torque_request;
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