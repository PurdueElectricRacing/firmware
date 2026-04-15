#include "cooling.h"

#include "common/can_library/generated/PDU.h"
#include "cooling_bangbang.h"
#include "fan_control.h"
#include "state.h"
#include "switches.h"

// ! move cooling bang bang into here aswell

typedef enum {
    COOLING_STATE_AUTO   = 0,
    COOLING_STATE_MANUAL = 1 // ! leave manual empty for now, auto should be tuned well
} cooling_state_t;

static cooling_state_t cooling_state = COOLING_STATE_AUTO;
static cooling_state_t next_cooling_state = COOLING_STATE_AUTO;

static inline void auto_periodic() {
    // update bang bangs

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

static inline void set_fans() {
    setFan1Speed(g_pdu_state.cooling_command.fan1_enabled ? g_pdu_state.cooling_command.fan1_percent : 0);
    setFan2Speed(g_pdu_state.cooling_command.fan2_enabled ? g_pdu_state.cooling_command.fan2_percent : 0);
    setFan3Speed(g_pdu_state.cooling_command.fan3_enabled ? g_pdu_state.cooling_command.fan3_percent : 0);
    setFan4Speed(g_pdu_state.cooling_command.fan4_enabled ? g_pdu_state.cooling_command.fan4_percent : 0);
}

void cooling_fsm_periodic() {

    switch(cooling_state) {
        case COOLING_STATE_AUTO:
            auto_periodic();


            // if dashboard manual override !stale
                // next_cooling_state = MANUAL
            break;
        case COOLING_STATE_MANUAL:
            // todo update manual controls
            
            // if dashboard manual override stale
                // next_cooling_state = AUTO
            // else if temps are getting too high
                // next_cooling_state = AUTO
            break;
    }

    set_switches();
    set_fans();

    // report telemetry
    CAN_SEND_coolant_out(
        g_pdu_state.cooling_command.fan1_percent,
        g_pdu_state.cooling_command.fan2_percent,
        g_pdu_state.cooling_command.pump2_enabled,
        g_pdu_state.cooling_command.hxfan_enabled,
        g_pdu_state.cooling_command.pump1_enabled
    );
}
