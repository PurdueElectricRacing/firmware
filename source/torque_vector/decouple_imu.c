#include "common/utils/linear_algebra.h"
#include "can_library/generated/TORQUE_VECTOR.h"
#include <math.h>

typedef enum {
    DECOUPLING_STATE_IDLE,
    DECOUPLING_STATE_CALIBRATING,
    DECOUPLING_STATE_ACTIVE
} decoupling_state_t;

static volatile decoupling_state_t decoupling_state = DECOUPLING_STATE_IDLE;
matrix3x3_t calibration_matrix;

// Statistics for calibration
static constexpr uint32_t CALIBRATION_SAMPLE_THRESHOLD = 100;
static vector3_t accel_sum = {
    .x = 0,
    .y = 0,
    .z = 0
};
static uint32_t num_samples = 0;

void initialize_calibration() {
    decoupling_state = DECOUPLING_STATE_CALIBRATING;
    accel_sum.x      = 0;
    accel_sum.y      = 0;
    accel_sum.z      = 0;
    num_samples      = 0;
}

static void finish_calibration(void) {
    if (num_samples < CALIBRATION_SAMPLE_THRESHOLD) {
        return;
    }

    vector3_t avg_accel = {
        .x = accel_sum.x / num_samples,
        .y = accel_sum.y / num_samples,
        .z = accel_sum.z / num_samples
    };

    vector3_t unit_g = vector3_normalize(avg_accel);

    // Target coordinate frame: X+ is forward, Y+ is left, Z+ is up
    // unit_g should be [0, 0, 1]
    euler_angles_t angles = {0};

    angles.pitch = asinf(-unit_g.x); // Pitch is rotation around Y: sin(pitch) = -accel_x
    angles.roll  = atan2f(unit_g.y, unit_g.z); // Roll is rotation around X: sin(roll) = accel_y / cos(pitch)
    angles.yaw   = 0.0f; // Yaw cannot be determined by gravity alone

    calibration_matrix = tait_bryan(angles);
    decoupling_state = DECOUPLING_STATE_ACTIVE;
}

void IZZE_angular_rate_CALLBACK(void) {
    if (decoupling_state != DECOUPLING_STATE_ACTIVE) {
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

    if (decoupling_state == DECOUPLING_STATE_CALIBRATING) {
        accel_sum.x += raw_data.x;
        accel_sum.y += raw_data.y;
        accel_sum.z += raw_data.z;
        num_samples++;

        if (num_samples >= CALIBRATION_SAMPLE_THRESHOLD) {
            finish_calibration();
        }
    }

    if (decoupling_state != DECOUPLING_STATE_ACTIVE) {
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
