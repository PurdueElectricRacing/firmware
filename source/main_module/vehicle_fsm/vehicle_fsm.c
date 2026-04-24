/**
 * @file vehicle_fsm.c
 * @brief Master vehicle state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "vehicle_fsm.h"
#include "main.h"

#include "can_library/faults_common.h"
#include "can_library/generated/MAIN_MODULE.h"
#include "common/phal/gpio.h"

// Global data structures
car_t g_car;
torque_request_t g_torque_request;

static void ready2drive_periodic() {
    if (can_data.pedals.is_stale()) {
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

    // assumes pedals.throttle is in the range [0, 4095]
    float throttle = can_data.pedals.throttle / 4095.0f;
    int16_t torque_req_percent = (int16_t)(throttle * 100);

    int16_t rear_torque = torque_req_percent * 2.1f; // allow 110% over-torque

    // Bias to feel like a 40% - 60% torque split
    int16_t front_torque = (40.0f / 60.0f) * rear_torque;
    
    g_torque_request.front_right = front_torque;
    g_torque_request.front_left  = front_torque;
    g_torque_request.rear_left   = rear_torque;
    g_torque_request.rear_right  = rear_torque;
}

static inline bool is_all_AMKS_running() {
    return g_car.front_right.state == AMK_STATE_RUNNING
        && g_car.front_left.state  == AMK_STATE_RUNNING
        && g_car.rear_left.state   == AMK_STATE_RUNNING
        && g_car.rear_right.state  == AMK_STATE_RUNNING;
}

static inline bool is_start_button_pressed() {
    if (can_data.start_button.is_stale()) {
        return false;
    }

    return can_data.start_button.is_pressed;
}

static inline bool is_buzzing_time_elapsed() {
    // FSAE 2026 EV.9.7.2: The Ready to Drive Sound must be sounded continuously for minimum 1 second and maximum 3 seconds
    static constexpr uint32_t MIN_BUZZING_TIME_MS = 2500;

    return (OS_TICKS - g_car.buzzer_start_time >= MIN_BUZZING_TIME_MS);
}

static void update_brake_light() {
    static constexpr uint16_t BRAKE_LIGHT_ON_THRESHOLD  = 200; // ~5% of 4095
    static constexpr uint16_t BRAKE_LIGHT_OFF_THRESHOLD = 100; // ~2.5% of 4095

    if (can_data.pedals.brake > BRAKE_LIGHT_ON_THRESHOLD) {
        if (!g_car.brake_light) {
            g_car.brake_light = true;
        }
    } else if (can_data.pedals.brake < BRAKE_LIGHT_OFF_THRESHOLD) {
        if (g_car.brake_light) {
            g_car.brake_light = false;
        }
    }
}

static void update_tsal() {
    // FSAE 2026 EV.5.11.5
    if (is_latched(FAULT_ID_SDC2_BMS) || is_latched(FAULT_ID_IMD)) { 
        g_car.tsal_green_enable = false;
        g_car.tsal_red_enable = true;
    } else {
        g_car.tsal_green_enable = true;
        g_car.tsal_red_enable = false;
    }
}

static void update_amks() {
    // iterate the AMK fsms
    AMK_periodic(&g_car.front_right);
    AMK_periodic(&g_car.front_left);
    AMK_periodic(&g_car.rear_left);
    AMK_periodic(&g_car.rear_right);
}

void vehicle_fsm_periodic(void) {
    // set default states
    g_car.current_state = g_car.next_state;
    g_car.next_state    = g_car.current_state; // explicit self loop
    g_car.brake_light   = false;
    g_car.buzzer_enable = false;

    // zero torque request by default
    g_torque_request.front_right = 0;
    g_torque_request.front_left  = 0;
    g_torque_request.rear_left   = 0;
    g_torque_request.rear_right  = 0;
    
    update_amks();
    update_brake_light();
    update_tsal();

    // update precharge status
    bool precharge_pin = !PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
    update_fault(FAULT_ID_PRECHARGE_INCOMPLETE, precharge_pin);
    // amks need a bool to point to for precharge status
    g_car.is_precharge_complete = is_clear(FAULT_ID_PRECHARGE_INCOMPLETE);

    // any SDCs latched 1-15 faults will cause a fatal state
    if (is_latched(FAULT_ID_SDC15_REAR_INTERLOCK)) {
        g_car.current_state = CAR_STATE_FATAL;
        g_car.next_state = CAR_STATE_FATAL;
    } else if (is_latched(FAULT_ID_SDC16_TSMS)) { // return to idle if TSMS is opened
        g_car.current_state = CAR_STATE_IDLE;
        g_car.next_state = CAR_STATE_IDLE;
    }

    switch (g_car.current_state) {
        case CAR_STATE_FATAL: {
            // nothing for now

            if (is_clear(FAULT_ID_SDC15_REAR_INTERLOCK)) {
                g_car.next_state = CAR_STATE_IDLE;
            }
            break;
        }
        case CAR_STATE_IDLE: {
            // do nothing for now

            if (is_clear(FAULT_ID_SDC16_TSMS)) { // TSMS is closed
                g_car.next_state = CAR_STATE_PRECHARGING;
            }
            break;
        }
        case CAR_STATE_PRECHARGING: {
            // do nothing for now

            if (is_clear(FAULT_ID_PRECHARGE_INCOMPLETE)) {
                g_car.next_state = CAR_STATE_ENERGIZED;
            }
            break;
        }
        case CAR_STATE_ENERGIZED: {
            // do nothing for now

            if (is_start_button_pressed() && is_all_AMKS_running()) {
                g_car.buzzer_start_time = OS_TICKS;
                g_car.next_state = CAR_STATE_BUZZING;
            }
            break;
        }
        case CAR_STATE_BUZZING: {
            g_car.buzzer_enable = true;

            if (is_buzzing_time_elapsed()) {
                g_car.next_state = CAR_STATE_READY2DRIVE;
            }
            break;
        }
        case CAR_STATE_READY2DRIVE: {
            ready2drive_periodic();

            if (is_start_button_pressed()) {
                g_car.next_state = CAR_STATE_ENERGIZED;
            }
            break;
        }
    }

    // flush the internal state
    AMK_set_torque(&g_car.front_right, g_torque_request.front_right);
    AMK_set_torque(&g_car.front_left,  g_torque_request.front_left);
    AMK_set_torque(&g_car.rear_left,   g_torque_request.rear_left);
    AMK_set_torque(&g_car.rear_right,  g_torque_request.rear_right);

    CAN_SEND_main_hb(g_car.current_state);

    PHAL_writeGPIO(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, g_car.brake_light);
    PHAL_writeGPIO(TSAL_GREEN_CTRL_PORT, TSAL_GREEN_CTRL_PIN, g_car.tsal_green_enable);
    PHAL_writeGPIO(TSAL_RED_CTRL_PORT, TSAL_RED_CTRL_PIN, g_car.tsal_red_enable);
    PHAL_writeGPIO(BUZZER_PORT, BUZZER_PIN, g_car.buzzer_enable);
}