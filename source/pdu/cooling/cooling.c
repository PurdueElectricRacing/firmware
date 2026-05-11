#include "cooling.h"

#include "can_library/generated/PDU.h"
#include "common/bangbang/bangbang.h"
#include "common/phal_F4_F7/pwm/pwm.h"
#include "state.h"
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

// init assumes they're all on the same channel
#define FAN_PWM_TIM (TIM1)
static_assert(FAN_1_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_2_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_3_PWM_TIM == FAN_PWM_TIM);
static_assert(FAN_4_PWM_TIM == FAN_PWM_TIM);

// todo: decide thresholds
INIT_BANG_BANG(motor_pump, 50.0f, 35.0f, motor_pump_on, motor_pump_off, 1000);
INIT_BANG_BANG(inverter_pump, 50.0f, 35.0f, inverter_pump_on, inverter_pump_off, 1000);
INIT_BANG_BANG(hx_fan, 50.0f, 35.0f, hx_fan_on, hx_fan_off, 1000);
INIT_BANG_BANG(battery_fans, 50.0f, 35.0f, battery_fans_on, battery_fans_off, 1000);

static inline void auto_periodic(void) {
    uint32_t now = xTaskGetTickCount();
    // todo: pull current temps
    bangbang_update(&motor_pump, 0, now);
    bangbang_update(&inverter_pump, 0, now);
    bangbang_update(&battery_fans, 0, now);
}
static void cooling_fsm_periodic(void) {
    cooling_state = next_cooling_state;
    next_cooling_state = cooling_state; // default self loop

    switch (cooling_state) {
        case COOLING_STATE_INIT:
            // set the PWMS to full
            PHAL_PWMsetPercent(FAN_1_PWM_TIM, FAN_1_PWM_TIM_CH, 95);
            PHAL_PWMsetPercent(FAN_2_PWM_TIM, FAN_2_PWM_TIM_CH, 95);
            PHAL_PWMsetPercent(FAN_3_PWM_TIM, FAN_3_PWM_TIM_CH, 95);
            PHAL_PWMsetPercent(FAN_4_PWM_TIM, FAN_4_PWM_TIM_CH, 95);

            if (PHAL_initPWM(PWM_FREQUENCY_HZ, FAN_PWM_TIM, 4)) {
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
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}