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
#define ID_MAIN_MODULE_BL_RESP 0x8e00000
#define ID_DASHBOARD_BL_RESP 0x8e00200
#define ID_TORQUEVECTOR_BL_RESP 0x8e00400
#define ID_A_BOX_BL_RESP 0x8e00600
#define ID_PDU_BL_RESP 0x8e00800
#define ID_L4_TESTING_BL_RESP 0x8e00a00
#define ID_F4_TESTING_BL_RESP 0x8e00c00
#define ID_F7_TESTING_BL_RESP 0x8e00e00
#define ID_DAQ_BL_RESP 0x8e01000
#define ID_BITSTREAM_DATA 0x8000000
#define ID_MAIN_MODULE_BL_CMD 0x8000400
#define ID_DASHBOARD_BL_CMD 0x8000600
#define ID_TORQUEVECTOR_BL_CMD 0x8000800
#define ID_A_BOX_BL_CMD 0x8000a00
#define ID_PDU_BL_CMD 0x8000c00
#define ID_L4_TESTING_BL_CMD 0x8000e00
#define ID_F4_TESTING_BL_CMD 0x8001000
#define ID_F7_TESTING_BL_CMD 0x8001200
#define ID_DAQ_BL_CMD 0x8001400
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_MODULE_BL_RESP 5
#define DLC_DASHBOARD_BL_RESP 5
#define DLC_TORQUEVECTOR_BL_RESP 5
#define DLC_A_BOX_BL_RESP 5
#define DLC_PDU_BL_RESP 5
#define DLC_L4_TESTING_BL_RESP 5
#define DLC_F4_TESTING_BL_RESP 5
#define DLC_F7_TESTING_BL_RESP 5
#define DLC_DAQ_BL_RESP 5
#define DLC_BITSTREAM_DATA 8
#define DLC_MAIN_MODULE_BL_CMD 5
#define DLC_DASHBOARD_BL_CMD 5
#define DLC_TORQUEVECTOR_BL_CMD 5
#define DLC_A_BOX_BL_CMD 5
#define DLC_PDU_BL_CMD 5
#define DLC_L4_TESTING_BL_CMD 5
#define DLC_F4_TESTING_BL_CMD 5
#define DLC_F7_TESTING_BL_CMD 5
#define DLC_DAQ_BL_CMD 5
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_MODULE_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_MODULE_BL_RESP, .DLC=DLC_MAIN_MODULE_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_module_bl_resp.cmd = cmd_;\
        data_a->main_module_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DASHBOARD_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DASHBOARD_BL_RESP, .DLC=DLC_DASHBOARD_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->dashboard_bl_resp.cmd = cmd_;\
        data_a->dashboard_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_TORQUEVECTOR_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TORQUEVECTOR_BL_RESP, .DLC=DLC_TORQUEVECTOR_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->torquevector_bl_resp.cmd = cmd_;\
        data_a->torquevector_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_A_BOX_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_A_BOX_BL_RESP, .DLC=DLC_A_BOX_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->a_box_bl_resp.cmd = cmd_;\
        data_a->a_box_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PDU_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PDU_BL_RESP, .DLC=DLC_PDU_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pdu_bl_resp.cmd = cmd_;\
        data_a->pdu_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_L4_TESTING_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_L4_TESTING_BL_RESP, .DLC=DLC_L4_TESTING_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->l4_testing_bl_resp.cmd = cmd_;\
        data_a->l4_testing_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_F4_TESTING_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_F4_TESTING_BL_RESP, .DLC=DLC_F4_TESTING_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->f4_testing_bl_resp.cmd = cmd_;\
        data_a->f4_testing_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_F7_TESTING_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_F7_TESTING_BL_RESP, .DLC=DLC_F7_TESTING_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->f7_testing_bl_resp.cmd = cmd_;\
        data_a->f7_testing_bl_resp.data = data_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_BL_RESP(cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_BL_RESP, .DLC=DLC_DAQ_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_bl_resp.cmd = cmd_;\
        data_a->daq_bl_resp.data = data_;\
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
        uint64_t cmd: 8;
        uint64_t data: 32;
    } main_module_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } torquevector_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } a_box_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } pdu_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } l4_testing_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } f4_testing_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } f7_testing_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } daq_bl_resp;
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
        uint64_t cmd: 8;
        uint64_t data: 32;
    } main_module_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } torquevector_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } a_box_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } pdu_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } l4_testing_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } f4_testing_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } f7_testing_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } daq_bl_cmd;
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
        uint8_t cmd;
        uint32_t data;
    } main_module_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } dashboard_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } torquevector_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } a_box_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } pdu_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } l4_testing_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } f4_testing_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } f7_testing_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } daq_bl_cmd;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a);
extern void main_module_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void dashboard_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void torquevector_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void a_box_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void pdu_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void l4_testing_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void f4_testing_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void f7_testing_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void daq_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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