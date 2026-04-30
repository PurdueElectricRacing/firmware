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

// init default settings
volatile vcu_mode_t vcu_mode = VCU_MODE_AUTOCROSS;
volatile vcu_settings_data_t vcu_settings[4] = {
    [VCU_MODE_ACCEL] = {
        .lateral_gain = 50,
        .longitudinal_gain = 50,
        .electronic_brake_bias = 50,
        .is_regen_enabled = true,
        .is_tv_enabled = true
    },
    [VCU_MODE_SKIDPAD] = {
        .lateral_gain = 50,
        .longitudinal_gain = 50,
        .electronic_brake_bias = 50,
        .is_regen_enabled = true,
        .is_tv_enabled = true
    },
    [VCU_MODE_AUTOCROSS] = {
        .lateral_gain = 50,
        .longitudinal_gain = 50,
        .electronic_brake_bias = 50,
        .is_regen_enabled = true,
        .is_tv_enabled = true
    },
    [VCU_MODE_ENDURANCE] = {
        .lateral_gain = 50,
        .longitudinal_gain = 50,
        .electronic_brake_bias = 50,
        .is_regen_enabled = true,
        .is_tv_enabled = true
    }
};
uint32_t last_vcu_settings_tx = 0;

static_assert(VCU_SETTINGS_LAYOUT_HASH == VCU_DRIVER_REQUEST_LAYOUT_HASH);

void control_init(void) {
    // TV initialization
    pVCU = init_pVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();
}

static inline void report_vcu_settings() {
    volatile vcu_settings_data_t *current_settings = &vcu_settings[vcu_mode];

    CAN_SEND_vcu_settings(
        vcu_mode,
        current_settings->lateral_gain,
        current_settings->longitudinal_gain,
        current_settings->electronic_brake_bias,
        current_settings->is_regen_enabled,
        current_settings->is_tv_enabled
    );
}

void vcu_driver_request_CALLBACK(void) {
    vcu_mode_t requested_mode = can_data.vcu_driver_request.vcu_mode;
    if (vcu_mode != requested_mode) {
        vcu_mode = requested_mode;
        return;
    }

    vcu_settings[vcu_mode].lateral_gain = can_data.vcu_driver_request.lateral_gain;
    vcu_settings[vcu_mode].longitudinal_gain = can_data.vcu_driver_request.longitudinal_gain;
    vcu_settings[vcu_mode].electronic_brake_bias = can_data.vcu_driver_request.electronic_brake_bias;
    vcu_settings[vcu_mode].is_regen_enabled = can_data.vcu_driver_request.is_regen_enabled;
    vcu_settings[vcu_mode].is_tv_enabled = can_data.vcu_driver_request.is_tv_enabled;

    // echo back the current settings
    report_vcu_settings();
}

void control_loop() {
    uint32_t now = xTaskGetTickCount();
    if ((now - last_vcu_settings_tx) >= VCU_SETTINGS_PERIOD_MS) { // periodic sync
        report_vcu_settings();
        last_vcu_settings_tx = now;
    }

    // load up the xVCU (input) struct with most recent data
    xVCU.VCU_MODE_REQ = vcu_mode;

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