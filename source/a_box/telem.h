#ifndef A_BOX_TELEM_H_
#define A_BOX_TELEM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t next_pack_core_stats_ms;
    uint32_t next_pack_voltage_temp_stats_ms;
    uint32_t next_module_stats_ms;
    uint32_t next_sample_ms;
    size_t module_stats_module_idx;
    bool send_balance_stats_next;
    size_t sample_module_idx;
    size_t sample_sensor_idx;
    bool sample_is_temp;
} telem_state_t;


void report_telemetry(void);

#endif