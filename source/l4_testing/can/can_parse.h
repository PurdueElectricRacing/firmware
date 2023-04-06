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

#define NODE_NAME "TEST_NODE"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_ORION_CURRENTS_VOLTS2 0x140006ff
#define ID_PACK_CHARGE_STATUS2 0x80080bf
#define ID_TEST_MSG 0x1400007f
#define ID_TEST_MSG2 0x140000bf
#define ID_TEST_MSG3 0x140000ff
#define ID_TEST_MSG4 0x1400013f
#define ID_TEST_MSG5 0x1400017f
#define ID_WHEEL_SPEEDS 0xc0001ff
#define ID_ADC_VALUES 0x1234
#define ID_CAR_STATE 0xbeef420
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_DAQ_RESPONSE_TEST_NODE 0x17ffffff
#define ID_FRONT_DRIVELINE_HB 0x4001903
#define ID_TEST_MSG5_2 0x1400017d
#define ID_TEST_STALE 0x2222
#define ID_CAR_STATE2 0xbeef421
#define ID_L4_TESTING_BL_CMD 0x409c83e
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_FAULT_SYNC_DRIVELINE 0x8ca83
#define ID_FAULT_SYNC_DASHBOARD 0x8cb05
#define ID_FAULT_SYNC_PRECHARGE 0x8cac4
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8ca42
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_TEST_NODE 0x14000ff2
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_ORION_CURRENTS_VOLTS2 4
#define DLC_PACK_CHARGE_STATUS2 7
#define DLC_TEST_MSG 2
#define DLC_TEST_MSG2 2
#define DLC_TEST_MSG3 2
#define DLC_TEST_MSG4 2
#define DLC_TEST_MSG5 2
#define DLC_WHEEL_SPEEDS 8
#define DLC_ADC_VALUES 5
#define DLC_CAR_STATE 1
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_DAQ_RESPONSE_TEST_NODE 8
#define DLC_FRONT_DRIVELINE_HB 6
#define DLC_TEST_MSG5_2 8
#define DLC_TEST_STALE 1
#define DLC_CAR_STATE2 1
#define DLC_L4_TESTING_BL_CMD 5
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DRIVELINE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_PRECHARGE 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_TEST_NODE 8
/* END AUTO DLC DEFS */

// Used to represent a float as 32 bits
typedef union {
    float f;
    uint32_t u;
} FloatConvert_t;
#define FLOAT_TO_UINT32(float_) (((FloatConvert_t) float_).u)
#define UINT32_TO_FLOAT(uint32_) (((FloatConvert_t) ((uint32_t) uint32_)).f)

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_ORION_CURRENTS_VOLTS2(queue, pack_current_, pack_voltage_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ORION_CURRENTS_VOLTS2, .DLC=DLC_ORION_CURRENTS_VOLTS2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->orion_currents_volts2.pack_current = pack_current_;\
        data_a->orion_currents_volts2.pack_voltage = pack_voltage_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_CHARGE_STATUS2(queue, power_, charge_enable_, voltage_, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_CHARGE_STATUS2, .DLC=DLC_PACK_CHARGE_STATUS2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_charge_status2.power = power_;\
        data_a->pack_charge_status2.charge_enable = charge_enable_;\
        data_a->pack_charge_status2.voltage = voltage_;\
        data_a->pack_charge_status2.current = current_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_MSG(queue, test_sig_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG, .DLC=DLC_TEST_MSG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg.test_sig = test_sig_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_MSG2(queue, test_sig2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG2, .DLC=DLC_TEST_MSG2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg2.test_sig2 = test_sig2_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_MSG3(queue, test_sig3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG3, .DLC=DLC_TEST_MSG3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg3.test_sig3 = test_sig3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_MSG4(queue, test_sig4_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG4, .DLC=DLC_TEST_MSG4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg4.test_sig4 = test_sig4_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_MSG5(queue, test_sig5_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG5, .DLC=DLC_TEST_MSG5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg5.test_sig5 = test_sig5_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_WHEEL_SPEEDS(queue, left_speed_, right_speed_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_WHEEL_SPEEDS, .DLC=DLC_WHEEL_SPEEDS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->wheel_speeds.left_speed = FLOAT_TO_UINT32(left_speed_);\
        data_a->wheel_speeds.right_speed = FLOAT_TO_UINT32(right_speed_);\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_ADC_VALUES(queue, pot1_, pot2_, pot3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ADC_VALUES, .DLC=DLC_ADC_VALUES, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->adc_values.pot1 = pot1_;\
        data_a->adc_values.pot2 = pot2_;\
        data_a->adc_values.pot3 = pot3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CAR_STATE(queue, car_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CAR_STATE, .DLC=DLC_CAR_STATE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->car_state.car_state = car_state_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FAULT_SYNC_TEST_NODE(queue, idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_TEST_NODE, .DLC=DLC_FAULT_SYNC_TEST_NODE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_test_node.idx = idx_;\
        data_a->fault_sync_test_node.latched = latched_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_TEST_NODE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_TEST_NODE, .DLC=DLC_DAQ_RESPONSE_TEST_NODE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_TEST_NODE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period) in milliseconds*/
#define UP_FRONT_DRIVELINE_HB 100
#define UP_TEST_MSG5_2 15
#define UP_TEST_STALE 1000
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_READY2GO,
    CAR_STATE_FLIPPED,
    CAR_STATE_FLYING,
    CAR_STATE_LIGHTSPEED,
} car_state_t;

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
    CAR_STATE2_READY2GO,
    CAR_STATE2_FLIPPED,
    CAR_STATE2_FLYING,
    CAR_STATE2_LIGHTSPEED,
} car_state2_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t pack_current: 16;
        uint64_t pack_voltage: 16;
    } orion_currents_volts2;
    struct {
        uint64_t power: 16;
        uint64_t charge_enable: 1;
        uint64_t voltage: 16;
        uint64_t current: 16;
    } pack_charge_status2;
    struct {
        uint64_t test_sig: 16;
    } test_msg;
    struct {
        uint64_t test_sig2: 16;
    } test_msg2;
    struct {
        uint64_t test_sig3: 16;
    } test_msg3;
    struct {
        uint64_t test_sig4: 16;
    } test_msg4;
    struct {
        uint64_t test_sig5: 16;
    } test_msg5;
    struct {
        uint64_t left_speed: 32;
        uint64_t right_speed: 32;
    } wheel_speeds;
    struct {
        uint64_t pot1: 12;
        uint64_t pot2: 12;
        uint64_t pot3: 12;
    } adc_values;
    struct {
        uint64_t car_state: 8;
    } car_state;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_test_node;
    struct {
        uint64_t daq_response: 64;
    } daq_response_TEST_NODE;
    struct {
        uint64_t front_left_motor: 8;
        uint64_t front_left_motor_link: 8;
        uint64_t front_left_last_link_error: 8;
        uint64_t front_right_motor: 8;
        uint64_t front_right_motor_link: 8;
        uint64_t front_right_last_link_error: 8;
    } front_driveline_hb;
    struct {
        uint64_t test_sig5: 16;
        uint64_t test_sig5_2: 16;
        uint64_t test_sig5_3: 32;
    } test_msg5_2;
    struct {
        uint64_t data: 8;
    } test_stale;
    struct {
        uint64_t car_state2: 8;
    } car_state2;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } l4_testing_bl_cmd;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_driveline;
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
        uint64_t id: 16;
        uint64_t value: 1;
    } set_fault;
    struct {
        uint64_t id: 16;
    } return_fault_control;
    struct {
        uint64_t daq_command: 64;
    } daq_command_TEST_NODE;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        front_left_motor_t front_left_motor;
        front_left_motor_link_t front_left_motor_link;
        front_left_last_link_error_t front_left_last_link_error;
        front_right_motor_t front_right_motor;
        front_right_motor_link_t front_right_motor_link;
        front_right_last_link_error_t front_right_last_link_error;
        uint8_t stale;
        uint32_t last_rx;
    } front_driveline_hb;
    struct {
        uint16_t test_sig5;
        int16_t test_sig5_2;
        float test_sig5_3;
        uint8_t stale;
        uint32_t last_rx;
    } test_msg5_2;
    struct {
        uint8_t data;
        uint8_t stale;
        uint32_t last_rx;
    } test_stale;
    struct {
        car_state2_t car_state2;
    } car_state2;
    struct {
        uint8_t cmd;
        uint32_t data;
    } l4_testing_bl_cmd;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_main_module;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_driveline;
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
        uint16_t id;
        uint8_t value;
    } set_fault;
    struct {
        uint16_t id;
    } return_fault_control;
    struct {
        uint64_t daq_command;
    } daq_command_TEST_NODE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_TEST_NODE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void l4_testing_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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
void canRxUpdate(void);

/**
 * @brief Process any rx message callbacks from the CAN Rx IRQ
 * 
 * @param rx rx data from message just recieved
 */
void canProcessRxIRQs(CanMsgTypeDef_t* rx);

#endif