#ifndef TELEMETRY_H
#define TELEMETRY_H

/**
 * @file telemetry.h
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

void report_telemetry_50hz(void); // 50hz
void report_telemetry_1hz(void);  // 1hz
void report_telemetry_02hz(void); // 0.2hz

#endif // TELEMETRY_H