#ifndef A_BOX_TELEM_H_
#define A_BOX_TELEM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static constexpr uint32_t TELEM_REPORT_PERIOD_MS = 8;

typedef struct {
    uint32_t next_pack_stats_ms;
    uint32_t next_module_stats_ms;
    uint32_t next_sample_ms;
    size_t module_stats_module_idx;
    bool send_balance_stats_next;
    size_t sample_module_idx;
    size_t sample_sensor_idx;
    bool sample_is_temp;
} telem_state_t;

extern telem_state_t g_telem;

void report_telemetry(void);

#endif