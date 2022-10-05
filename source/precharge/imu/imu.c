/**
 * @file imu.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Integration of the bmi and bsxlite filter software provided by Bosch
 * @version 0.1
 * @date 2022-10-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "imu.h"

/**
 * @brief Initializes the bsxlite library
 * 
 * @param imu_h IMU Handle containing initialized bmi088
 * @return True on success
 */
bool imu_init(IMU_Handle_t* imu_h)
{
    if (!imu_h->bmi || imu_h->bmi->accel_ready)
        return false;

    bsxlite_get_version(&imu_h->version);
    imu_h->last_result = bsxlite_init(&imu_h->inst);
    if (imu_h->last_result != BSXLITE_OK)
        return false;

    return true; 
}


void imu_periodic_update(IMU_Handle_t* imu_h)
{
    int32_t t_us;
    vector_3d_t accel_in, gyro_in;

    BMI088_readGyro(&imu_h->bmi, &gyro_in);
    BMI088_readAccel(&imu_h->bmi, &accel_in);
    // Only take a portion of the ms counter to prevent
    // overflow when converting to us
    t_us = ((int32_t) (sched.os_ticks & 0x1FFFFFF)) * 1000;
    imu_h->last_result = bsxlite_do_step(&imu_h->inst,
                                         t_us,
                                         &accel_in,
                                         &gyro_in,
                                         &imu_h->output);
    if (imu_h->last_result != BSXLITE_OK)
        return;

    imu_h->output.orientation.
    SEND_ACCEL_DATA(q_tx_can, ax, ay, az);
    SEND_GYRO_DATA(q_tx_can, gx, gy, gz);
}

void imu_periodic_orientation(IMU_Handle_t* imu_h)
{
}