#ifndef VEHICLE_FSM_H
#define VEHICLE_FSM_H

/**
 * @file vehicle_fsm.h
 * @brief Master vehicle state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "can_library/generated/can_types.h"

typedef struct {
    car_state_t current_state;
    car_state_t next_state;

    // internal state variables
    uint32_t buzzer_start_time;
    bool last_start_button;
    bool brake_light;
    bool tsal_green_enable;
    bool tsal_red_enable;
    bool buzzer_enable;
} car_t;

extern car_t g_car;

static constexpr uint32_t VEHICLE_FSM_PERIOD_MS = 15;

void vehicle_fsm_init(void);
void vehicle_fsm_periodic(void);

#endif // VEHICLE_FSM_H
