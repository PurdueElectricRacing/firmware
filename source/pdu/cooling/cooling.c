#include "cooling.h"

#include "can_library/generated/PDU.h"
#include "cooling_bangbang.h"
#include "fan_control.h"
#include "state.h"
#include "switches.h"

typedef enum {
    COOLING_STATE_AUTO   = 0,
    COOLING_STATE_MANUAL = 1,
} cooling_state_t;

static cooling_state_t cooling_state      = COOLING_STATE_AUTO;
static cooling_state_t next_cooling_state = COOLING_STATE_AUTO;

static inline void auto_periodic(void) {
    g_pdu_state.cooling_command.fan1_enabled = true;
    if (g_pdu_state.cooling_command.batt_fan_autospeed_enabled) {
        g_pdu_state.cooling_command.fan1_percent = 100;
    }

    g_pdu_state.cooling_command.fan2_enabled = true;
    if (g_pdu_state.cooling_command.motor_fan_autospeed_enabled) {
        g_pdu_state.cooling_command.fan2_percent = 100;
    }

    g_pdu_state.cooling_command.fan3_enabled = g_pdu_state.cooling_command.fan1_enabled;
    g_pdu_state.cooling_command.fan3_percent = g_pdu_state.cooling_command.fan1_percent;
    g_pdu_state.cooling_command.fan4_enabled = g_pdu_state.cooling_command.fan2_enabled;
    g_pdu_state.cooling_command.fan4_percent = g_pdu_state.cooling_command.fan2_percent;

    cooling_bangbang_update(&g_pdu_state.cooling_command);
}

static inline void manual_periodic(void) {
    // Dashboard manual commands are not wired yet, so fall back to AUTO behavior.
    auto_periodic();
}

static inline void set_switches(void) {
    switches_set_state(SW_FAN_1, g_pdu_state.cooling_command.fan1_enabled);
    switches_set_state(SW_FAN_2, g_pdu_state.cooling_command.fan2_enabled);
    switches_set_state(SW_FAN_3, g_pdu_state.cooling_command.fan3_enabled);
    switches_set_state(SW_FAN_4, g_pdu_state.cooling_command.fan4_enabled);
    switches_set_state(SW_PUMP_1, g_pdu_state.cooling_command.pump1_enabled);
    switches_set_state(SW_PUMP_2, g_pdu_state.cooling_command.pump2_enabled);
    switches_set_state(SW_HXFAN, g_pdu_state.cooling_command.hxfan_enabled);
}

static inline void set_fans(void) {
    setFan1Speed(g_pdu_state.cooling_command.fan1_enabled ? g_pdu_state.cooling_command.fan1_percent : 0);
    setFan2Speed(g_pdu_state.cooling_command.fan2_enabled ? g_pdu_state.cooling_command.fan2_percent : 0);
    setFan3Speed(g_pdu_state.cooling_command.fan3_enabled ? g_pdu_state.cooling_command.fan3_percent : 0);
    setFan4Speed(g_pdu_state.cooling_command.fan4_enabled ? g_pdu_state.cooling_command.fan4_percent : 0);
}

static inline void cooling_fsm_periodic(void) {
    next_cooling_state = cooling_state;

    switch (cooling_state) {
        case COOLING_STATE_AUTO:
            auto_periodic();
            break;

        case COOLING_STATE_MANUAL:
            manual_periodic();
            break;

        default:
            next_cooling_state = COOLING_STATE_AUTO;
            break;
    }

    cooling_state = next_cooling_state;
}

static inline void cooling_send_outputs(void) {
    CAN_SEND_coolant_out(
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}

void cooling_init(void) {
    cooling_state      = COOLING_STATE_AUTO;
    next_cooling_state = COOLING_STATE_AUTO;
    cooling_bangbang_init();
}

void cooling_periodic(void) {
    cooling_fsm_periodic();
    set_switches();
    set_fans();
    cooling_send_outputs();
}
