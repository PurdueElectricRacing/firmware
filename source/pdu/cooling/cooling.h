/**
 * @file cooling.h
 * @author Nicolas Vera (nverapae@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-2-29
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _COOLING_H_
#define _COOLING_H_

#include "PDU.h"

#define GPS_SPEED_MOVING          2 // TODO determine
#define MOTOR_COOLING_ENABLE_TEMP 60
#define MOTOR_COOLING_MAX_TEMP    100
#define BATT_COOLING_ENABLE_TEMP  30

typedef struct {
    uint16_t fan1_speed; // value from 0-100
    uint16_t fan2_speed; // value from 0-100
    bool pump1_status;
    bool pump2_status;
    bool fan1_status;
    bool fan2_status;
    bool aux_status;
} cooling_request_t;

/**
 * @brief Initializes cooling_request struct values to 0
 *
 * @param
 * @return
 */
void coolingInit();

/**
 * @brief Periodic function that sets switches and fan speeds based off of values in the cooling_request struct
 *
 * @param
 * @return
 */
void update_cooling_periodic();

/**
 * @brief Callback function for cooling_driver_request message sent from dashboard, updates values in cooling_request struct based off signal values
 *
 * @param *msg_data_a CAN msg data
 * @return
 */
void cooling_driver_request_CALLBACK(can_data_t* p_can_data);
#endif
