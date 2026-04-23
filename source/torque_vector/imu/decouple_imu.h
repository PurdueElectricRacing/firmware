#ifndef DECOUPLE_IMU_H
#define DECOUPLE_IMU_H

/**
 * @file decouple_imu.c
 * @brief IMU decoupling utility functions.
 *
 * Functions for calibrating and applying decoupling transformations to IMU data.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

void initialize_calibration(void);

#endif // DECOUPLE_IMU_H