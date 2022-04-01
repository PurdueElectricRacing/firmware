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
#define ID_FRONT_DRIVELINE_HB 0x4001902
#define ID_REAR_DRIVELINE_HB 0x4001942
#define ID_FRONT_WHEEL_DATA 0x4000002
#define ID_REAR_WHEEL_DATA 0x4000042
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000282
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c2
#define ID_TORQUE_REQUEST_MAIN 0x4000040
#define ID_MAIN_STATUS 0x4001900
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_FRONT_DRIVELINE_HB 2
#define DLC_REAR_DRIVELINE_HB 2
#define DLC_FRONT_WHEEL_DATA 8
#define DLC_REAR_WHEEL_DATA 8
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 6
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_MAIN_STATUS 3
/* END AUTO DLC DEFS */
extern uint32_t last_can_rx_time_ms;
// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_FRONT_DRIVELINE_HB(queue, front_left_motor_, front_right_motor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_DRIVELINE_HB, .DLC=DLC_FRONT_DRIVELINE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_driveline_hb.front_left_motor = front_left_motor_;\
        data_a->front_driveline_hb.front_right_motor = front_right_motor_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_DRIVELINE_HB(queue, back_left_motor_, back_right_motor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_DRIVELINE_HB, .DLC=DLC_REAR_DRIVELINE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_driveline_hb.back_left_motor = back_left_motor_;\
        data_a->rear_driveline_hb.back_right_motor = back_right_motor_;\
        qSendToBack(&queue, &msg);\
    } while(0)
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
#define SEND_FRONT_MOTOR_CURRENTS_TEMPS(queue, left_current_, right_current_, left_temp_, right_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_MOTOR_CURRENTS_TEMPS, .DLC=DLC_FRONT_MOTOR_CURRENTS_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_motor_currents_temps.left_current = left_current_;\
        data_a->front_motor_currents_temps.right_current = right_current_;\
        data_a->front_motor_currents_temps.left_temp = left_temp_;\
        data_a->front_motor_currents_temps.right_temp = right_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_MOTOR_CURRENTS_TEMPS(queue, left_, right_, left_temp_, right_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_CURRENTS_TEMPS, .DLC=DLC_REAR_MOTOR_CURRENTS_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_currents_temps.left = left_;\
        data_a->rear_motor_currents_temps.right = right_;\
        data_a->rear_motor_currents_temps.left_temp = left_temp_;\
        data_a->rear_motor_currents_temps.right_temp = right_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_TORQUE_REQUEST_MAIN 15
#define UP_MAIN_STATUS 100
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    FRONT_LEFT_MOTOR_DISCONNECTED,
    FRONT_LEFT_MOTOR_SETTING_PARAMS,
    FRONT_LEFT_MOTOR_CONNECTED,
    FRONT_LEFT_MOTOR_ERROR,
} front_left_motor_t;

typedef enum {
    FRONT_RIGHT_MOTOR_DISCONNECTED,
    FRONT_RIGHT_MOTOR_SETTING_PARAMS,
    FRONT_RIGHT_MOTOR_CONNECTED,
    FRONT_RIGHT_MOTOR_ERROR,
} front_right_motor_t;

typedef enum {
    BACK_LEFT_MOTOR_DISCONNECTED,
    BACK_LEFT_MOTOR_SETTING_PARAMS,
    BACK_LEFT_MOTOR_CONNECTED,
    BACK_LEFT_MOTOR_ERROR,
} back_left_motor_t;

typedef enum {
    BACK_RIGHT_MOTOR_DISCONNECTED,
    BACK_RIGHT_MOTOR_SETTING_PARAMS,
    BACK_RIGHT_MOTOR_CONNECTED,
    BACK_RIGHT_MOTOR_ERROR,
} back_right_motor_t;

typedef enum {
    CAR_STATE_INIT,
    CAR_STATE_PRECHARGING,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
} car_state_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t front_left_motor: 8;
        uint64_t front_right_motor: 8;
    } front_driveline_hb;
    struct {
        uint64_t back_left_motor: 8;
        uint64_t back_right_motor: 8;
    } rear_driveline_hb;
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
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } front_motor_currents_temps;
    struct {
        uint64_t left: 16;
        uint64_t right: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } rear_motor_currents_temps;
    struct {
        uint64_t front_left: 16;
        uint64_t front_right: 16;
        uint64_t rear_left: 16;
        uint64_t rear_right: 16;
    } torque_request_main;
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
        int16_t front_left;
        int16_t front_right;
        int16_t rear_left;
        int16_t rear_right;
        uint8_t stale;
        uint32_t last_rx;
    } torque_request_main;
    struct {
        car_state_t car_state;
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