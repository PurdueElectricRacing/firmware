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
#include "common/phal_F4_F7/can/can.h"
#include "common/daq/can_parse_base.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "PDU"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_V_RAILS 0x1001045f
#define ID_RAIL_CURRENTS 0x1001049f
#define ID_PUMP_AND_FAN_CURRENT 0x100104df
#define ID_OTHER_CURRENTS 0x1001051f
#define ID_COOLANT_OUT 0x100008df
#define ID_FLOWRATES 0x1000089f
#define ID_PDU_CAN_STATS 0x1001631f
#define ID_FAULT_SYNC_PDU 0x8cb1f
#define ID_DAQ_RESPONSE_PDU_VCAN 0x17ffffdf
#define ID_PDU_BL_CMD 0x409c53e
#define ID_COOLING_DRIVER_REQUEST 0xc0002c5
#define ID_MAIN_HB 0xc001901
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_FAULT_SYNC_DASHBOARD 0x8cac5
#define ID_FAULT_SYNC_A_BOX 0x8ca44
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8cab7
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_PDU_VCAN 0x140007f2
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_V_RAILS 6
#define DLC_RAIL_CURRENTS 4
#define DLC_PUMP_AND_FAN_CURRENT 7
#define DLC_OTHER_CURRENTS 8
#define DLC_COOLANT_OUT 3
#define DLC_FLOWRATES 2
#define DLC_PDU_CAN_STATS 4
#define DLC_FAULT_SYNC_PDU 3
#define DLC_DAQ_RESPONSE_PDU_VCAN 8
#define DLC_PDU_BL_CMD 5
#define DLC_COOLING_DRIVER_REQUEST 5
#define DLC_MAIN_HB 2
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_A_BOX 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_PDU_VCAN 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_V_RAILS(in_24v_, out_5v_, out_3v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_V_RAILS, .DLC=DLC_V_RAILS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->v_rails.in_24v = in_24v_;\
        data_a->v_rails.out_5v = out_5v_;\
        data_a->v_rails.out_3v3 = out_3v3_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAIL_CURRENTS(i_24v_, i_5v_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAIL_CURRENTS, .DLC=DLC_RAIL_CURRENTS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rail_currents.i_24v = i_24v_;\
        data_a->rail_currents.i_5v = i_5v_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PUMP_AND_FAN_CURRENT(i_pump1_, i_pump2_, i_fan1_, i_fan2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PUMP_AND_FAN_CURRENT, .DLC=DLC_PUMP_AND_FAN_CURRENT, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pump_and_fan_current.i_pump1 = i_pump1_;\
        data_a->pump_and_fan_current.i_pump2 = i_pump2_;\
        data_a->pump_and_fan_current.i_fan1 = i_fan1_;\
        data_a->pump_and_fan_current.i_fan2 = i_fan2_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_OTHER_CURRENTS(i_sdc_, i_aux_, i_dash_, i_abox_, i_main_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_OTHER_CURRENTS, .DLC=DLC_OTHER_CURRENTS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->other_currents.i_sdc = i_sdc_;\
        data_a->other_currents.i_aux = i_aux_;\
        data_a->other_currents.i_dash = i_dash_;\
        data_a->other_currents.i_abox = i_abox_;\
        data_a->other_currents.i_main = i_main_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_COOLANT_OUT(bat_fan_, dt_fan_, bat_pump_, bat_pump_aux_, dt_pump_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_COOLANT_OUT, .DLC=DLC_COOLANT_OUT, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->coolant_out.bat_fan = bat_fan_;\
        data_a->coolant_out.dt_fan = dt_fan_;\
        data_a->coolant_out.bat_pump = bat_pump_;\
        data_a->coolant_out.bat_pump_aux = bat_pump_aux_;\
        data_a->coolant_out.dt_pump = dt_pump_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_FLOWRATES(battery_flowrate_, drivetrain_flowrate_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FLOWRATES, .DLC=DLC_FLOWRATES, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->flowrates.battery_flowrate = battery_flowrate_;\
        data_a->flowrates.drivetrain_flowrate = drivetrain_flowrate_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PDU_CAN_STATS(can_tx_overflow_, can_tx_fail_, can_rx_overflow_, can_rx_overrun_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PDU_CAN_STATS, .DLC=DLC_PDU_CAN_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pdu_can_stats.can_tx_overflow = can_tx_overflow_;\
        data_a->pdu_can_stats.can_tx_fail = can_tx_fail_;\
        data_a->pdu_can_stats.can_rx_overflow = can_rx_overflow_;\
        data_a->pdu_can_stats.can_rx_overrun = can_rx_overrun_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_FAULT_SYNC_PDU(idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_PDU, .DLC=DLC_FAULT_SYNC_PDU, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_pdu.idx = idx_;\
        data_a->fault_sync_pdu.latched = latched_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_PDU_VCAN(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_PDU_VCAN, .DLC=DLC_DAQ_RESPONSE_PDU_VCAN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_PDU_VCAN.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 5 / 2 // 5 / 2 would be 250% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_MAIN_HB 500
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_IDLE,
    CAR_STATE_PRECHARGING,
    CAR_STATE_ENERGIZED,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
    CAR_STATE_CONSTANT_TORQUE,
} car_state_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t in_24v: 16;
        uint64_t out_5v: 16;
        uint64_t out_3v3: 16;
    } v_rails;
    struct {
        uint64_t i_24v: 16;
        uint64_t i_5v: 16;
    } rail_currents;
    struct {
        uint64_t i_pump1: 16;
        uint64_t i_pump2: 16;
        uint64_t i_fan1: 12;
        uint64_t i_fan2: 12;
    } pump_and_fan_current;
    struct {
        uint64_t i_sdc: 12;
        uint64_t i_aux: 12;
        uint64_t i_dash: 12;
        uint64_t i_abox: 12;
        uint64_t i_main: 12;
    } other_currents;
    struct {
        uint64_t bat_fan: 8;
        uint64_t dt_fan: 8;
        uint64_t bat_pump: 1;
        uint64_t bat_pump_aux: 1;
        uint64_t dt_pump: 1;
    } coolant_out;
    struct {
        uint64_t battery_flowrate: 8;
        uint64_t drivetrain_flowrate: 8;
    } flowrates;
    struct {
        uint64_t can_tx_overflow: 8;
        uint64_t can_tx_fail: 8;
        uint64_t can_rx_overflow: 8;
        uint64_t can_rx_overrun: 8;
    } pdu_can_stats;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_pdu;
    struct {
        uint64_t daq_response: 64;
    } daq_response_PDU_VCAN;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } pdu_bl_cmd;
    struct {
        uint64_t dt_pump: 8;
        uint64_t dt_fan: 8;
        uint64_t batt_pump: 8;
        uint64_t batt_pump2: 8;
        uint64_t batt_fan: 8;
    } cooling_driver_request;
    struct {
        uint64_t car_state: 8;
        uint64_t precharge_state: 1;
    } main_hb;
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
    } daq_command_PDU_VCAN;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t cmd;
        uint32_t data;
    } pdu_bl_cmd;
    struct {
        uint8_t dt_pump;
        uint8_t dt_fan;
        uint8_t batt_pump;
        uint8_t batt_pump2;
        uint8_t batt_fan;
    } cooling_driver_request;
    struct {
        car_state_t car_state;
        uint8_t precharge_state;
        uint8_t stale;
        uint32_t last_rx;
    } main_hb;
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
    } daq_command_PDU_VCAN;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_PDU_VCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void pdu_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void cooling_driver_request_CALLBACK(CanParsedData_t* msg_data_a);
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
void initCANParse(void);

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

extern volatile uint32_t last_can_rx_time_ms;

#endif