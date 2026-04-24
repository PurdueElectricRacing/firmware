#ifndef DECOUPLE_IMU_H
#define DECOUPLE_IMU_H

/**
 * @file decouple_imu.h
 * @brief IMU decoupling utility functions.
 *
 * Functions for calibrating and applying decoupling transformations to IMU data.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

void initialize_calibration(void);
void IZZE_angular_rate_CALLBACK(void);
void IZZE_acceleration_CALLBACK(void);

#endif // DECOUPLE_IMU_H