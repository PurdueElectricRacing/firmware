#ifndef VEHICLE_FSM_H
#define VEHICLE_FSM_H

/**
 * @file vehicle_fsm.h
 * @brief Master vehicle state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "can_library/generated/can_types.h"
#include "amk.h"

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

typedef struct {
    int16_t front_right;
    int16_t front_left;
    int16_t rear_left;
    int16_t rear_right;
} torque_request_t;

extern car_t g_car;
extern torque_request_t g_torque_request;

static constexpr uint32_t VEHICLE_FSM_PERIOD_MS = 15;

void vehicle_fsm_periodic(void);

#endif // VEHICLE_FSM_H