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

// Make this match the node name within the can_config.json
#define NODE_NAME "daq"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_DAQ_CAN_STATS 0x10019531
#define ID_DAQ_QUEUE_STATS 0x10019571
#define ID_DAQ_COMMAND_MAIN_MODULE 0x14003231
#define ID_DAQ_COMMAND_DASHBOARD 0x14003271
#define ID_DAQ_COMMAND_A_BOX 0x140032b1
#define ID_DAQ_COMMAND_PDU 0x140032f1
#define ID_DAQ_RESPONSE_DAQ 0x14001a71
#define ID_UDS_COMMAND_MAIN_MODULE 0x18003231
#define ID_UDS_COMMAND_DASHBOARD 0x18003271
#define ID_UDS_COMMAND_A_BOX 0x180032b1
#define ID_UDS_COMMAND_PDU 0x180032f1
#define ID_UDS_COMMAND_TORQUE_VECTOR 0x18003331
#define ID_UDS_RESPONSE_DAQ 0x18001a7c
#define ID_DAQ_RESPONSE_MAIN_MODULE 0x14001901
#define ID_DAQ_RESPONSE_DASHBOARD 0x14001945
#define ID_DAQ_RESPONSE_A_BOX 0x14001984
#define ID_DAQ_RESPONSE_PDU 0x140019df
#define ID_DAQ_COMMAND_DAQ 0x14003372
#define ID_UDS_RESPONSE_MAIN_MODULE 0x1800193c
#define ID_UDS_RESPONSE_DASHBOARD 0x1800197c
#define ID_UDS_RESPONSE_A_BOX 0x180019bc
#define ID_UDS_RESPONSE_PDU 0x180019fc
#define ID_UDS_RESPONSE_TORQUE_VECTOR 0x18001a3c
#define ID_UDS_COMMAND_DAQ 0x18003372
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_DAQ_CAN_STATS 8
#define DLC_DAQ_QUEUE_STATS 8
#define DLC_DAQ_COMMAND_MAIN_MODULE 8
#define DLC_DAQ_COMMAND_DASHBOARD 8
#define DLC_DAQ_COMMAND_A_BOX 8
#define DLC_DAQ_COMMAND_PDU 8
#define DLC_DAQ_RESPONSE_DAQ 8
#define DLC_UDS_COMMAND_MAIN_MODULE 8
#define DLC_UDS_COMMAND_DASHBOARD 8
#define DLC_UDS_COMMAND_A_BOX 8
#define DLC_UDS_COMMAND_PDU 8
#define DLC_UDS_COMMAND_TORQUE_VECTOR 8
#define DLC_UDS_RESPONSE_DAQ 8
#define DLC_DAQ_RESPONSE_MAIN_MODULE 8
#define DLC_DAQ_RESPONSE_DASHBOARD 8
#define DLC_DAQ_RESPONSE_A_BOX 8
#define DLC_DAQ_RESPONSE_PDU 8
#define DLC_DAQ_COMMAND_DAQ 8
#define DLC_UDS_RESPONSE_MAIN_MODULE 8
#define DLC_UDS_RESPONSE_DASHBOARD 8
#define DLC_UDS_RESPONSE_A_BOX 8
#define DLC_UDS_RESPONSE_PDU 8
#define DLC_UDS_RESPONSE_TORQUE_VECTOR 8
#define DLC_UDS_COMMAND_DAQ 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_DAQ_CAN_STATS(can_tx_overflow_, can_tx_fail_, can_rx_overflow_, can_rx_overrun_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_CAN_STATS, .DLC=DLC_DAQ_CAN_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_can_stats.can_tx_overflow = can_tx_overflow_;\
        data_a->daq_can_stats.can_tx_fail = can_tx_fail_;\
        data_a->daq_can_stats.can_rx_overflow = can_rx_overflow_;\
        data_a->daq_can_stats.can_rx_overrun = can_rx_overrun_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_QUEUE_STATS(bcan_rx_overflow_, can1_rx_overflow_, sd_rx_overflow_, tcp_tx_overflow_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_QUEUE_STATS, .DLC=DLC_DAQ_QUEUE_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_queue_stats.bcan_rx_overflow = bcan_rx_overflow_;\
        data_a->daq_queue_stats.can1_rx_overflow = can1_rx_overflow_;\
        data_a->daq_queue_stats.sd_rx_overflow = sd_rx_overflow_;\
        data_a->daq_queue_stats.tcp_tx_overflow = tcp_tx_overflow_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_COMMAND_MAIN_MODULE(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_COMMAND_MAIN_MODULE, .DLC=DLC_DAQ_COMMAND_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_command_MAIN_MODULE.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_COMMAND_DASHBOARD(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_COMMAND_DASHBOARD, .DLC=DLC_DAQ_COMMAND_DASHBOARD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_command_DASHBOARD.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_COMMAND_A_BOX(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_COMMAND_A_BOX, .DLC=DLC_DAQ_COMMAND_A_BOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_command_A_BOX.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_COMMAND_PDU(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_COMMAND_PDU, .DLC=DLC_DAQ_COMMAND_PDU, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_command_PDU.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_DAQ(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_DAQ, .DLC=DLC_DAQ_RESPONSE_DAQ, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_DAQ.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_COMMAND_MAIN_MODULE(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_COMMAND_MAIN_MODULE, .DLC=DLC_UDS_COMMAND_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_command_main_module.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_COMMAND_DASHBOARD(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_COMMAND_DASHBOARD, .DLC=DLC_UDS_COMMAND_DASHBOARD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_command_dashboard.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_COMMAND_A_BOX(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_COMMAND_A_BOX, .DLC=DLC_UDS_COMMAND_A_BOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_command_a_box.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_COMMAND_PDU(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_COMMAND_PDU, .DLC=DLC_UDS_COMMAND_PDU, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_command_pdu.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_COMMAND_TORQUE_VECTOR(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_COMMAND_TORQUE_VECTOR, .DLC=DLC_UDS_COMMAND_TORQUE_VECTOR, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_command_torque_vector.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_RESPONSE_DAQ(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_RESPONSE_DAQ, .DLC=DLC_UDS_RESPONSE_DAQ, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_response_daq.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 30 / 2 // 5 / 2 would be 250% of period
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
        uint64_t can_tx_overflow: 16;
        uint64_t can_tx_fail: 16;
        uint64_t can_rx_overflow: 16;
        uint64_t can_rx_overrun: 16;
    } daq_can_stats;
    struct {
        uint64_t bcan_rx_overflow: 16;
        uint64_t can1_rx_overflow: 16;
        uint64_t sd_rx_overflow: 16;
        uint64_t tcp_tx_overflow: 16;
    } daq_queue_stats;
    struct {
        uint64_t payload: 64;
    } daq_command_MAIN_MODULE;
    struct {
        uint64_t payload: 64;
    } daq_command_DASHBOARD;
    struct {
        uint64_t payload: 64;
    } daq_command_A_BOX;
    struct {
        uint64_t payload: 64;
    } daq_command_PDU;
    struct {
        uint64_t payload: 64;
    } daq_response_DAQ;
    struct {
        uint64_t payload: 64;
    } uds_command_main_module;
    struct {
        uint64_t payload: 64;
    } uds_command_dashboard;
    struct {
        uint64_t payload: 64;
    } uds_command_a_box;
    struct {
        uint64_t payload: 64;
    } uds_command_pdu;
    struct {
        uint64_t payload: 64;
    } uds_command_torque_vector;
    struct {
        uint64_t payload: 64;
    } uds_response_daq;
    struct {
        uint64_t payload: 64;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t payload: 64;
    } daq_response_DASHBOARD;
    struct {
        uint64_t payload: 64;
    } daq_response_A_BOX;
    struct {
        uint64_t payload: 64;
    } daq_response_PDU;
    struct {
        uint64_t payload: 64;
    } daq_command_DAQ;
    struct {
        uint64_t payload: 64;
    } uds_response_main_module;
    struct {
        uint64_t payload: 64;
    } uds_response_dashboard;
    struct {
        uint64_t payload: 64;
    } uds_response_a_box;
    struct {
        uint64_t payload: 64;
    } uds_response_pdu;
    struct {
        uint64_t payload: 64;
    } uds_response_torque_vector;
    struct {
        uint64_t payload: 64;
    } uds_command_daq;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint64_t payload;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t payload;
    } daq_response_DASHBOARD;
    struct {
        uint64_t payload;
    } daq_response_A_BOX;
    struct {
        uint64_t payload;
    } daq_response_PDU;
    struct {
        uint64_t payload;
    } daq_command_DAQ;
    struct {
        uint64_t payload;
    } uds_response_main_module;
    struct {
        uint64_t payload;
    } uds_response_dashboard;
    struct {
        uint64_t payload;
    } uds_response_a_box;
    struct {
        uint64_t payload;
    } uds_response_pdu;
    struct {
        uint64_t payload;
    } uds_response_torque_vector;
    struct {
        uint64_t payload;
    } uds_command_daq;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_DAQ_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void uds_command_daq_CALLBACK(uint64_t payload);
extern void handleCallbacks(uint16_t id, bool latched);
extern void set_fault_daq(uint16_t id, bool value);
extern void return_fault_control(uint16_t id);
extern void send_fault(uint16_t id, bool latched);
/* END AUTO EXTERN CALLBACK */

/* BEGIN AUTO EXTERN RX IRQ */
/* END AUTO EXTERN RX IRQ */

bool initCANFilter(void);

#endif
