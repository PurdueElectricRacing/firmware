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

// Used to represent a float as 32 bits
typedef union {
    float f;
    uint32_t u;
} FloatConvert_t;
#define FLOAT_TO_UINT32(float_) (((FloatConvert_t) float_).u)
#define UINT32_TO_FLOAT(uint32_) (((FloatConvert_t) ((uint32_t) uint32_)).f)



// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_FRONT_DRIVELINE_HB 0x4001903
#define ID_FRONT_WHEEL_DATA 0x4000003
#define ID_REAR_WHEEL_DATA 0x4000043
#define ID_REAR_MC_REQ 0x4000483
#define ID_REAR_POW_LIM_L 0x40004c3
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000283
#define ID_FAULT_SYNC_DRIVELINE 0x8ca83
#define ID_DAQ_RESPONSE_DRIVELINE 0x17ffffc3
#define ID_TORQUE_REQUEST_MAIN 0x4000041
#define ID_MAIN_HB 0x4001901
#define ID_ORION_INFO 0x140006b8
#define ID_ORION_CURRENTS_VOLTS 0x140006f8
#define ID_DRIVELINE_FRONT_BL_CMD 0x409c4fe
#define ID_DRIVELINE_REAR_BL_CMD 0x409c53e
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_FAULT_SYNC_DASHBOARD 0x8cb05
#define ID_FAULT_SYNC_PRECHARGE 0x8cac4
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8ca42
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_DRIVELINE 0x140000f2
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_FRONT_DRIVELINE_HB 6
#define DLC_FRONT_WHEEL_DATA 8
#define DLC_REAR_WHEEL_DATA 8
#define DLC_REAR_MC_REQ 8
#define DLC_REAR_POW_LIM_L 8
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 8
#define DLC_FAULT_SYNC_DRIVELINE 3
#define DLC_DAQ_RESPONSE_DRIVELINE 8
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_MAIN_HB 2
#define DLC_ORION_INFO 7
#define DLC_ORION_CURRENTS_VOLTS 4
#define DLC_DRIVELINE_FRONT_BL_CMD 5
#define DLC_DRIVELINE_REAR_BL_CMD 5
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_PRECHARGE 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_DRIVELINE 8
/* END AUTO DLC DEFS */
extern uint32_t last_can_rx_time_ms;
// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_FRONT_DRIVELINE_HB(queue, front_left_motor_, front_left_motor_link_, front_left_last_link_error_, front_right_motor_, front_right_motor_link_, front_right_last_link_error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_DRIVELINE_HB, .DLC=DLC_FRONT_DRIVELINE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_driveline_hb.front_left_motor = front_left_motor_;\
        data_a->front_driveline_hb.front_left_motor_link = front_left_motor_link_;\
        data_a->front_driveline_hb.front_left_last_link_error = front_left_last_link_error_;\
        data_a->front_driveline_hb.front_right_motor = front_right_motor_;\
        data_a->front_driveline_hb.front_right_motor_link = front_right_motor_link_;\
        data_a->front_driveline_hb.front_right_last_link_error = front_right_last_link_error_;\
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
#define SEND_REAR_MC_REQ(queue, left_cmd_, right_cmd_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MC_REQ, .DLC=DLC_REAR_MC_REQ, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_mc_req.left_cmd = FLOAT_TO_UINT32(left_cmd_);\
        data_a->rear_mc_req.right_cmd = FLOAT_TO_UINT32(right_cmd_);\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_POW_LIM_L(queue, T_, P_c_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_POW_LIM_L, .DLC=DLC_REAR_POW_LIM_L, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_pow_lim_l.T = FLOAT_TO_UINT32(T_);\
        data_a->rear_pow_lim_l.P_c = FLOAT_TO_UINT32(P_c_);\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FRONT_MOTOR_CURRENTS_TEMPS(queue, left_current_, right_current_, left_temp_, right_temp_, right_voltage_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FRONT_MOTOR_CURRENTS_TEMPS, .DLC=DLC_FRONT_MOTOR_CURRENTS_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->front_motor_currents_temps.left_current = left_current_;\
        data_a->front_motor_currents_temps.right_current = right_current_;\
        data_a->front_motor_currents_temps.left_temp = left_temp_;\
        data_a->front_motor_currents_temps.right_temp = right_temp_;\
        data_a->front_motor_currents_temps.right_voltage = right_voltage_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FAULT_SYNC_DRIVELINE(queue, idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_DRIVELINE, .DLC=DLC_FAULT_SYNC_DRIVELINE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_driveline.idx = idx_;\
        data_a->fault_sync_driveline.latched = latched_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_DRIVELINE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_DRIVELINE, .DLC=DLC_DAQ_RESPONSE_DRIVELINE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_DRIVELINE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_TORQUE_REQUEST_MAIN 15
#define UP_MAIN_HB 100
#define UP_FRONT_WHEEL_DATA 10
#define UP_REAR_WHEEL_DATA 10
#define UP_ORION_INFO 32
#define UP_ORION_CURRENTS_VOLTS 32
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    FRONT_LEFT_MOTOR_DISCONNECTED,
    FRONT_LEFT_MOTOR_CONNECTED,
    FRONT_LEFT_MOTOR_CONFIG,
    FRONT_LEFT_MOTOR_ERROR,
} front_left_motor_t;

typedef enum {
    FRONT_LEFT_MOTOR_LINK_DISCONNECTED,
    FRONT_LEFT_MOTOR_LINK_ATTEMPTING,
    FRONT_LEFT_MOTOR_LINK_VERIFYING,
    FRONT_LEFT_MOTOR_LINK_DELAY,
    FRONT_LEFT_MOTOR_LINK_CONNECTED,
    FRONT_LEFT_MOTOR_LINK_FAIL,
} front_left_motor_link_t;

typedef enum {
    FRONT_LEFT_LAST_LINK_ERROR_NONE,
    FRONT_LEFT_LAST_LINK_ERROR_NOT_SERIAL,
    FRONT_LEFT_LAST_LINK_ERROR_CMD_TIMEOUT,
    FRONT_LEFT_LAST_LINK_ERROR_GEN_TIMEOUT,
} front_left_last_link_error_t;

typedef enum {
    FRONT_RIGHT_MOTOR_DISCONNECTED,
    FRONT_RIGHT_MOTOR_CONNECTED,
    FRONT_RIGHT_MOTOR_CONFIG,
    FRONT_RIGHT_MOTOR_ERROR,
} front_right_motor_t;

typedef enum {
    FRONT_RIGHT_MOTOR_LINK_DISCONNECTED,
    FRONT_RIGHT_MOTOR_LINK_ATTEMPTING,
    FRONT_RIGHT_MOTOR_LINK_VERIFYING,
    FRONT_RIGHT_MOTOR_LINK_DELAY,
    FRONT_RIGHT_MOTOR_LINK_CONNECTED,
    FRONT_RIGHT_MOTOR_LINK_FAIL,
} front_right_motor_link_t;

typedef enum {
    FRONT_RIGHT_LAST_LINK_ERROR_NONE,
    FRONT_RIGHT_LAST_LINK_ERROR_NOT_SERIAL,
    FRONT_RIGHT_LAST_LINK_ERROR_CMD_TIMEOUT,
    FRONT_RIGHT_LAST_LINK_ERROR_GEN_TIMEOUT,
} front_right_last_link_error_t;

typedef enum {
    CAR_STATE_IDLE,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
    CAR_STATE_FAN_CTRL,
} car_state_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t front_left_motor: 8;
        uint64_t front_left_motor_link: 8;
        uint64_t front_left_last_link_error: 8;
        uint64_t front_right_motor: 8;
        uint64_t front_right_motor_link: 8;
        uint64_t front_right_last_link_error: 8;
    } front_driveline_hb;
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
        uint64_t left_cmd: 32;
        uint64_t right_cmd: 32;
    } rear_mc_req;
    struct {
        uint64_t T: 32;
        uint64_t P_c: 32;
    } rear_pow_lim_l;
    struct {
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
        uint64_t right_voltage: 16;
    } front_motor_currents_temps;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_driveline;
    struct {
        uint64_t daq_response: 64;
    } daq_response_DRIVELINE;
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
    struct {
        uint64_t discharge_enable: 1;
        uint64_t charge_enable: 1;
        uint64_t charger_safety: 1;
        uint64_t dtc_status: 1;
        uint64_t multi_input: 1;
        uint64_t always_on: 1;
        uint64_t is_ready: 1;
        uint64_t is_charging: 1;
        uint64_t multi_input_2: 1;
        uint64_t multi_input_3: 1;
        uint64_t reserved: 1;
        uint64_t multi_output_2: 1;
        uint64_t multi_output_3: 1;
        uint64_t multi_output_4: 1;
        uint64_t multi_enable: 1;
        uint64_t multi_output_1: 1;
        uint64_t pack_dcl: 16;
        uint64_t pack_ccl: 16;
        uint64_t pack_soc: 8;
    } orion_info;
    struct {
        uint64_t pack_current: 16;
        uint64_t pack_voltage: 16;
    } orion_currents_volts;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_front_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_rear_bl_cmd;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_dashboard;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_precharge;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_torque_vector;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_test_node;
    struct {
        uint64_t id: 16;
        uint64_t value: 1;
    } set_fault;
    struct {
        uint64_t id: 16;
    } return_fault_control;
    struct {
        uint64_t daq_command: 64;
    } daq_command_DRIVELINE;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
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
        uint8_t discharge_enable;
        uint8_t charge_enable;
        uint8_t charger_safety;
        uint8_t dtc_status;
        uint8_t multi_input;
        uint8_t always_on;
        uint8_t is_ready;
        uint8_t is_charging;
        uint8_t multi_input_2;
        uint8_t multi_input_3;
        uint8_t reserved;
        uint8_t multi_output_2;
        uint8_t multi_output_3;
        uint8_t multi_output_4;
        uint8_t multi_enable;
        uint8_t multi_output_1;
        uint16_t pack_dcl;
        uint16_t pack_ccl;
        uint8_t pack_soc;
        uint8_t stale;
        uint32_t last_rx;
    } orion_info;
    struct {
        int16_t pack_current;
        uint16_t pack_voltage;
        uint8_t stale;
        uint32_t last_rx;
    } orion_currents_volts;
    struct {
        uint8_t cmd;
        uint32_t data;
    } driveline_front_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } driveline_rear_bl_cmd;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_main_module;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_dashboard;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_precharge;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_torque_vector;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_test_node;
    struct {
        uint16_t id;
        uint8_t value;
    } set_fault;
    struct {
        uint16_t id;
    } return_fault_control;
    struct {
        uint64_t daq_command;
    } daq_command_DRIVELINE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_DRIVELINE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void driveline_front_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void driveline_rear_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void handleCallbacks(uint16_t id, bool latched);
extern void set_fault_daq(uint16_t id, bool value);
extern void return_fault_control(uint16_t id);
extern void send_fault(uint16_t id, bool latched);
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