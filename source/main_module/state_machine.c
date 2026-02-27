/**
 * @file state_machine.c
 * @brief "Main Module" master state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "main.h"
#include "pindefs.h"

#include "common/can_library/faults_common.h"
#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/can_library/generated/can_types.h"
#include "common/phal/gpio.h"

static constexpr uint32_t MIN_BUZZING_TIME_MS = 2000;
static constexpr uint16_t BRAKE_LIGHT_ON_THRESHOLD = 200; // ~5% of 4095
static constexpr uint16_t BRAKE_LIGHT_OFF_THRESHOLD = 100; // ~2.5% of 4095

void error_periodic() {
    // todo
}

void energized_periodic() {
    // todo
}

void ready2drive_periodic() {
    if (can_data.filt_throttle_brake.stale) {
        g_torque_request.front_right = 0;
        g_torque_request.front_left  = 0;
        g_torque_request.rear_left   = 0;
        g_torque_request.rear_right  = 0;
        return;
    }

    // todo regen
    // todo smarter request scheme
    // todo torque vectoring
    // todo alternative throttle mapping (like S curve)

    float throttle = can_data.filt_throttle_brake.throttle / 4095.0f;
    int16_t torque_req_percent = (int16_t)(throttle * 100);
    
    g_torque_request.front_right = torque_req_percent;
    g_torque_request.front_left  = torque_req_percent;
    g_torque_request.rear_left   = torque_req_percent;
    g_torque_request.rear_right  = torque_req_percent;
}

static inline bool is_SDC_closed() {
    return g_car.is_SDC_closed;
}

static inline bool is_init_complete() {
    return true;
}

static inline bool is_TSMS_high() {
    return true;
}

static inline bool is_precharge_complete() {
    return PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
}

static inline bool is_AMKS_running() {
    return g_car.front_right.state == AMK_STATE_RUNNING
        && g_car.front_left.state  == AMK_STATE_RUNNING
        && g_car.rear_left.state   == AMK_STATE_RUNNING
        && g_car.rear_right.state  == AMK_STATE_RUNNING;
}

static inline bool is_start_button_pressed() {
    if (can_data.start_button.stale) {
        return false;
    }

    return can_data.start_button.is_pressed;
}

static inline bool is_buzzing_time_elapsed() {
    return (OS_TICKS - g_car.buzzer_start_time >= MIN_BUZZING_TIME_MS);
}

void buzzing_periodic() {
    g_car.buzzer_enable = true; // todo make a pattern
}

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

void fsm_periodic() {
    g_car.current_state = g_car.next_state;
    g_car.next_state    = g_car.current_state; // explicit self loop

    // check SDC before doing anything else
    if (!is_SDC_closed()) {
        g_car.next_state = CARSTATE_FATAL;
        return;
    }

    set_brake_light();

    switch (g_car.current_state) {
        case CARSTATE_INIT: {
            // do nothing for now

            if (is_init_complete()) {
                g_car.next_state = CARSTATE_IDLE;
            }
            break;
        }
        case CARSTATE_IDLE: {
            // do nothing for now

            if (is_TSMS_high()) {
                g_car.next_state = CARSTATE_PRECHARGING;
            }
            break;
        }
        case CARSTATE_PRECHARGING: {
            // do nothing for now

            if (is_precharge_complete()) {
                g_car.next_state = CARSTATE_ENERGIZED;
            }
            break;
        }
        case CARSTATE_ENERGIZED: {
            energized_periodic();

            if (is_start_button_pressed() && is_AMKS_running()) {
                g_car.buzzer_start_time = OS_TICKS;
                g_car.next_state = CARSTATE_BUZZING;
            }
            break;
        }
        case CARSTATE_BUZZING: {
            buzzing_periodic();

            if (is_buzzing_time_elapsed()) {
                g_car.buzzer_enable = false; // explicitly turn off the buzzer before transitioning
                g_car.next_state = CARSTATE_READY2DRIVE;
            }
            break;
        }
        case CARSTATE_READY2DRIVE: {
            ready2drive_periodic();

            if (is_start_button_pressed()) {
                g_car.next_state = CARSTATE_IDLE;
            }
            break;
        }
        case CARSTATE_FATAL: {
            error_periodic();

            if (is_SDC_closed()) {
                g_car.next_state = CARSTATE_IDLE;
            }
            break;
        }
        default: { // should never reach here
            g_car.next_state = CARSTATE_FATAL;
            break;
        }
    }

    AMK_set_torque(&g_car.front_right, g_torque_request.front_right);
    AMK_set_torque(&g_car.front_left,  g_torque_request.front_left);
    AMK_set_torque(&g_car.rear_left,   g_torque_request.rear_left);
    AMK_set_torque(&g_car.rear_right,  g_torque_request.rear_right);

    // flush the internal state
    PHAL_writeGPIO(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, g_car.brake_light);
    PHAL_writeGPIO(TSAL_GREEN_CTRL_PORT, TSAL_GREEN_CTRL_PIN, g_car.tsal_green_enable);
    PHAL_writeGPIO(TSAL_RED_CTRL_PORT, TSAL_RED_CTRL_PIN, g_car.tsal_red_enable);
    PHAL_writeGPIO(BUZZER_PORT, BUZZER_PIN, g_car.buzzer_enable);
}