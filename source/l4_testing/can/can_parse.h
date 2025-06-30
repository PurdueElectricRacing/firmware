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
#include "main.h"

// Make this match the node name within the can_config.json

#define NODE_NAME "TEST_NODE"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_ORION_CURRENTS_VOLTS2 0x9a00000
#define ID_PACK_CHARGE_STATUS2 0x9a00200
#define ID_TEST_MSG 0x9a00400
#define ID_TEST_MSG2 0x9a00600
#define ID_TEST_MSG3 0x9a00800
#define ID_TEST_MSG4 0x9a00a00
#define ID_TEST_MSG5 0x9a00c00
#define ID_WHEEL_SPEEDS 0x9a00e00
#define ID_ADC_VALUES 0x1234
#define ID_CAR_STATE 0xbeef420
#define ID_FAULT_SYNC_TEST_NODE 0x1a01400
#define ID_DAQ_RESPONSE_TEST_NODE_TEST 0x9a01600
#define ID_TEST_MSG5_2 0x9c00800
#define ID_TEST_STALE 0x2222
#define ID_CAR_STATE2 0xbeef421
#define ID_L4_TESTING_BL_CMD 0x8000e00
#define ID_FAULT_SYNC_PDU 0x1601000
#define ID_FAULT_SYNC_MAIN_MODULE 0xc02200
#define ID_FAULT_SYNC_DASHBOARD 0xa01600
#define ID_FAULT_SYNC_A_BOX 0x1402400
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x401600
#define ID_SET_FAULT 0x8001600
#define ID_RETURN_FAULT_CONTROL 0x8001800
#define ID_DAQ_COMMAND_TEST_NODE_TEST 0x9e00000
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
#define DLC_DAQ_RESPONSE_TEST_NODE_TEST 8
#define DLC_TEST_MSG5_2 8
#define DLC_TEST_STALE 1
#define DLC_CAR_STATE2 1
#define DLC_L4_TESTING_BL_CMD 5
#define DLC_FAULT_SYNC_PDU 3
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_A_BOX 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_TEST_NODE_TEST 8
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
#define SEND_ORION_CURRENTS_VOLTS2(pack_current_, pack_voltage_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ORION_CURRENTS_VOLTS2, .DLC=DLC_ORION_CURRENTS_VOLTS2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->orion_currents_volts2.pack_current = pack_current_;\
        data_a->orion_currents_volts2.pack_voltage = pack_voltage_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PACK_CHARGE_STATUS2(power_, charge_enable_, voltage_, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_CHARGE_STATUS2, .DLC=DLC_PACK_CHARGE_STATUS2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_charge_status2.power = power_;\
        data_a->pack_charge_status2.charge_enable = charge_enable_;\
        data_a->pack_charge_status2.voltage = voltage_;\
        data_a->pack_charge_status2.current = current_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TEST_MSG(test_sig_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG, .DLC=DLC_TEST_MSG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg.test_sig = test_sig_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TEST_MSG2(test_sig2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG2, .DLC=DLC_TEST_MSG2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg2.test_sig2 = test_sig2_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TEST_MSG3(test_sig3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG3, .DLC=DLC_TEST_MSG3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg3.test_sig3 = test_sig3_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TEST_MSG4(test_sig4_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG4, .DLC=DLC_TEST_MSG4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg4.test_sig4 = test_sig4_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TEST_MSG5(test_sig5_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_MSG5, .DLC=DLC_TEST_MSG5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_msg5.test_sig5 = test_sig5_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_WHEEL_SPEEDS(left_speed_, right_speed_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_WHEEL_SPEEDS, .DLC=DLC_WHEEL_SPEEDS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->wheel_speeds.left_speed = FLOAT_TO_UINT32(left_speed_);\
        data_a->wheel_speeds.right_speed = FLOAT_TO_UINT32(right_speed_);\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_ADC_VALUES(pot1_, pot2_, pot3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ADC_VALUES, .DLC=DLC_ADC_VALUES, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->adc_values.pot1 = pot1_;\
        data_a->adc_values.pot2 = pot2_;\
        data_a->adc_values.pot3 = pot3_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAR_STATE(car_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CAR_STATE, .DLC=DLC_CAR_STATE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->car_state.car_state = car_state_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_FAULT_SYNC_TEST_NODE(idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_TEST_NODE, .DLC=DLC_FAULT_SYNC_TEST_NODE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_test_node.idx = idx_;\
        data_a->fault_sync_test_node.latched = latched_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_TEST_NODE_TEST(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_TEST_NODE_TEST, .DLC=DLC_DAQ_RESPONSE_TEST_NODE_TEST, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_TEST_NODE_TEST.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 5 / 2 // 5 / 2 would be 250% of period
/* BEGIN AUTO UP DEFS (Update Period) in milliseconds*/
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
    } daq_response_TEST_NODE_TEST;
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
    } fault_sync_pdu;
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
    } fault_sync_a_box;
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
    } daq_command_TEST_NODE_TEST;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
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
    } fault_sync_pdu;
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
    } fault_sync_a_box;
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
    } daq_command_TEST_NODE_TEST;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_TEST_NODE_TEST_CALLBACK(CanMsgTypeDef_t* msg_header_a);
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