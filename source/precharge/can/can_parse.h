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
#define NODE_NAME "Precharge"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_HEAT_REQ 0x8007d2a
#define ID_PACK_CURR 0x4007d6a
#define ID_TEST_PRECHARGE_MSG 0x8008004
#define ID_SOC_CELLS 0x8007d6b
#define ID_VOLTS_CELLS 0x4007dab
#define ID_PACK_INFO 0x8007deb
#define ID_TEMPS_CELLS 0x4007e2b
#define ID_CELL_INFO 0x8007e6b
#define ID_POWER_LIM 0x4007eab
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
#define DLC_TEST_PRECHARGE_MSG 1
#define DLC_SOC_CELLS 7
#define DLC_VOLTS_CELLS 7
#define DLC_PACK_INFO 6
#define DLC_TEMPS_CELLS 7
#define DLC_CELL_INFO 6
#define DLC_POWER_LIM 4
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_HEAT_REQ(queue, toggle_, time_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_HEAT_REQ, .DLC=DLC_HEAT_REQ, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->heat_req.toggle = toggle_;\
        data_a->heat_req.time = time_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_CURR(queue, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_PACK_CURR, .DLC=DLC_PACK_CURR, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_curr.current = current_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEST_PRECHARGE_MSG(queue, test_precharge_sig_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_PRECHARGE_MSG, .DLC=DLC_TEST_PRECHARGE_MSG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_precharge_msg.test_precharge_sig = test_precharge_sig_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t toggle: 1;
        uint64_t time: 16;
    } heat_req;
    struct {
        uint64_t current: 16;
    } pack_curr;
    struct {
        uint64_t test_precharge_sig: 8;
    } test_precharge_msg;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
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