#include "common/utils/linear_algebra.h"
#include "can_library/generated/TORQUE_VECTOR.h"
#include <math.h>

matrix3x3_t calibration_matrix;
volatile bool is_calibration_started = false;
volatile bool is_calibration_finished = false;

// Statistics for calibration
static vector3_t accel_sum = {
    .x = 0,
    .y = 0,
    .z = 0
};
static uint32_t accel_samples = 0;
#define CALIBRATION_SAMPLES 100

void initialize_calibration() {
    is_calibration_started = true;
    is_calibration_finished = false;
    accel_sum.x = 0;
    accel_sum.y = 0;
    accel_sum.z = 0;
    accel_samples = 0;
}

static void finish_calibration(void) {
    if (accel_samples < CALIBRATION_SAMPLES) return;

    vector3_t avg_accel = {
        .x = accel_sum.x / accel_samples,
        .y = accel_sum.y / accel_samples,
        .z = accel_sum.z / accel_samples
    };

    vector3_t unit_g = vector3_normalize(avg_accel);

    // Target is Z+ up (unit_g should be [0, 0, 1] in vehicle frame)
    // Pitch is rotation around Y: sin(pitch) = -accel_x
    // Roll is rotation around X: sin(roll) = accel_y / cos(pitch)
    euler_angles_t angles = {0};
    angles.pitch = asinf(-unit_g.x);
    angles.roll = atan2f(unit_g.y, unit_g.z);
    angles.yaw = 0.0f; // Yaw cannot be determined by gravity alone

    calibration_matrix = matrix3x3_from_euler(angles);
    is_calibration_finished = true;
    is_calibration_started = false;
}

void IZZE_angular_rate_CALLBACK(void) {
    if (!is_calibration_finished) {
        return;
    }

    vector3_t raw_data = {
        .x = can_data.IZZE_angular_rate.X_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_X_AXIS,
        .y = can_data.IZZE_angular_rate.Y_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_Y_AXIS,
        .z = can_data.IZZE_angular_rate.Z_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_Z_AXIS
    };

    vector3_t calibrated_data = matrix_multiply_vector3(&calibration_matrix, &raw_data);

    CAN_SEND_IMU_angular_rate(
        calibrated_data.x * PACK_COEFF_IMU_ANGULAR_RATE_X_AXIS,
        calibrated_data.y * PACK_COEFF_IMU_ANGULAR_RATE_Y_AXIS,
        calibrated_data.z * PACK_COEFF_IMU_ANGULAR_RATE_Z_AXIS,
        0
    );
}

void IZZE_acceleration_CALLBACK(void) {
    vector3_t raw_data = {
        .x = can_data.IZZE_acceleration.X_axis * UNPACK_COEFF_IZZE_ACCELERATION_X_AXIS,
        .y = can_data.IZZE_acceleration.Y_axis * UNPACK_COEFF_IZZE_ACCELERATION_Y_AXIS,
        .z = can_data.IZZE_acceleration.Z_axis * UNPACK_COEFF_IZZE_ACCELERATION_Z_AXIS
    };

    if (is_calibration_started) {
        accel_sum.x += raw_data.x;
        accel_sum.y += raw_data.y;
        accel_sum.z += raw_data.z;
        accel_samples++;

        if (accel_samples >= CALIBRATION_SAMPLES) {
            finish_calibration();
        }
    }

    if (!is_calibration_finished) {
        return;
    }

    vector3_t calibrated_data = matrix_multiply_vector3(&calibration_matrix, &raw_data);

    CAN_SEND_IMU_acceleration(
        calibrated_data.x * PACK_COEFF_IMU_ACCELERATION_X_AXIS,
        calibrated_data.y * PACK_COEFF_IMU_ACCELERATION_Y_AXIS,
        calibrated_data.z * PACK_COEFF_IMU_ACCELERATION_Z_AXIS,
        0
    );
}
