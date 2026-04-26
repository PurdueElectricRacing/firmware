/**
 * @file telemetry.c
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "telemetry.h"

#include "can_library/generated/TORQUE_VECTOR.h"
#include "sensors.h"

/**
 * @brief Reports telemetry data at 100HZ rate
 * Includes: GPS coordinates, velocity, speed, heading
 */
static_assert(GPS_COORDINATES_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
static_assert(GPS_VELOCITY_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
static_assert(GPS_SPEED_HEADING_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
void report_telemetry_100hz(void) {
    CAN_SEND_gps_coordinates(nav_pvt.latitude, nav_pvt.longitude);
    CAN_SEND_gps_velocity(nav_pvt.velNorth, nav_pvt.velEast);
    CAN_SEND_gps_speed_heading(nav_pvt.groundSpeed, nav_pvt.headingVehicle);
}

/**
 * @brief Reports telemetry data at 1HZ rate
 * Includes: GPS time (UTC)
 */
static_assert(GPS_TIME_PERIOD_MS == TELEMETRY_1HZ_PERIOD_MS);
void report_telemetry_1hz(void) {
    CAN_SEND_gps_time(
        (uint8_t)(nav_pvt.year - 2000),
        nav_pvt.month,
        nav_pvt.day,
        nav_pvt.hour,
        nav_pvt.minute,
        nav_pvt.second,
        0
    );
}