#include "sensors.h"
#include "can_library/generated/TORQUE_VECTOR.h"

void report_telemetry_10hz(void) {
    CAN_SEND_gps_coordinates(nav_pvt.latitude, nav_pvt.longitude);
    CAN_SEND_gps_velocity(nav_pvt.velNorth, nav_pvt.velEast);
    CAN_SEND_gps_speed(nav_pvt.groundSpeed, nav_pvt.headingVehicle);
}

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