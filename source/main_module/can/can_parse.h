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
#define NODE_NAME "Main_Module"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_DRIVER_REQUEST 0x4000040
#define ID_MAIN_STATUS 0x4001900
#define ID_DAQ_RESPONSE_MAIN_MODULE 0x17ffffc0
#define ID_RAW_THROTTLE_BRAKE 0x14000285
#define ID_START_BUTTON 0x4000005
#define ID_FRONT_MOTOR_CURRENTS_TEMPS 0xc000282
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c2
#define ID_DAQ_COMMAND_MAIN_MODULE 0x14000032
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_DRIVER_REQUEST 4
#define DLC_MAIN_STATUS 3
#define DLC_DAQ_RESPONSE_MAIN_MODULE 8
#define DLC_RAW_THROTTLE_BRAKE 3
#define DLC_START_BUTTON 1
#define DLC_FRONT_MOTOR_CURRENTS_TEMPS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 6
#define DLC_DAQ_COMMAND_MAIN_MODULE 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_DRIVER_REQUEST(queue, accel_, regen_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DRIVER_REQUEST, .DLC=DLC_DRIVER_REQUEST, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->driver_request.accel = accel_;\
        data_a->driver_request.regen = regen_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MAIN_STATUS(queue, car_state_, apps_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_STATUS, .DLC=DLC_MAIN_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_status.car_state = car_state_;\
        data_a->main_status.apps_state = apps_state_;\
        data_a->main_status.precharge_state = precharge_state_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_MAIN_MODULE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_MAIN_MODULE, .DLC=DLC_DAQ_RESPONSE_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_MAIN_MODULE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_RAW_THROTTLE_BRAKE 5
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_INIT,
    CAR_STATE_PRECHARGING,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
} car_state_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t accel: 16;
        uint64_t regen: 16;
    } driver_request;
    struct {
        uint64_t car_state: 8;
        uint64_t apps_state: 8;
        uint64_t precharge_state: 1;
    } main_status;
    struct {
        uint64_t daq_response: 64;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t throttle: 12;
        uint64_t brake: 12;
    } raw_throttle_brake;
    struct {
        uint64_t start: 1;
    } start_button;
    struct {
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } front_motor_currents_temps;
    struct {
        uint64_t left: 16;
        uint64_t right: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } rear_motor_currents_temps;
    struct {
        uint64_t daq_command: 64;
    } daq_command_MAIN_MODULE;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t throttle;
        uint16_t brake;
        uint8_t stale;
        uint32_t last_rx;
    } raw_throttle_brake;
    struct {
        uint8_t start;
    } start_button;
    struct {
        uint16_t left_current;
        uint16_t right_current;
        uint8_t left_temp;
        uint8_t right_temp;
    } front_motor_currents_temps;
    struct {
        uint16_t left;
        uint16_t right;
        uint8_t left_temp;
        uint8_t right_temp;
    } rear_motor_currents_temps;
    struct {
        uint64_t daq_command;
    } daq_command_MAIN_MODULE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
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