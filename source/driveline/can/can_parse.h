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
#define ID_FRONT_DRIVELINE_HB 0x4001903
#define ID_REAR_DRIVELINE_HB 0x4001943
#define ID_FRONT_WHEEL_DATA 0x4000003
#define ID_REAR_WHEEL_DATA 0x4000043
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000283
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c3
#define ID_FRONT_MOTOR_INIT 0x14000303
#define ID_REAR_MOTOR_INIT 0x14000343
#define ID_TORQUE_REQUEST_MAIN 0x4000041
#define ID_MAIN_HB 0x4001901
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_FRONT_DRIVELINE_HB 2
#define DLC_REAR_DRIVELINE_HB 2
#define DLC_FRONT_WHEEL_DATA 8
#define DLC_REAR_WHEEL_DATA 8
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 6
#define DLC_FRONT_MOTOR_INIT 2
#define DLC_REAR_MOTOR_INIT 2
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_MAIN_HB 2
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
#define SEND_REAR_DRIVELINE_HB(queue, rear_left_motor_, rear_right_motor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_DRIVELINE_HB, .DLC=DLC_REAR_DRIVELINE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_driveline_hb.rear_left_motor = rear_left_motor_;\
        data_a->rear_driveline_hb.rear_right_motor = rear_right_motor_;\
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
#define SEND_REAR_MOTOR_CURRENTS_TEMPS(queue, left_current_, right_current_, left_temp_, right_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_CURRENTS_TEMPS, .DLC=DLC_REAR_MOTOR_CURRENTS_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_currents_temps.left_current = left_current_;\
        data_a->rear_motor_currents_temps.right_current = right_current_;\
        data_a->rear_motor_currents_temps.left_temp = left_temp_;\
        data_a->rear_motor_currents_temps.right_temp = right_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FRONT_MOTOR_INIT(queue, front_left_init_, front_right_init_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_MOTOR_INIT, .DLC=DLC_FRONT_MOTOR_INIT, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_motor_init.front_left_init = front_left_init_;\
        data_a->front_motor_init.front_right_init = front_right_init_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_MOTOR_INIT(queue, rear_left_init_, rear_right_init_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_INIT, .DLC=DLC_REAR_MOTOR_INIT, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_init.rear_left_init = rear_left_init_;\
        data_a->rear_motor_init.rear_right_init = rear_right_init_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_TORQUE_REQUEST_MAIN 15
#define UP_MAIN_HB 100
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    FRONT_LEFT_MOTOR_DISCONNECTED,
    FRONT_LEFT_MOTOR_INITIALIZING,
    FRONT_LEFT_MOTOR_CONNECTED,
    FRONT_LEFT_MOTOR_ERROR,
} front_left_motor_t;

typedef enum {
    FRONT_RIGHT_MOTOR_DISCONNECTED,
    FRONT_RIGHT_MOTOR_INITIALIZING,
    FRONT_RIGHT_MOTOR_CONNECTED,
    FRONT_RIGHT_MOTOR_ERROR,
} front_right_motor_t;

typedef enum {
    REAR_LEFT_MOTOR_DISCONNECTED,
    REAR_LEFT_MOTOR_INITIALIZING,
    REAR_LEFT_MOTOR_CONNECTED,
    REAR_LEFT_MOTOR_ERROR,
} rear_left_motor_t;

typedef enum {
    REAR_RIGHT_MOTOR_DISCONNECTED,
    REAR_RIGHT_MOTOR_INITIALIZAING,
    REAR_RIGHT_MOTOR_CONNECTED,
    REAR_RIGHT_MOTOR_ERROR,
} rear_right_motor_t;

typedef enum {
    FRONT_LEFT_INIT_STARTING,
    FRONT_LEFT_INIT_WAITING,
    FRONT_LEFT_INIT_FAILED,
    FRONT_LEFT_INIT_CONNECTED,
} front_left_init_t;

typedef enum {
    FRONT_RIGHT_INIT_STARTING,
    FRONT_RIGHT_INIT_WAITING,
    FRONT_RIGHT_INIT_FAILED,
    FRONT_RIGHT_INIT_CONNECTED,
} front_right_init_t;

typedef enum {
    REAR_LEFT_INIT_STARTING,
    REAR_LEFT_INIT_WAITING,
    REAR_LEFT_INIT_FAILED,
    REAR_LEFT_INIT_CONNECTED,
} rear_left_init_t;

typedef enum {
    REAR_RIGHT_INIT_STARTING,
    REAR_RIGHT_INIT_WAITING,
    REAR_RIGHT_INIT_FAILED,
    REAR_RIGHT_INIT_CONNECTED,
} rear_right_init_t;

typedef enum {
    CAR_STATE_INIT,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
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
        uint64_t rear_left_motor: 8;
        uint64_t rear_right_motor: 8;
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
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } rear_motor_currents_temps;
    struct {
        uint64_t front_left_init: 8;
        uint64_t front_right_init: 8;
    } front_motor_init;
    struct {
        uint64_t rear_left_init: 8;
        uint64_t rear_right_init: 8;
    } rear_motor_init;
    struct {
        uint64_t front_left: 16;
        uint64_t front_right: 16;
        uint64_t rear_left: 16;
        uint64_t rear_right: 16;
    } torque_request_main;
    struct {
        uint64_t car_state: 8;
        uint64_t precharge_state: 1;
    } main_hb;
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
        uint8_t precharge_state;
        uint8_t stale;
        uint32_t last_rx;
    } main_hb;
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