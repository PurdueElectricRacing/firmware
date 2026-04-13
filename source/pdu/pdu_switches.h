#ifndef PDU_SWITCHES_H
#define PDU_SWITCHES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    // High power switches
    SW_PUMP_1,
    SW_PUMP_2,
    SW_SDC,
    SW_HXFAN,
    // High power switches (mux-sensed)
    SW_FAN_1,
    SW_FAN_2,
    SW_FAN_3,
    SW_FAN_4,
    SW_AMK1,
    SW_AMK2,

    // Low power switches
    SW_DASH,
    SW_ABOX,
    SW_MAIN,
    SW_DLFR,
    SW_DLBK,

    // Not switch outputs
    CS_24V,
    CS_5V,
    CS_SWITCH_COUNT,

    // Low power switches (no current-sense)
    SW_BLT,
    // 5V switches (no current-sense)
    SW_CRIT_5V,
    SW_TV,
    SW_DAQ,
    SW_FAN_5V
} switches_t;

void pdu_switches_init(void);
void pdu_switches_periodic(void);
void pdu_switches_enable_default_rails(void);
void pdu_switches_set_state(switches_t switch_id, bool enabled);
bool pdu_switches_is_enabled(switches_t switch_id);
uint16_t pdu_switches_get_mux_adc_counts(uint8_t channel);

#endif // PDU_SWITCHES_H
