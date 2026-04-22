#include "can_library/generated/TORQUE_VECTOR.h"

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct {
    float m[3][3];
} rotation_matrix_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} euler_angles_t;

rotation_matrix_t calibration_matrix;
bool calibration_initialized = false;


static_assert(
    IMU_ANGULAR_RATE_LAYOUT_HASH == IZZE_ANGULAR_RATE_LAYOUT_HASH,
    "IMU_angular_rate and IZZE_angular_rate message layouts do not match!"
);

static_assert(
    IMU_ACCELERATION_LAYOUT_HASH == IZZE_ACCELERATION_LAYOUT_HASH,
    "IMU_acceleration and IZZE_acceleration message layouts do not match!"
);

// on boot, find the "downwards" direction
static euler_angles_t find_initial_orientation(void) {
    euler_angles_t angles = {0};
    // read the IMU data and calculate the initial orientation
    return angles;
}

static euler_angles_t find_rotation_to_align_with_gravity(euler_angles_t initial_orientation) {
    euler_angles_t rotation = {0};
    // calculate the rotation needed to align the IMU's "down" with gravity
    return rotation;
}

static rotation_matrix_t calculate_rotation_matrix(euler_angles_t angles) {
    rotation_matrix_t rot = {0};
    // calculate the rotation matrix from the Euler angles
    return rot;
}


static vector3_t apply_rotation_matrix(vector3_t *in, rotation_matrix_t *rot) {
    vector3_t out = {0};
    // apply the rotation matrix to the IMU data
    return out;
}

void initialize_calibration() {
    euler_angles_t initial_orientation = find_initial_orientation();
    euler_angles_t rotation_to_gravity = find_rotation_to_align_with_gravity(initial_orientation);
    calibration_matrix = calculate_rotation_matrix(rotation_to_gravity);
    calibration_initialized = true;
}

void IZZE_angular_rate_CALLBACK() {
    if (!calibration_initialized) {
        return; // send nothing until we have a valid calibration
    }

    vector3_t raw_data = {
        .x = can_data.IZZE_angular_rate.X_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_X_AXIS,
        .y = can_data.IZZE_angular_rate.Y_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_Y_AXIS,
        .z = can_data.IZZE_angular_rate.Z_axis * UNPACK_COEFF_IZZE_ANGULAR_RATE_Z_AXIS
    };

    vector3_t calibrated_data = apply_rotation_matrix(&raw_data, &calibration_matrix);

    CAN_SEND_IMU_angular_rate(
        calibrated_data.x * PACK_COEFF_IMU_ANGULAR_RATE_X_AXIS,
        calibrated_data.y * PACK_COEFF_IMU_ANGULAR_RATE_Y_AXIS,
        calibrated_data.z * PACK_COEFF_IMU_ANGULAR_RATE_Z_AXIS,
        0
    );
}

void IZZE_acceleration_CALLBACK() {
    if (!calibration_initialized) {
        return; // send nothing until we have a valid calibration
    }

    vector3_t raw_data = {
        .x = can_data.IZZE_acceleration.X_axis * UNPACK_COEFF_IZZE_ACCELERATION_X_AXIS,
        .y = can_data.IZZE_acceleration.Y_axis * UNPACK_COEFF_IZZE_ACCELERATION_Y_AXIS,
        .z = can_data.IZZE_acceleration.Z_axis * UNPACK_COEFF_IZZE_ACCELERATION_Z_AXIS
    };

    vector3_t calibrated_data = apply_rotation_matrix(&raw_data, &calibration_matrix);

    CAN_SEND_IMU_acceleration(
        calibrated_data.x * PACK_COEFF_IMU_ACCELERATION_X_AXIS,
        calibrated_data.y * PACK_COEFF_IMU_ACCELERATION_Y_AXIS,
        calibrated_data.z * PACK_COEFF_IMU_ACCELERATION_Z_AXIS,
        can_data.IZZE_acceleration.temperature
    );
}