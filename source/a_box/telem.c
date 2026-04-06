#include "telem.h"

#include "adbms.h"
#include "common/can_library/generated/A_BOX.h"
#include "common/can_library/generated/VCAN.h"

extern adbms_bms_t g_bms;
extern volatile uint16_t isense_raw;

static constexpr uint32_t INDIVIDUAL_SAMPLE_SEND_PERIOD_MS = 50;

static telem_state_t g_telem = {0};

// DHAB S/134 current sensor conversion
static int16_t isense_to_current(uint16_t raw_isense) {
    static constexpr float ADC_VREF = 3.3f;
    static constexpr float ADC_MAX  = 4095.0f;

    static constexpr float DIV_R1 = 2400.0f;
    static constexpr float DIV_R2 = 4700.0f;

    static constexpr float V_OFFSET = 2.5f;
    static constexpr float G = 10.0e-3f;

    float divider_gain = (DIV_R1 + DIV_R2) / DIV_R2;
    float v_adc = raw_isense * ADC_VREF / ADC_MAX;
    float v_sensor = v_adc * divider_gain;
    float current = (v_sensor - V_OFFSET) / G;
    float scaled_current = current * PACK_COEFF_PACK_CORE_STATS_PACK_CURRENT;

    return (int16_t)scaled_current;
}

static uint16_t balance_mask_from_module(size_t module_idx) {
    uint16_t balance_mask = 0;

    for (size_t cell_idx = 0; cell_idx < ADBMS6380_CELL_COUNT; cell_idx++) {
        if (g_bms.modules[module_idx].is_discharging[cell_idx]) {
            balance_mask |= (uint16_t)(1u << cell_idx);
        }
    }

    return balance_mask;
}

static void send_module_voltage_stats(size_t module_idx) {
    CAN_SEND_module_voltage_stats((uint8_t)module_idx,
                                  (uint16_t)(g_bms.modules[module_idx].min_voltage * PACK_COEFF_MODULE_VOLTAGE_STATS_MIN_VOLTAGE),
                                  (uint16_t)(g_bms.modules[module_idx].max_voltage * PACK_COEFF_MODULE_VOLTAGE_STATS_MAX_VOLTAGE),
                                  (uint16_t)(g_bms.modules[module_idx].sum_voltage * PACK_COEFF_MODULE_VOLTAGE_STATS_SUM_VOLTAGE));
}

static void send_module_temp_balance_stats(size_t module_idx) {
    CAN_SEND_module_temp_balance_stats((uint8_t)module_idx,
                                       (int16_t)(g_bms.modules[module_idx].min_therm_temp * PACK_COEFF_MODULE_TEMP_BALANCE_STATS_MIN_TEMP),
                                       (int16_t)(g_bms.modules[module_idx].max_therm_temp * PACK_COEFF_MODULE_TEMP_BALANCE_STATS_MAX_TEMP),
                                       balance_mask_from_module(module_idx));
}

static void send_pack_voltage_temp_stats(void) {
    CAN_SEND_pack_voltage_temp_stats((uint16_t)(g_bms.min_voltage * PACK_COEFF_PACK_VOLTAGE_TEMP_STATS_MIN_CELL_VOLTAGE),
                                    (uint16_t)(g_bms.max_voltage * PACK_COEFF_PACK_VOLTAGE_TEMP_STATS_MAX_CELL_VOLTAGE),
                                    (int16_t)(g_bms.min_therm_temp * PACK_COEFF_PACK_VOLTAGE_TEMP_STATS_MIN_THERM_TEMP),
                                    (int16_t)(g_bms.max_therm_temp * PACK_COEFF_PACK_VOLTAGE_TEMP_STATS_MAX_THERM_TEMP));
}

static void send_individual_module_sample(size_t module_idx, bool is_temp, size_t sample_idx) {
    if (is_temp) {
        CAN_SEND_module_sample((uint8_t)module_idx,
                               (uint8_t)sample_idx,
                               true,
                               (int16_t)(g_bms.modules[module_idx].therms_temps[sample_idx] * PACK_COEFF_MODULE_SAMPLE_SAMPLE_VALUE));
    } else {

		CAN_SEND_module_sample((uint8_t)module_idx,
							   (uint8_t)sample_idx,
                           	   false,
                           	   (int16_t)(g_bms.modules[module_idx].cell_voltages[sample_idx] * PACK_COEFF_MODULE_SAMPLE_SAMPLE_VALUE));
	}
}

// Call at PACK_CORE_STATS_PERIOD_MS interval
void report_telemetry(void) {
    uint32_t now = OS_TICKS;

    uint16_t pack_voltage = (uint16_t)(g_bms.sum_voltage * PACK_COEFF_PACK_CORE_STATS_PACK_VOLTAGE);
    int16_t pack_current = isense_to_current(isense_raw);
    int16_t avg_temp = (int16_t)(g_bms.avg_therm_temp * PACK_COEFF_PACK_CORE_STATS_AVG_TEMP);

    CAN_SEND_pack_core_stats(pack_voltage, pack_current, avg_temp);

    if (now >= g_telem.next_pack_voltage_temp_stats_ms) {
        send_pack_voltage_temp_stats();
        g_telem.next_pack_voltage_temp_stats_ms = now + PACK_VOLTAGE_TEMP_STATS_PERIOD_MS;
    }

    while (now >= g_telem.next_module_stats_ms) {
        if (g_telem.send_balance_stats_next) {
            send_module_temp_balance_stats(g_telem.module_stats_module_idx);
            g_telem.module_stats_module_idx = (g_telem.module_stats_module_idx + 1) % ADBMS_MODULE_COUNT;
        } else {
            send_module_voltage_stats(g_telem.module_stats_module_idx);
        }

        g_telem.send_balance_stats_next = !g_telem.send_balance_stats_next;
        g_telem.next_module_stats_ms += MODULE_VOLTAGE_STATS_PERIOD_MS;
    }

    while (now >= g_telem.next_sample_ms) {
        send_individual_module_sample(g_telem.sample_module_idx, g_telem.sample_is_temp, g_telem.sample_sensor_idx);

        if (g_telem.sample_is_temp) {
            g_telem.sample_sensor_idx++;
            if (g_telem.sample_sensor_idx >= ADBMS6380_GPIO_COUNT) {
                g_telem.sample_sensor_idx = 0;
                g_telem.sample_is_temp    = false;
                g_telem.sample_module_idx = (g_telem.sample_module_idx + 1) % ADBMS_MODULE_COUNT;
            }
        } else {
            g_telem.sample_sensor_idx++;
            if (g_telem.sample_sensor_idx >= ADBMS6380_CELL_COUNT) {
                g_telem.sample_sensor_idx = 0;
                g_telem.sample_is_temp    = true;
            }
        }

        g_telem.next_sample_ms += INDIVIDUAL_SAMPLE_SEND_PERIOD_MS;
    }
}