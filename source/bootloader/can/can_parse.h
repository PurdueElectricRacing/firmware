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
#if defined(STM32F407xx) || defined(STM32F732xx)
#include "common/phal_F4_F7/can/can.h"
#else
#include "common/phal_L4/can/can.h"
#endif

// defined in main.c
extern void canTxSendToBack(CanMsgTypeDef_t *msg);

// Make this match the node name within the can_config.json
#define NODE_NAME "bootloader"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_UDS_RESPONSE_MAIN_MODULE 0x1800193c
#define ID_UDS_RESPONSE_DASHBOARD 0x1800197c
#define ID_UDS_RESPONSE_A_BOX 0x180019bc
#define ID_UDS_RESPONSE_PDU 0x180019fc
#define ID_BITSTREAM_DATA 0x400193e
#define ID_UDS_COMMAND_MAIN_MODULE 0x18003231
#define ID_UDS_COMMAND_DASHBOARD 0x18003271
#define ID_UDS_COMMAND_A_BOX 0x180032b1
#define ID_UDS_COMMAND_PDU 0x180032f1
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_UDS_RESPONSE_MAIN_MODULE 8
#define DLC_UDS_RESPONSE_DASHBOARD 8
#define DLC_UDS_RESPONSE_A_BOX 8
#define DLC_UDS_RESPONSE_PDU 8
#define DLC_BITSTREAM_DATA 8
#define DLC_UDS_COMMAND_MAIN_MODULE 8
#define DLC_UDS_COMMAND_DASHBOARD 8
#define DLC_UDS_COMMAND_A_BOX 8
#define DLC_UDS_COMMAND_PDU 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_UDS_RESPONSE_MAIN_MODULE(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_RESPONSE_MAIN_MODULE, .DLC=DLC_UDS_RESPONSE_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_response_main_module.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_RESPONSE_DASHBOARD(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_RESPONSE_DASHBOARD, .DLC=DLC_UDS_RESPONSE_DASHBOARD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_response_dashboard.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_RESPONSE_A_BOX(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_RESPONSE_A_BOX, .DLC=DLC_UDS_RESPONSE_A_BOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_response_a_box.payload = payload_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_UDS_RESPONSE_PDU(payload_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_UDS_RESPONSE_PDU, .DLC=DLC_UDS_RESPONSE_PDU, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->uds_response_pdu.payload = payload_;\
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
        uint64_t d0: 8;
        uint64_t d1: 8;
        uint64_t d2: 8;
        uint64_t d3: 8;
        uint64_t d4: 8;
        uint64_t d5: 8;
        uint64_t d6: 8;
        uint64_t d7: 8;
    } bitstream_data;
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
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t d0;
        uint8_t d1;
        uint8_t d2;
        uint8_t d3;
        uint8_t d4;
        uint8_t d5;
        uint8_t d6;
        uint8_t d7;
    } bitstream_data;
    struct {
        uint64_t payload;
    } uds_command_main_module;
    struct {
        uint64_t payload;
    } uds_command_dashboard;
    struct {
        uint64_t payload;
    } uds_command_a_box;
    struct {
        uint64_t payload;
    } uds_command_pdu;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a);
extern void uds_command_main_module_CALLBACK(uint64_t payload);
extern void uds_command_dashboard_CALLBACK(uint64_t payload);
extern void uds_command_a_box_CALLBACK(uint64_t payload);
extern void uds_command_pdu_CALLBACK(uint64_t payload);
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