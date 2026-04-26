#ifndef TELEMETRY_H
#define TELEMETRY_H

/**
 * @file telemetry.h
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

static constexpr uint32_t TELEMETRY_10HZ_PERIOD_MS = 100;
void report_telemetry_10hz(void);

static constexpr uint32_t TELEMETRY_1HZ_PERIOD_MS = 1000;
void report_telemetry_1hz(void);

#endif // TELEMETRY_H