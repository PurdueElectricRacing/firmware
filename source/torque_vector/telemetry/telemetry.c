/**
 * @file telemetry.c
 * @brief TORQUE VECTOR telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "telemetry.h"

#include "can_library/generated/TORQUE_VECTOR.h"
#include "sensors.h"
#include "common/utils/clamp.h"

/**
 * @brief Reports telemetry data at 100HZ rate
 * Includes: GPS coordinates, velocity, speed, heading
 */
static_assert(GPS_COORDINATES_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
static_assert(GPS_VELOCITY_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
static_assert(GPS_SPEED_HEADING_PERIOD_MS == TELEMETRY_100HZ_PERIOD_MS);
void report_telemetry_100hz(void) {
    bool is_gps_locked =
        (nav_pvt.fixType == GPS_FIX_TYPE_GNSS_2D) ||
        (nav_pvt.fixType == GPS_FIX_TYPE_GNSS_3D) ||
        (nav_pvt.fixType == GPS_FIX_TYPE_GNSS_DEAD_RECKONING);

    if (is_gps_locked) {
        CAN_SEND_gps_coordinates(nav_pvt.latitude, nav_pvt.longitude);
        CAN_SEND_gps_velocity(nav_pvt.velNorth, nav_pvt.velEast);
        CAN_SEND_gps_speed_heading(nav_pvt.groundSpeed, nav_pvt.headingVehicle);
    }
}

/**
 * @brief Reports telemetry data at 1HZ rate
 * Includes: GPS time (UTC)
 */
static_assert(GPS_TIME_PERIOD_MS == TELEMETRY_1HZ_PERIOD_MS);
void report_telemetry_1hz(void) {
    bool is_gps_time_synced = nav_pvt.valid & GPS_VALID_FULLY_RESOLVED;

    if (is_gps_time_synced) {
        CAN_SEND_gps_time(
            (uint8_t)(nav_pvt.year - 2000),
            nav_pvt.month,
            nav_pvt.day,
            nav_pvt.hour,
            nav_pvt.minute,
            nav_pvt.second,
            (uint16_t)CLAMP(nav_pvt.nano / 1'000'000, 0, 999)
        );
    }
}