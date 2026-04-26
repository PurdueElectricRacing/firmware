/**
 * @file control_loop.c
 * @brief Main torque vectoring control loop
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Trevor Koessler (tkoessle@purdue.edu)
 */

#include <stdint.h>

#include "can_library/generated/TORQUE_VECTOR.h"
#include "common/utils/max.h"
#include "sensors.h"
#include "vcu.h"

static pVCU_struct pVCU;
static xVCU_struct xVCU;
static yVCU_struct yVCU;

void control_init(void) {
    // TV initialization
    pVCU = init_pVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();
}

void control_loop() {
    // load up the xVCU (input) struct with most recent data
    xVCU.VCU_MODE_REQ = VCU_MODE_AUTOCROSS;

    xVCU.THROT_RAW = can_data.pedals.throttle / 4095.0f / 100.0f; // scale to [1,0]
    xVCU.BRAKE_RAW = can_data.pedals.brake / 4095.0f / 100.0f * -1.0f; // scale to [0,-1]

    xVCU.ST_RAW = can_data.steering_angle.angle * UNPACK_COEFF_STEERING_ANGLE_ANGLE;
    xVCU.VB_RAW = can_data.pack_stats.pack_voltage * UNPACK_COEFF_PACK_STATS_PACK_VOLTAGE;
    xVCU.WM_RAW[0] = can_data.wheel_speeds.front_left;
    xVCU.WM_RAW[1] = can_data.wheel_speeds.front_right;
    xVCU.WM_RAW[2] = can_data.wheel_speeds.rear_left;
    xVCU.WM_RAW[3] = can_data.wheel_speeds.rear_right;
    xVCU.GS_RAW = nav_pvt.groundSpeed * (1000.0f); // convert mm/s to m/s
    xVCU.AV_RAW[0] = imu_data.gyro_x * UNPACK_COEFF_IMU_ANGULAR_RATE_X_AXIS;
    xVCU.AV_RAW[1] = imu_data.gyro_y * UNPACK_COEFF_IMU_ANGULAR_RATE_Y_AXIS;
    xVCU.AV_RAW[2] = imu_data.gyro_z * UNPACK_COEFF_IMU_ANGULAR_RATE_Z_AXIS;
    xVCU.IB_RAW = can_data.pack_stats.pack_current * UNPACK_COEFF_PACK_STATS_PACK_CURRENT;

    int16_t max_motor_temp = MAXOF(
        can_data.motor_temps.front_right,
        can_data.motor_temps.front_left,
        can_data.motor_temps.rear_left,
        can_data.motor_temps.rear_right
    );
    int16_t scaled_motor_temp = max_motor_temp * UNPACK_COEFF_MOTOR_TEMPS_FRONT_RIGHT;
    xVCU.MT_RAW = scaled_motor_temp;

    int16_t max_igbt_temp = MAXOF(
        can_data.igbt_temps.front_right,
        can_data.igbt_temps.front_left,
        can_data.igbt_temps.rear_left,
        can_data.igbt_temps.rear_right
    );
    int16_t scaled_igbt_temp = max_igbt_temp * UNPACK_COEFF_IGBT_TEMPS_FRONT_RIGHT;
    xVCU.IGBT_T_RAW = scaled_igbt_temp;

    xVCU.BT_RAW = 30.0f; // hardcoded to 30C

    xVCU.RG_split_FR_RAW = 0.3f; // todo driver configurable

    // step the VCU model
    vcu_step(&pVCU, &xVCU, &yVCU);

    // todo send yVCU -> torque requests
}