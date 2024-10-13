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
#define NODE_NAME "Rhenium"

// Used to represent a float as 32 bits
typedef union {
    float f;
    uint32_t u;
} FloatConvert_t;
#define FLOAT_TO_UINT32(float_) (((FloatConvert_t) float_).u)
#define UINT32_TO_FLOAT(uint32_) (((FloatConvert_t) ((uint32_t) uint32_)).f)

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_FAULT_SYNC_RHENIUM 0x8ca36
#define ID_DAQ_RESPONSE_RHENIUM 0x17fffff6
#define ID_FAULT_SYNC_PDU 0x8cb5f
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca41
#define ID_FAULT_SYNC_DASHBOARD 0x8cb05
#define ID_FAULT_SYNC_A_BOX 0x8ca84
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8caf7
#define ID_FAULT_SYNC_TEST_NODE 0x8cbbf
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_RHENIUM 0x14000db2
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_FAULT_SYNC_RHENIUM 3
#define DLC_DAQ_RESPONSE_RHENIUM 8
#define DLC_FAULT_SYNC_PDU 3
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_A_BOX 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_RHENIUM 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_FAULT_SYNC_RHENIUM(idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_RHENIUM, .DLC=DLC_FAULT_SYNC_RHENIUM, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_rhenium.idx = idx_;\
        data_a->fault_sync_rhenium.latched = latched_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_RHENIUM(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_RHENIUM, .DLC=DLC_DAQ_RESPONSE_RHENIUM, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_RHENIUM.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 5 / 2 // 5 / 2 would be 250% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_rhenium;
    struct {
        uint64_t daq_response: 64;
    } daq_response_RHENIUM;
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
    } daq_command_RHENIUM;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
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
    } daq_command_RHENIUM;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_RHENIUM_CALLBACK(CanMsgTypeDef_t* msg_header_a);
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