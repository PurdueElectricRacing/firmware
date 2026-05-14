/**
 * @file cooling.c
 * @brief thermal control state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "cooling_fsm.h"

#include "can_library/generated/PDU.h"
#include "common/bangbang/bangbang.h"
#include "common/phal_F4_F7/pwm/pwm.h"
#include "common/utils/max.h"
#include "cooling_callbacks.h"
#include "main.h"

typedef enum {
    COOLING_STATE_INIT   = 0,
    COOLING_STATE_AUTO   = 1,
    COOLING_STATE_MANUAL = 2,
} cooling_state_t;

static cooling_state_t cooling_state      = COOLING_STATE_INIT;
static cooling_state_t next_cooling_state = COOLING_STATE_INIT;

static constexpr uint32_t PWM_FREQUENCY_HZ = 25'000; // 25kHz
static constexpr uint8_t  NUM_BATTERY_FANS = 4;

// PWM init assumes they're all on the same channel
#define FAN_PWM_TIM (TIM1)
static_assert(FAN_1_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_2_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_3_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_4_PWM_TIM == FAN_PWM_TIM);

// todo: decide thresholds
static constexpr float MOTOR_PUMP_UPPER_LIMIT = 50.0f;
static constexpr float MOTOR_PUMP_LOWER_LIMIT = 35.0f;
static constexpr uint32_t MOTOR_PUMP_HYSTERESIS = 1000;

static constexpr float INVERTER_PUMP_UPPER_LIMIT = 50.0f;
static constexpr float INVERTER_PUMP_LOWER_LIMIT = 35.0f;
static constexpr uint32_t INVERTER_PUMP_HYSTERESIS = 1000;

static constexpr float HX_FAN_UPPER_LIMIT = 50.0f;
static constexpr float HX_FAN_LOWER_LIMIT = 35.0f;
static constexpr uint32_t HX_FAN_HYSTERESIS = 1000;

static constexpr float BATTERY_FAN_UPPER_LIMIT = 50.0f;
static constexpr float BATTERY_FAN_LOWER_LIMIT = 35.0f;
static constexpr uint32_t BATTERY_FAN_HYSTERESIS = 1000;

INIT_BANG_BANG(
    motor_pump,
    MOTOR_PUMP_UPPER_LIMIT,
    MOTOR_PUMP_LOWER_LIMIT,
    motor_pump_on,
    motor_pump_off,
    MOTOR_PUMP_HYSTERESIS
);
INIT_BANG_BANG(
    inverter_pump,
    INVERTER_PUMP_UPPER_LIMIT,
    INVERTER_PUMP_LOWER_LIMIT,
    inverter_pump_on,
    inverter_pump_off,
    INVERTER_PUMP_HYSTERESIS
);
INIT_BANG_BANG(
    hx_fan,
    HX_FAN_UPPER_LIMIT,
    HX_FAN_LOWER_LIMIT,
    hx_fan_on,
    hx_fan_off,
    HX_FAN_HYSTERESIS
);
INIT_BANG_BANG(
    battery_fans,
    BATTERY_FAN_UPPER_LIMIT,
    BATTERY_FAN_LOWER_LIMIT,
    battery_fans_on,
    battery_fans_off,
    BATTERY_FAN_HYSTERESIS
);

static inline void auto_periodic(void) {
    uint32_t now = xTaskGetTickCount();
    
    if (can_data.motor_temps.is_stale()) {
        // force on
        bangbang_update(&motor_pump, MOTOR_PUMP_UPPER_LIMIT + 1, now);
    } else {
        int16_t max_motor_temp = MAXOF(
            can_data.motor_temps.front_right,
            can_data.motor_temps.front_left,
            can_data.motor_temps.rear_left,
            can_data.motor_temps.rear_right
        );

        int16_t scaled_motor_temp = max_motor_temp * UNPACK_COEFF_MOTOR_TEMPS_FRONT_RIGHT;
        bangbang_update(&motor_pump, scaled_motor_temp, now);
    }

    if (can_data.igbt_temps.is_stale()) {
        // force on
        bangbang_update(&inverter_pump, INVERTER_PUMP_UPPER_LIMIT + 1, now);
    } else {
        int16_t max_igbt_temp = MAXOF(
            can_data.igbt_temps.front_right,
            can_data.igbt_temps.front_left,
            can_data.igbt_temps.rear_left,
            can_data.igbt_temps.rear_right
        );

        int16_t scaled_igbt_temp = max_igbt_temp * UNPACK_COEFF_IGBT_TEMPS_FRONT_RIGHT;
        bangbang_update(&inverter_pump, scaled_igbt_temp, now);
    }

    if (can_data.pack_stats.is_stale()) {
        // force on
        bangbang_update(&battery_fans, BATTERY_FAN_UPPER_LIMIT + 1, now);
    } else {
        int16_t max_temp = can_data.pack_stats.max_temp;
        bangbang_update(&battery_fans, max_temp, now);
    }

    // todo: read water temps
    // force on
    bangbang_update(&hx_fan, HX_FAN_UPPER_LIMIT + 1, now);
}

void cooling_fsm_periodic(void) {
    cooling_state = next_cooling_state;
    next_cooling_state = cooling_state; // default self loop

    switch (cooling_state) {
        case COOLING_STATE_INIT:
            if (PHAL_initPWM(PWM_FREQUENCY_HZ, FAN_PWM_TIM, NUM_BATTERY_FANS)) {
                // set the PWMS to full
                PHAL_PWMsetPercent(FAN_1_PWM_TIM, FAN_1_PWM_TIM_CH, 95);
                PHAL_PWMsetPercent(FAN_2_PWM_TIM, FAN_2_PWM_TIM_CH, 95);
                PHAL_PWMsetPercent(FAN_3_PWM_TIM, FAN_3_PWM_TIM_CH, 95);
                PHAL_PWMsetPercent(FAN_4_PWM_TIM, FAN_4_PWM_TIM_CH, 95);
                next_cooling_state = COOLING_STATE_AUTO;
            }
            break;
        case COOLING_STATE_AUTO:
            auto_periodic();

            // todo: driver request into manual
            break;
        case COOLING_STATE_MANUAL:
            // do nothing

            // todo: driver request into auto
            break;
    }

    CAN_SEND_coolant_out(
        motor_pump.is_on,
        inverter_pump.is_on,
        hx_fan.is_on,
        battery_fans.is_on
    );
}