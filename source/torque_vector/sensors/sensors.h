#ifndef SENSORS_H
#define SENSORS_H

/**
 * @file sensors.h
 * @brief IMU decoupling utility functions.
 *
 * Functions for calibrating and applying decoupling transformations to IMU data.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/ublox/nav_pvt.h"
#include "common/ublox/nav_relposned.h"

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float accel_x;
    float accel_y;
    float accel_z;
} imu_data_t;

extern NAV_PVT_data_t nav_pvt;
extern NAV_RELPOSNED_data_t nav_relpos;
extern imu_data_t imu_data;

void gps_periodic(void);
void initialize_calibration(void);
void IZZE_angular_rate_CALLBACK(void);
void IZZE_acceleration_CALLBACK(void);

#endif // SENSORS_H