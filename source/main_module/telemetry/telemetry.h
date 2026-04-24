#ifndef TELEMETRY_H
#define TELEMETRY_H

/**
 * @file telemetry.h
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include "can_library/generated/MAIN_MODULE.h"

static constexpr uint32_t TELEMETRY_50HZ_PERIOD_MS = 20;
static_assert(TELEMETRY_50HZ_PERIOD_MS == WHEEL_SPEEDS_PERIOD_MS);
void report_telemetry_50hz(void); // 50hz

static constexpr uint32_t TELEMETRY_1HZ_PERIOD_MS  = 1000;
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVA_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVB_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVC_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVD_DIAGNOSTICS_PERIOD_MS);
void report_telemetry_1hz(void);  // 1hz

static constexpr uint32_t TELEMETRY_02HZ_PERIOD_MS = 5000;
static_assert(TELEMETRY_02HZ_PERIOD_MS == MOTOR_TEMPS_PERIOD_MS);
static_assert(TELEMETRY_02HZ_PERIOD_MS == IGBT_TEMPS_PERIOD_MS);
void report_telemetry_02hz(void); // 0.2hz

#endif // TELEMETRY_H