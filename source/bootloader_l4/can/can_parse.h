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
#include "common/phal_L4/can/can.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "bootloader"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_MAINMODULE_BL_RESP 0x404e23c
#define ID_DASHBOARD_BL_RESP 0x404e27c
#define ID_MAINMODULE_BL_CMD 0x409c43e
#define ID_DASHBOARD_BL_CMD 0x409c47e
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAINMODULE_BL_RESP 5
#define DLC_DASHBOARD_BL_RESP 5
#define DLC_MAINMODULE_BL_CMD 5
#define DLC_DASHBOARD_BL_CMD 5
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAINMODULE_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAINMODULE_BL_RESP, .DLC=DLC_MAINMODULE_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mainmodule_bl_resp.cmd = cmd_;\
        data_a->mainmodule_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DASHBOARD_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DASHBOARD_BL_RESP, .DLC=DLC_DASHBOARD_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->dashboard_bl_resp.cmd = cmd_;\
        data_a->dashboard_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } mainmodule_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } mainmodule_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_cmd;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t cmd;
        uint32_t data;
    } mainmodule_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } dashboard_bl_cmd;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void mainmodule_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void dashboard_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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