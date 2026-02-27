#ifndef MAIN_H
#define MAIN_H

#include "common/amk/amk.h"

typedef struct {
    CarState_t current_state;
    CarState_t next_state;
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

    // input states
    bool is_SDC_closed;
    bool is_precharge_complete;
} car_t;

typedef struct {
    int16_t front_right;
    int16_t front_left;
    int16_t rear_left;
    int16_t rear_right;
} torque_request_t;

typedef uint16_t SDC_state_t;

extern car_t g_car;
extern torque_request_t g_torque_request;
extern SDC_state_t g_SDC_state;

void fsm_periodic();

#endif // MAIN_H