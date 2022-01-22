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
#define NODE_NAME "Precharge"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_BALANCE_REQUEST 0x4007d2a
#define ID_TEST_PRECHARGE_MSG 0x8008004
#define ID_SOC1 0x4007d2b
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_BALANCE_REQUEST 2
#define DLC_TEST_PRECHARGE_MSG 1
#define DLC_SOC1 2
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_BALANCE_REQUEST(queue, volts_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_BALANCE_REQUEST, .DLC=DLC_BALANCE_REQUEST, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->balance_request.volts = volts_;\
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
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t volts: 16;
    } balance_request;
    struct {
        uint64_t test_precharge_sig: 8;
    } test_precharge_msg;
    struct {
        uint64_t soc: 16;
    } soc1;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t soc;
    } soc1;
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