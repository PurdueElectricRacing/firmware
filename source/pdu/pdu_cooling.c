#include "pdu_cooling.h"

#include "common/can_library/generated/PDU.h"
#include "fan_control.h"
#include "pdu_cooling_bangbang.h"
#include "pdu_state.h"
#include "pdu_switches.h"

static void pdu_cooling_compute_policy(void) {
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

    pdu_cooling_bangbang_update(&g_pdu_state.cooling_command);
}

static void pdu_cooling_apply_outputs(void) {
    pdu_switches_set_state(SW_FAN_1, g_pdu_state.cooling_command.fan1_enabled);
    pdu_switches_set_state(SW_FAN_2, g_pdu_state.cooling_command.fan2_enabled);
    pdu_switches_set_state(SW_FAN_3, g_pdu_state.cooling_command.fan3_enabled);
    pdu_switches_set_state(SW_FAN_4, g_pdu_state.cooling_command.fan4_enabled);
    pdu_switches_set_state(SW_PUMP_1, g_pdu_state.cooling_command.pump1_enabled);
    pdu_switches_set_state(SW_PUMP_2, g_pdu_state.cooling_command.pump2_enabled);
    pdu_switches_set_state(SW_HXFAN, g_pdu_state.cooling_command.hxfan_enabled);

    setFan1Speed(g_pdu_state.cooling_command.fan1_enabled ? g_pdu_state.cooling_command.fan1_percent : 0);
    setFan2Speed(g_pdu_state.cooling_command.fan2_enabled ? g_pdu_state.cooling_command.fan2_percent : 0);
    setFan3Speed(g_pdu_state.cooling_command.fan3_enabled ? g_pdu_state.cooling_command.fan3_percent : 0);
    setFan4Speed(g_pdu_state.cooling_command.fan4_enabled ? g_pdu_state.cooling_command.fan4_percent : 0);
}

static void pdu_cooling_send_outputs(void) {
    CAN_SEND_coolant_out(
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}

void pdu_cooling_init(void) {
    g_pdu_state.cooling_command.fan1_percent = 10;
    g_pdu_state.cooling_command.fan2_percent = 10;
    g_pdu_state.cooling_command.fan3_percent = 10;
    g_pdu_state.cooling_command.fan4_percent = 10;

    g_pdu_state.cooling_command.fan1_enabled = true;
    g_pdu_state.cooling_command.fan2_enabled = true;
    g_pdu_state.cooling_command.fan3_enabled = true;
    g_pdu_state.cooling_command.fan4_enabled = true;
    g_pdu_state.cooling_command.pump1_enabled = true;
    g_pdu_state.cooling_command.pump2_enabled = true;
    g_pdu_state.cooling_command.hxfan_enabled = true;
    g_pdu_state.cooling_command.batt_fan_autospeed_enabled = false;
    g_pdu_state.cooling_command.motor_fan_autospeed_enabled = false;

    pdu_cooling_bangbang_init();
}

void pdu_cooling_periodic(void) {
    pdu_cooling_compute_policy();
    pdu_cooling_apply_outputs();
    pdu_cooling_send_outputs();
}
