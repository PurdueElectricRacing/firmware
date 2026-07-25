#ifndef VEHICLE_FSM_H
#define VEHICLE_FSM_H

/**
 * @file vehicle_fsm.h
 * @brief Master vehicle state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "amk.h"
#include "can_library/generated/can_types.h"

typedef struct {
    car_state_t current_state;
    car_state_t next_state;

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
extern torque_request_t g_torque_request;

static constexpr uint32_t VEHICLE_FSM_PERIOD_MS = 15;

// static assert that the FSM flushes the CAN messages it owns at least as fast as their defined periods
static_assert(VEHICLE_FSM_PERIOD_MS == INVA_SET_PERIOD_MS);
static_assert(VEHICLE_FSM_PERIOD_MS == INVB_SET_PERIOD_MS);
static_assert(VEHICLE_FSM_PERIOD_MS == INVC_SET_PERIOD_MS);
static_assert(VEHICLE_FSM_PERIOD_MS == INVD_SET_PERIOD_MS);

void vehicle_fsm_periodic(void);

#endif // VEHICLE_FSM_H