#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

/**
 * @file linear_algebra.h
 * @brief Linear algebra functions utility header.
 *
 * Basic floating point vector and matrix types and operations.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <math.h>

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct {
    float data[3][3];
} matrix3x3_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} euler_angles_t;

/**
 * @brief Calculate the magnitude of a 3D vector.
 *
 * @param vec The input vector
 * @return The magnitude of the vector.
 */
static inline float vector3_magnitude(vector3_t vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

/**
 * @brief Normalize a 3D vector.
 *
 * @param vec The input vector
 * @return The normalized vector.
 */
static inline vector3_t vector3_normalize(vector3_t vec) {
    float mag = vector3_magnitude(vec);
    if (mag < 0.0001f) {
        return (vector3_t){0, 0, 0};
    }
    
    return (vector3_t){
        vec.x / mag,
        vec.y / mag,
        vec.z / mag
    };
}

/**
 * @brief Multiply a 3x3 matrix with a 3D vector.
 *
 * @param mat The the input matrix
 * @param in The input vector
 * @return The resulting vector.
 */
static inline vector3_t matrix_multiply_vector3(matrix3x3_t *mat, vector3_t *in) {
    vector3_t out;
    out.x = mat->data[0][0] * in->x + mat->data[0][1] * in->y + mat->data[0][2] * in->z;
    out.y = mat->data[1][0] * in->x + mat->data[1][1] * in->y + mat->data[1][2] * in->z;
    out.z = mat->data[2][0] * in->x + mat->data[2][1] * in->y + mat->data[2][2] * in->z;
    return out;
}

/**
 * @brief Perform Tait-Bryan ZYX Euler angle to rotation matrix conversion.
 *
 * @param angles The Euler angles.
 * @return The resulting rotation matrix.
 */
static inline matrix3x3_t tait_bryan(euler_angles_t angles) {
    matrix3x3_t rot;
    float cos_roll  = cosf(angles.roll);
    float sin_roll  = sinf(angles.roll);
    float cos_pitch = cosf(angles.pitch);
    float sin_pitch = sinf(angles.pitch);
    float cos_yaw   = cosf(angles.yaw);
    float sin_yaw   = sinf(angles.yaw);

    // ZYX Euler sequence (Yaw -> Pitch -> Roll)
    // R = Rz(yaw) * Ry(pitch) * Rx(roll)
    rot.data[0][0] = cos_pitch * cos_yaw;
    rot.data[0][1] = cos_yaw * sin_pitch * sin_roll - cos_roll * sin_yaw;
    rot.data[0][2] = sin_roll * sin_yaw + cos_roll * cos_yaw * sin_pitch;

    rot.data[1][0] = cos_pitch * sin_yaw;
    rot.data[1][1] = cos_roll * cos_yaw + sin_roll * sin_pitch * sin_yaw;
    rot.data[1][2] = cos_roll * sin_pitch * sin_yaw - cos_yaw * sin_roll;

    rot.data[2][0] = -sin_pitch;
    rot.data[2][1] = cos_pitch * sin_roll;
    rot.data[2][2] = cos_pitch * cos_roll;

    return rot;
}

#endif // LINEAR_ALGEBRA_H
