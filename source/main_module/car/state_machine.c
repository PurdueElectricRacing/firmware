#include <stdbool.h>

#include "can_parse.h"
#include "car.h"
#include "common/amk/amk.h"

typedef struct {
    car_state_t current_state;
    car_state_t next_state;
    amk_motor_t amk_front_right;
    amk_motor_t amk_front_left;
    amk_motor_t amk_rear_left;
    amk_motor_t amk_rear_right;

    bool is_regen_enabled;
    bool brake_light;
    bool is_sdc_closed;
    bool buzzer;
} car_t;

car_t g_car;

void init_periodic();
void idle_periodic();
void precharge_periodic();
void energized_periodic();
void buzzing_periodic();
void ready2drive_periodic();
void error_periodic();

static inline bool is_SDC_open() {
    return true;
}

static inline bool is_init_complete() {
    return true;
}

static inline bool is_TSMS_high() {
    return true;
}

static inline bool is_precharge_complete() {
    return true;
}

static inline bool is_start_button_pressed() {
    return true;
}

static inline bool is_buzzing_time_elapsed() {
    return true;
}

/* BRAKE LIGHT CONFIG */
#define BRAKE_LIGHT_ON_THRESHOLD  (170)
#define BRAKE_LIGHT_OFF_THRESHOLD (70)

/**
* Brake Light Control
* The on threshold is larger than the off threshold to
* behave similar to a Shmitt-trigger, preventing blinking
* during a transition
*/
void set_brake_light() {
    if (can_data.filt_throttle_brake.brake > BRAKE_LIGHT_ON_THRESHOLD) {
        if (!g_car.brake_light) {
            g_car.brake_light = true;
        }
    } else if (can_data.filt_throttle_brake.brake < BRAKE_LIGHT_OFF_THRESHOLD) {
        if (g_car.brake_light) {
            g_car.brake_light = false;
        }
    }
}

void set_outputs() {
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, car.sdc_close);
    PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, car.brake_light | daq_brake);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, car.buzzer);
}

void car_init() {
    // enter INIT at n_reset
    g_car.current_state = CAR_STATE_INIT;
    g_car.next_state    = CAR_STATE_INIT;
}

#define TRANSITION_ON(condition, next_state_val) \
    if (condition) { \
        g_car.next_state = next_state_val; \
        break; \
    }

void fsm_periodic() {
    g_car.current_state = g_car.next_state;
    g_car.next_state    = g_car.current_state; // explicit self loop

    // check SDC before doing anything else
    if (is_SDC_open()) {
        g_car.next_state = CAR_STATE_ERROR;
        return;
    }

    set_brake_light();

    switch (g_car.current_state) {
        case CAR_STATE_INIT: {
            init_periodic();

            TRANSITION_ON(is_init_complete(), CAR_STATE_IDLE);
            break;
        }
        case CAR_STATE_IDLE: {
            idle_periodic();

            TRANSITION_ON(is_TSMS_high(), CAR_STATE_PRECHARGING);
            break;
        }
        case CAR_STATE_PRECHARGING: {
            precharge_periodic();

            TRANSITION_ON(is_precharge_complete(), CAR_STATE_ENERGIZED);
            break;
        }
        case CAR_STATE_ENERGIZED: {
            energized_periodic();

            TRANSITION_ON(is_start_button_pressed(), CAR_STATE_BUZZING);
            break;
        }
        case CAR_STATE_BUZZING: {
            buzzing_periodic();

            TRANSITION_ON(is_buzzing_time_elapsed(), CAR_STATE_READY2DRIVE);
            break;
        }
        case CAR_STATE_READY2DRIVE: {
            ready2drive_periodic();

            TRANSITION_ON(is_start_button_pressed(), CAR_STATE_IDLE);
            break;
        }
        case CAR_STATE_ERROR: {
            error_periodic();

            TRANSITION_ON(!is_SDC_open(), CAR_STATE_IDLE);
            break;
        }
        default: { // should never reach here
            g_car.next_state = CAR_STATE_ERROR;
            break;
        }
    }
}
