#include "cooling.h"

#include "common/can_library/generated/PDU.h"
#include "cooling_bangbang.h"
#include "fan_control.h"
#include "state.h"
#include "switches.h"

static void cooling_compute_policy(void) {
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

static void cooling_apply_outputs(void) {
    switches_set_state(SW_FAN_1, g_pdu_state.cooling_command.fan1_enabled);
    switches_set_state(SW_FAN_2, g_pdu_state.cooling_command.fan2_enabled);
    switches_set_state(SW_FAN_3, g_pdu_state.cooling_command.fan3_enabled);
    switches_set_state(SW_FAN_4, g_pdu_state.cooling_command.fan4_enabled);
    switches_set_state(SW_PUMP_1, g_pdu_state.cooling_command.pump1_enabled);
    switches_set_state(SW_PUMP_2, g_pdu_state.cooling_command.pump2_enabled);
    switches_set_state(SW_HXFAN, g_pdu_state.cooling_command.hxfan_enabled);

    setFan1Speed(g_pdu_state.cooling_command.fan1_enabled ? g_pdu_state.cooling_command.fan1_percent : 0);
    setFan2Speed(g_pdu_state.cooling_command.fan2_enabled ? g_pdu_state.cooling_command.fan2_percent : 0);
    setFan3Speed(g_pdu_state.cooling_command.fan3_enabled ? g_pdu_state.cooling_command.fan3_percent : 0);
    setFan4Speed(g_pdu_state.cooling_command.fan4_enabled ? g_pdu_state.cooling_command.fan4_percent : 0);
}

static void cooling_send_outputs(void) {
    CAN_SEND_coolant_out(
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}

void cooling_init(void) {
    cooling_bangbang_init();
}

void cooling_periodic(void) {
    cooling_compute_policy();
    cooling_apply_outputs();
    cooling_send_outputs();
}
