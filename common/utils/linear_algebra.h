#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

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

static inline float vector3_magnitude(vector3_t vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

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

static inline vector3_t matrix_multiply_vector3(matrix3x3_t *mat, vector3_t *in) {
    vector3_t out;
    out.x = mat->data[0][0] * in->x + mat->data[0][1] * in->y + mat->data[0][2] * in->z;
    out.y = mat->data[1][0] * in->x + mat->data[1][1] * in->y + mat->data[1][2] * in->z;
    out.z = mat->data[2][0] * in->x + mat->data[2][1] * in->y + mat->data[2][2] * in->z;
    return out;
}

static inline matrix3x3_t matrix3x3_from_euler(euler_angles_t angles) {
    matrix3x3_t rot;
    float cr = cosf(angles.roll);
    float sr = sinf(angles.roll);
    float cp = cosf(angles.pitch);
    float sp = sinf(angles.pitch);
    float cy = cosf(angles.yaw);
    float sy = sinf(angles.yaw);

    // ZYX Euler sequence (Yaw -> Pitch -> Roll)
    // R = Rz(yaw) * Ry(pitch) * Rx(roll)
    rot.data[0][0] = cp * cy;
    rot.data[0][1] = cy * sp * sr - cr * sy;
    rot.data[0][2] = sr * sy + cr * cy * sp;

    rot.data[1][0] = cp * sy;
    rot.data[1][1] = cr * cy + sr * sp * sy;
    rot.data[1][2] = cr * sp * sy - cy * sr;

    rot.data[2][0] = -sp;
    rot.data[2][1] = cp * sr;
    rot.data[2][2] = cp * cr;

    return rot;
}

#endif // LINEAR_ALGEBRA_H
