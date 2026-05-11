#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "switches.h"

typedef struct {
    uint16_t in_24v_mv;
    uint16_t out_5v_mv;
    uint16_t out_3v3_mv;
} pdu_rail_voltage_mv_t;

typedef struct {
    pdu_rail_voltage_mv_t rail_voltage_mv;
    uint16_t switch_current_ma[CS_SWITCH_COUNT];
    uint16_t mux_adc_counts[6];
    uint8_t next_mux_channel;
    uint8_t next_rail_fault_index;
} pdu_state_t;

extern pdu_state_t g_pdu_state;

void state_init_defaults(void);

#endif // STATE_H
