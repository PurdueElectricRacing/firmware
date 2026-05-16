#include "state.h"

#include <string.h>

pdu_state_t g_pdu_state;

void state_init_defaults(void) {
    memset(&g_pdu_state, 0, sizeof(g_pdu_state));

    g_pdu_state.next_mux_channel               = 0;
    g_pdu_state.next_rail_fault_index          = 0;
}
