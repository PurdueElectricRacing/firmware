#include "pdu_state.h"

#include <string.h>

pdu_state_t g_pdu_state;

void pdu_state_init_defaults(void) {
    memset(&g_pdu_state, 0, sizeof(g_pdu_state));

    g_pdu_state.next_mux_channel               = 0;
    g_pdu_state.next_rail_fault_index          = 0;
    g_pdu_state.cooling_command.fan1_percent   = 10;
    g_pdu_state.cooling_command.fan2_percent   = 10;
    g_pdu_state.cooling_command.fan3_percent   = 10;
    g_pdu_state.cooling_command.fan4_percent   = 10;
    g_pdu_state.cooling_command.fan1_enabled   = true;
    g_pdu_state.cooling_command.fan2_enabled   = true;
    g_pdu_state.cooling_command.fan3_enabled   = true;
    g_pdu_state.cooling_command.fan4_enabled   = true;
    g_pdu_state.cooling_command.pump1_enabled  = true;
    g_pdu_state.cooling_command.pump2_enabled  = true;
    g_pdu_state.cooling_command.hxfan_enabled  = true;
}
