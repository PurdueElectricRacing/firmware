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
#define NODE_NAME "Main_Module"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_MAIN_STATUS 0x4001901
#define ID_TORQUE_REQUEST_MAIN 0x4000041
#define ID_DAQ_RESPONSE_MAIN_MODULE 0x17ffffc1
#define ID_RAW_THROTTLE_BRAKE 0x14000285
#define ID_START_BUTTON 0x4000005
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000283
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c3
#define ID_FRONT_DRIVELINE_HB 0x4001903
#define ID_REAR_DRIVELINE_HB 0x4001943
#define ID_DASHBOARD_STATUS 0x4001905
#define ID_DAQ_COMMAND_MAIN_MODULE 0x14000072
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_STATUS 3
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_DAQ_RESPONSE_MAIN_MODULE 8
#define DLC_RAW_THROTTLE_BRAKE 3
#define DLC_START_BUTTON 1
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 6
#define DLC_FRONT_DRIVELINE_HB 2
#define DLC_REAR_DRIVELINE_HB 2
#define DLC_DASHBOARD_STATUS 1
#define DLC_DAQ_COMMAND_MAIN_MODULE 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_STATUS(queue, car_state_, apps_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_STATUS, .DLC=DLC_MAIN_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_status.car_state = car_state_;\
        data_a->main_status.apps_state = apps_state_;\
        data_a->main_status.precharge_state = precharge_state_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TORQUE_REQUEST_MAIN(queue, front_left_, front_right_, rear_left_, rear_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TORQUE_REQUEST_MAIN, .DLC=DLC_TORQUE_REQUEST_MAIN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->torque_request_main.front_left = front_left_;\
        data_a->torque_request_main.front_right = front_right_;\
        data_a->torque_request_main.rear_left = rear_left_;\
        data_a->torque_request_main.rear_right = rear_right_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_MAIN_MODULE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_MAIN_MODULE, .DLC=DLC_DAQ_RESPONSE_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_MAIN_MODULE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_RAW_THROTTLE_BRAKE 5
#define UP_FRONT_DRIVELINE_HB 100
#define UP_REAR_DRIVELINE_HB 100
#define UP_DASHBOARD_STATUS 100
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_INIT,
    CAR_STATE_PRECHARGING,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
} car_state_t;

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

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t car_state: 8;
        uint64_t apps_state: 8;
        uint64_t precharge_state: 1;
    } main_status;
    struct {
        uint64_t front_left: 16;
        uint64_t front_right: 16;
        uint64_t rear_left: 16;
        uint64_t rear_right: 16;
    } torque_request_main;
    struct {
        uint64_t daq_response: 64;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t throttle: 12;
        uint64_t brake: 12;
    } raw_throttle_brake;
    struct {
        uint64_t start: 1;
    } start_button;
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
        uint64_t front_left_motor: 8;
        uint64_t front_right_motor: 8;
    } front_driveline_hb;
    struct {
        uint64_t back_left_motor: 8;
        uint64_t back_right_motor: 8;
    } rear_driveline_hb;
    struct {
        uint64_t apps_faulted: 1;
        uint64_t bse_faulted: 1;
        uint64_t apps_brake_faulted: 1;
    } dashboard_status;
    struct {
        uint64_t daq_command: 64;
    } daq_command_MAIN_MODULE;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t throttle;
        uint16_t brake;
        uint8_t stale;
        uint32_t last_rx;
    } raw_throttle_brake;
    struct {
        uint8_t start;
    } start_button;
    struct {
        uint16_t left_current;
        uint16_t right_current;
        uint8_t left_temp;
        uint8_t right_temp;
    } front_motor_currents_temps;
    struct {
        uint16_t left;
        uint16_t right;
        uint8_t left_temp;
        uint8_t right_temp;
    } rear_motor_currents_temps;
    struct {
        front_left_motor_t front_left_motor;
        front_right_motor_t front_right_motor;
        uint8_t stale;
        uint32_t last_rx;
    } front_driveline_hb;
    struct {
        back_left_motor_t back_left_motor;
        back_right_motor_t back_right_motor;
        uint8_t stale;
        uint32_t last_rx;
    } rear_driveline_hb;
    struct {
        uint8_t apps_faulted;
        uint8_t bse_faulted;
        uint8_t apps_brake_faulted;
        uint8_t stale;
        uint32_t last_rx;
    } dashboard_status;
    struct {
        uint64_t daq_command;
    } daq_command_MAIN_MODULE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
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