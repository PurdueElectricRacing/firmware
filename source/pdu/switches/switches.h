#ifndef SWITCHES_H
#define SWITCHES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    // High power switches
    SW_PUMP_1 = 0,
    SW_PUMP_2 = 1,
    SW_SDC = 2,
    SW_HXFAN = 3,

    // High power switches (mux-sensed)
    SW_FAN_1 = 4,
    SW_FAN_2 = 5,
    SW_FAN_3 = 6,
    SW_FAN_4 = 7,
    SW_AMK1 = 8,
    SW_AMK2 = 9,

    // Low power switches
    SW_DASH = 10,
    SW_ABOX = 11,
    SW_MAIN = 12,
    SW_DLFR = 13,
    SW_DLBK = 14,

    // Not switch outputs
    CS_24V = 15,
    CS_5V = 16,
    CS_SWITCH_COUNT = 17,

    // Low power switches (no current-sense)
    SW_BLT = 18,
    // 5V switches (no current-sense)
    SW_CRIT_5V = 19,
    SW_TV = 20,
    SW_DAQ = 21,
    SW_FAN_5V = 22
} switches_t;

void switches_init(void);
void switches_periodic(void);
void switches_enable_default_rails(void);
void switches_set_state(switches_t switch_id, bool enabled);
bool switches_is_enabled(switches_t switch_id);
uint16_t switches_get_mux_adc_counts(uint8_t channel);

#endif // SWITCHES_H
