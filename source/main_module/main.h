#ifndef MAIN_H
#define MAIN_H

/**
 * @file main.h
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/amk/amk.h"

typedef struct {
    car_state_t current_state;
    car_state_t next_state;
    AMK_t front_right; // INVA
    AMK_t front_left;  // INVB
    AMK_t rear_left;   // INVC
    AMK_t rear_right;  // INVD

    // internal state variables
    uint32_t buzzer_start_time;
    bool last_start_button_state;
    bool brake_light;
    bool tsal_green_enable;
    bool tsal_red_enable;
    bool buzzer_enable;

    bool is_precharge_complete; // AMKs are pointed to this variable
} car_t;
extern car_t g_car;

typedef struct {
    int16_t front_right;
    int16_t front_left;
    int16_t rear_left;
    int16_t rear_right;
} torque_request_t;
extern torque_request_t g_torque_request;

void vehicle_fsm_periodic();
void car_init();
void SDC_thread_periodic();

#endif // MAIN_H