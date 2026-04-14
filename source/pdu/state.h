#ifndef PDU_STATE_H
#define PDU_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "switches.h"

typedef struct {
    uint8_t fan1_percent;
    uint8_t fan2_percent;
    uint8_t fan3_percent;
    uint8_t fan4_percent;
    bool fan1_enabled;
    bool fan2_enabled;
    bool fan3_enabled;
    bool fan4_enabled;
    bool pump1_enabled;
    bool pump2_enabled;
    bool hxfan_enabled;
    bool batt_fan_autospeed_enabled;
    bool motor_fan_autospeed_enabled;
} pdu_cooling_command_t;

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
    pdu_cooling_command_t cooling_command;
} pdu_state_t;

extern pdu_state_t g_pdu_state;

void state_init_defaults(void);

#endif // PDU_STATE_H
