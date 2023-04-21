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
#include "SFS_pp.h"

extern q_handle_t q_tx_can;
/**
 * @brief Initializes the bsxlite library
 *
 * @param imu_h IMU Handle containing initialized bmi088
 * @return True on success
 */
bool imu_init(IMU_Handle_t *imu_h)
{
    if (!imu_h->bmi || !imu_h->bmi->accel_ready)
        return false;

    bsxlite_get_version(&imu_h->version);
    imu_h->last_result = bsxlite_init(&imu_h->inst);
    if (imu_h->last_result != BSXLITE_OK)
        return false;

    return true;
}

int32_t t_us = 0;
void imu_periodic(IMU_Handle_t *imu_h, ExtU *rtU)
{
    vector_3d_t accel_in, gyro_in;
    int16_t h, p, r, y;

    BMI088_readGyro(imu_h->bmi, &gyro_in);
    BMI088_readAccel(imu_h->bmi, &accel_in);
    // Only take a portion of the ms counter to prevent
    // overflow when converting to us
    // t_us = ((int32_t) (sched.os_ticks & 0x1FFFFFF)) * 1000;
    imu_h->last_result = bsxlite_do_step(&imu_h->inst,
                                         t_us,
                                         &accel_in,
                                         &gyro_in,
                                         &imu_h->output);
    if (imu_h->last_result != BSXLITE_OK)
        return;

    p = (int16_t)(imu_h->output.orientation.pitch * 10.0f / DEG_TO_RAD);
    r = (int16_t)(imu_h->output.orientation.roll * 10.0f / DEG_TO_RAD);
    y = (int16_t)(imu_h->output.orientation.yaw * 10.0f / DEG_TO_RAD);

    // SEND_ANGLE_DATA(q_tx_can, p, r, y);
    // SEND_ACCEL_DATA(q_tx_can, (int16_t) (accel_in.x * 100), (int16_t) (accel_in.y * 100), (int16_t) (accel_in.z * 100));
    // SEND_GYRO_DATA(q_tx_can, (int16_t) (gyro_in.x * 100), (int16_t) (gyro_in.y * 100), (int16_t) (gyro_in.z * 100));

    rtU->gyro[0] = CLAMP(gyro_in.x * ACC_CALIBRATION, MIN_GYRO, MAX_GYRO);
    rtU->gyro[1] = CLAMP(gyro_in.y * ACC_CALIBRATION, MIN_GYRO, MAX_GYRO);
    rtU->gyro[2] = CLAMP(gyro_in.z * ACC_CALIBRATION, MIN_GYRO, MAX_GYRO);

    rtU->acc[0] = CLAMP(accel_in.x * GYRO_CALIBRATION, MIN_ACC, MAX_ACC);
    rtU->acc[1] = CLAMP(accel_in.y * GYRO_CALIBRATION, MIN_ACC, MAX_ACC);
    rtU->acc[2] = CLAMP(accel_in.z * GYRO_CALIBRATION, MIN_ACC, MAX_ACC);

    SEND_IMU_GYRO(q_tx_can, (int16_t)(gyro_in.x * 100), (int16_t)(gyro_in.y * 100), (int16_t)(gyro_in.z * 100));
    SEND_IMU_ACCEL(q_tx_can, (int16_t)(accel_in.x * 100), (int16_t)(accel_in.y * 100), (int16_t)(accel_in.z * 100));
    t_us += 10000;
}