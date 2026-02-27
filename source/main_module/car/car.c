#include "car.h"

#include "common/can_library/faults_common.h"
#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/can_library/generated/can_types.h"
#include "common/phal/gpio.h"
#include "main.h"

static constexpr uint32_t MIN_BUZZING_TIME_MS = 2000;
static constexpr uint16_t BRAKE_LIGHT_ON_THRESHOLD = 200; // ~5% of 4095
static constexpr uint16_t BRAKE_LIGHT_OFF_THRESHOLD = 100; // ~2.5% of 4095

car_t g_car;

void init_periodic();
void idle_periodic();
void precharge_periodic();
void energized_periodic();
void buzzing_periodic();
void ready2drive_periodic();
void error_periodic();

void flush_inva() {
    CAN_SEND_INVA_SET(
        g_car.front_left.set->AMK_Control_bReserve,
        g_car.front_left.set->AMK_Control_bInverterOn,
        g_car.front_left.set->AMK_Control_bDcOn,
        g_car.front_left.set->AMK_Control_bEnable,
        g_car.front_left.set->AMK_Control_bErrorReset,
        g_car.front_left.set->AMK_Control_bReserve2,
        g_car.front_left.set->AMK_TorqueSetpoint,
        g_car.front_left.set->AMK_PositiveTorqueLimit,
        g_car.front_left.set->AMK_NegativeTorqueLimit
    );
}

void flush_inb() {
    CAN_SEND_INVB_SET(
        g_car.front_right.set->AMK_Control_bReserve,
        g_car.front_right.set->AMK_Control_bInverterOn,
        g_car.front_right.set->AMK_Control_bDcOn,
        g_car.front_right.set->AMK_Control_bEnable,
        g_car.front_right.set->AMK_Control_bErrorReset,
        g_car.front_right.set->AMK_Control_bReserve2,
        g_car.front_right.set->AMK_TorqueSetpoint,
        g_car.front_right.set->AMK_PositiveTorqueLimit,
        g_car.front_right.set->AMK_NegativeTorqueLimit
    );
}

void init_periodic() {
    AMK_init(
        &g_car.front_left,
        flush_inva,
        g_car.front_left.set,
        g_car.front_left.crit,
        g_car.front_left.info,
        g_car.front_left.temps,
        g_car.front_left.err1,
        g_car.front_left.err2,
        &g_car.is_precharge_complete
    );

    AMK_init(
        &g_car.front_right,
        flush_inb,
        g_car.front_right.set,
        g_car.front_right.crit,
        g_car.front_right.info,
        g_car.front_right.temps,
        g_car.front_right.err1,
        g_car.front_right.err2,
        &g_car.is_precharge_complete
    );
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

void car_init() {
    // enter INIT at n_reset
    g_car.current_state = CARSTATE_INIT;
    g_car.next_state    = CARSTATE_INIT;
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
            init_periodic();

            if (is_init_complete()) {
                g_car.next_state = CARSTATE_IDLE;
            }
            break;
        }
        case CARSTATE_IDLE: {
            idle_periodic();

            if (is_TSMS_high()) {
                g_car.next_state = CARSTATE_PRECHARGING;
            }
            break;
        }
        case CARSTATE_PRECHARGING: {
            precharge_periodic();

            if (is_precharge_complete()) {
                g_car.next_state = CARSTATE_ENERGIZED;
            }
            break;
        }
        case CARSTATE_ENERGIZED: {
            energized_periodic();

            if (is_start_button_pressed()) {
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

    AMK_periodic(&g_car.front_right);
    AMK_periodic(&g_car.front_left);
    AMK_periodic(&g_car.rear_left);
    AMK_periodic(&g_car.rear_right);

    // flush the internal state
    PHAL_writeGPIO(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, g_car.brake_light);
    PHAL_writeGPIO(TSAL_GREEN_CTRL_PORT, TSAL_GREEN_CTRL_PIN, g_car.tsal_green_enable);
    PHAL_writeGPIO(TSAL_RED_CTRL_PORT, TSAL_RED_CTRL_PIN, g_car.tsal_red_enable);
    PHAL_writeGPIO(BUZZER_PORT, BUZZER_PIN, g_car.buzzer_enable);
}