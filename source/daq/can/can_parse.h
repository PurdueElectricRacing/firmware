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
#define ID_DAQ_CAN_STATS 0x8200000
#define ID_DAQ_QUEUE_STATS 0x8200200
#define ID_DAQ_BL_CMD 0x8001400
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_DAQ_CAN_STATS 8
#define DLC_DAQ_QUEUE_STATS 8
#define DLC_DAQ_BL_CMD 5
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
        uint8_t cmd;
        uint32_t data;
    } daq_bl_cmd;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void handleCallbacks(uint16_t id, bool latched);
extern void set_fault_daq(uint16_t id, bool value);
extern void return_fault_control(uint16_t id);
extern void send_fault(uint16_t id, bool latched);
/* END AUTO EXTERN CALLBACK */

/* BEGIN AUTO EXTERN RX IRQ */
/* END AUTO EXTERN RX IRQ */

bool initCANFilter(void);

#endif
