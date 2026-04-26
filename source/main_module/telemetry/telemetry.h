#ifndef TELEMETRY_H
#define TELEMETRY_H

/**
 * @file telemetry.h
 * @brief MAIN MODULE telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

static constexpr uint32_t TELEMETRY_50HZ_PERIOD_MS = 20;
void report_telemetry_50hz(void);

static constexpr uint32_t TELEMETRY_1HZ_PERIOD_MS  = 1000;
void report_telemetry_1hz(void);

static constexpr uint32_t TELEMETRY_02HZ_PERIOD_MS = 5000;
void report_telemetry_02hz(void);

#endif // TELEMETRY_H