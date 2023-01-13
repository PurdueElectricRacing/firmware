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
#define ID_MAIN_HB 0x4001901
#define ID_TORQUE_REQUEST_MAIN 0x4000041
#define ID_FAULT_SYNC_MAIN_MODULE 0xc001e41
#define ID_DAQ_RESPONSE_MAIN_MODULE 0x17ffffc1
#define ID_FAULT_SYNC_L4_TESTING 0xc001eff
#define ID_FAULT_SYNC_DASHBOARD 0xc001e45
#define ID_FAULT_SYNC_DRIVELINE 0xc001e83
#define ID_FAULT_SYNC_TORQUE_VECTOR 0xc001e82
#define ID_FAULT_SYNC_PRECHARGE 0xc001e84
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_RAW_THROTTLE_BRAKE 0x14000285
#define ID_START_BUTTON 0x4000005
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000283
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c3
#define ID_FRONT_DRIVELINE_HB 0x4001903
#define ID_REAR_DRIVELINE_HB 0x4001943
#define ID_DASHBOARD_HB 0x4001905
#define ID_MAX_CELL_TEMP 0x404e604
#define ID_FRONT_WHEEL_DATA 0x4000003
#define ID_REAR_WHEEL_DATA 0x4000043
#define ID_LWS_STANDARD 0x2b0
#define ID_DAQ_COMMAND_MAIN_MODULE 0x14000072
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_HB 2
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_DAQ_RESPONSE_MAIN_MODULE 8
#define DLC_FAULT_SYNC_L4_TESTING 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_DRIVELINE 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_PRECHARGE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_RAW_THROTTLE_BRAKE 3
#define DLC_START_BUTTON 1
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 6
#define DLC_FRONT_DRIVELINE_HB 2
#define DLC_REAR_DRIVELINE_HB 2
#define DLC_DASHBOARD_HB 1
#define DLC_MAX_CELL_TEMP 2
#define DLC_FRONT_WHEEL_DATA 8
#define DLC_REAR_WHEEL_DATA 8
#define DLC_LWS_STANDARD 5
#define DLC_DAQ_COMMAND_MAIN_MODULE 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_HB(queue, car_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_HB, .DLC=DLC_MAIN_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_hb.car_state = car_state_;\
        data_a->main_hb.precharge_state = precharge_state_;\
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
#define SEND_FAULT_SYNC_MAIN_MODULE(queue, idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_MAIN_MODULE, .DLC=DLC_FAULT_SYNC_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_main_module.idx = idx_;\
        data_a->fault_sync_main_module.latched = latched_;\
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
#define UP_RAW_THROTTLE_BRAKE 15
#define UP_FRONT_MOTOR_CURRENTS_TEMPS 500
#define UP_REAR_MOTOR_CURRENTS_TEMPS 500
#define UP_FRONT_DRIVELINE_HB 100
#define UP_REAR_DRIVELINE_HB 100
#define UP_DASHBOARD_HB 100
#define UP_FRONT_WHEEL_DATA 10
#define UP_REAR_WHEEL_DATA 10
#define UP_LWS_STANDARD 10
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_INIT,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
} car_state_t;

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

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t car_state: 8;
        uint64_t precharge_state: 1;
    } main_hb;
    struct {
        uint64_t front_left: 16;
        uint64_t front_right: 16;
        uint64_t rear_left: 16;
        uint64_t rear_right: 16;
    } torque_request_main;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
    struct {
        uint64_t daq_response: 64;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_l4_testing;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_dashboard;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_driveline;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_torque_vector;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_precharge;
    struct {
        uint64_t id: 16;
        uint64_t value: 1;
    } set_fault;
    struct {
        uint64_t id: 16;
    } return_fault_control;
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
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } rear_motor_currents_temps;
    struct {
        uint64_t front_left_motor: 8;
        uint64_t front_right_motor: 8;
    } front_driveline_hb;
    struct {
        uint64_t rear_left_motor: 8;
        uint64_t rear_right_motor: 8;
    } rear_driveline_hb;
    struct {
        uint64_t apps_faulted: 1;
        uint64_t bse_faulted: 1;
        uint64_t apps_brake_faulted: 1;
    } dashboard_hb;
    struct {
        uint64_t max_temp: 16;
    } max_cell_temp;
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
        uint64_t LWS_ANGLE: 16;
        uint64_t LWS_SPEED: 8;
        uint64_t Reserved_1: 8;
        uint64_t Reserved_2: 8;
    } LWS_Standard;
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
        uint16_t idx;
        uint8_t latched;
    } fault_sync_l4_testing;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_dashboard;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_driveline;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_torque_vector;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_precharge;
    struct {
        uint16_t id;
        uint8_t value;
    } set_fault;
    struct {
        uint16_t id;
    } return_fault_control;
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
        uint8_t stale;
        uint32_t last_rx;
    } front_motor_currents_temps;
    struct {
        uint16_t left_current;
        uint16_t right_current;
        uint8_t left_temp;
        uint8_t right_temp;
        uint8_t stale;
        uint32_t last_rx;
    } rear_motor_currents_temps;
    struct {
        front_left_motor_t front_left_motor;
        front_right_motor_t front_right_motor;
        uint8_t stale;
        uint32_t last_rx;
    } front_driveline_hb;
    struct {
        rear_left_motor_t rear_left_motor;
        rear_right_motor_t rear_right_motor;
        uint8_t stale;
        uint32_t last_rx;
    } rear_driveline_hb;
    struct {
        uint8_t apps_faulted;
        uint8_t bse_faulted;
        uint8_t apps_brake_faulted;
        uint8_t stale;
        uint32_t last_rx;
    } dashboard_hb;
    struct {
        uint16_t max_temp;
    } max_cell_temp;
    struct {
        uint16_t left_speed;
        uint16_t right_speed;
        uint16_t left_normal;
        uint16_t right_normal;
        uint8_t stale;
        uint32_t last_rx;
    } front_wheel_data;
    struct {
        uint16_t left_speed;
        uint16_t right_speed;
        uint16_t left_normal;
        uint16_t right_normal;
        uint8_t stale;
        uint32_t last_rx;
    } rear_wheel_data;
    struct {
        int16_t LWS_ANGLE;
        uint8_t LWS_SPEED;
        uint8_t Reserved_1;
        uint8_t Reserved_2;
        uint8_t stale;
        uint32_t last_rx;
    } LWS_Standard;
    struct {
        uint64_t daq_command;
    } daq_command_MAIN_MODULE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void fault_sync_l4_testing_CALLBACK(CanParsedData_t* msg_data_a);
extern void fault_sync_dashboard_CALLBACK(CanParsedData_t* msg_data_a);
extern void fault_sync_driveline_CALLBACK(CanParsedData_t* msg_data_a);
extern void fault_sync_torque_vector_CALLBACK(CanParsedData_t* msg_data_a);
extern void fault_sync_precharge_CALLBACK(CanParsedData_t* msg_data_a);
extern void set_fault_CALLBACK(CanParsedData_t* msg_data_a);
extern void return_fault_control_CALLBACK(CanParsedData_t* msg_data_a);
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