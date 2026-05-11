#include "cooling.h"

#include "can_library/generated/PDU.h"
#include "common/bangbang/bangbang.h"
#include "common/phal_F4_F7/pwm/pwm.h"
#include "state.h"
#include "switches.h"

typedef enum {
    COOLING_STATE_INIT   = 0,
    COOLING_STATE_AUTO   = 1,
    COOLING_STATE_MANUAL = 2,
} cooling_state_t;

static cooling_state_t cooling_state      = COOLING_STATE_INIT;
static cooling_state_t next_cooling_state = COOLING_STATE_INIT;

static constexpr uint32_t PWM_FREQUENCY_HZ = 25'000; // 25kHz
static constexpr uint8_t NUM_BATTERY_FANS = 4;
#define FAN_PWM_TIM (FAN_1_PWM_TIM) // Fan 1-4 use same timer defined in main.h

// todo: decide thresholds
INIT_BANG_BANG(motor_pump, 50, 35, nullptr, nullptr, 1000);
INIT_BANG_BANG(inverter_pump, 50, 35, nullptr, nullptr, 1000);
INIT_BANG_BANG(battery_fans, 50, 35, nullptr, nullptr, 1000);

static inline void auto_periodic(void) {
    uint32_t now = xTaskGetTickCount();
    // todo: pull current temps
    bangbang_update(&motor_pump, 0, now);
    bangbang_update(&inverter_pump, 0, now);
    bangbang_update(&battery_fans, 0, now);
}

static inline void manual_periodic(void) {
    // Dashboard manual commands are not wired yet, so fall back to AUTO behavior.
    auto_periodic();
}

static void cooling_fsm_periodic(void) {
    next_cooling_state = cooling_state;

    switch (cooling_state) {
        case COOLING_STATE_INIT:
            if (PHAL_initPWM(PWM_FREQUENCY_HZ, FAN_PWM_TIM, 4)) {
                next_cooling_state = COOLING_STATE_AUTO;
            }
            break;
        case COOLING_STATE_AUTO:
            auto_periodic();

            // todo: driver request into manual
            break;
        case COOLING_STATE_MANUAL:
            manual_periodic();

            // todo: driver request into auto
            break;
    }

    // flush the internal state
    
    switches_set_state(SW_PUMP_1, g_pdu_state.cooling_command.pump1_enabled);
    switches_set_state(SW_PUMP_2, g_pdu_state.cooling_command.pump2_enabled);
    switches_set_state(SW_HXFAN, g_pdu_state.cooling_command.hxfan_enabled);

    CAN_SEND_coolant_out(
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}