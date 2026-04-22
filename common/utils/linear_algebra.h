#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#include <math.h>

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct {
    float m[3][3];
} matrix3x3_t;

static float vector3_magnitude(vector3_t vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

static vector3_t vector3_normalize(vector3_t vec) {
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

static vector3_t matrix_multiply_vector3(matrix3x3_t *mat, vector3_t *vec) {
    vector3_t out;
    out.x = mat->m[0][0] * vec->x + mat->m[0][1] * vec->y + mat->m[0][2] * vec->z;
    out.y = mat->m[1][0] * vec->x + mat->m[1][1] * vec->y + mat->m[1][2] * vec->z;
    out.z = mat->m[2][0] * vec->x + mat->m[2][1] * vec->y + mat->m[2][2] * vec->z;
    return out;
}

#endif // LINEAR_ALGEBRA_H