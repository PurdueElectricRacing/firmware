#ifndef LERP_LUT_H
#define LERP_LUT_H

typedef struct {
    const float* keys;
    const float* values;
    int size;
} lerp_lut_t;

#define LERP_LUT_INIT(name, keys, values, _size) \
    const lerp_lut_t name = { \
        .keys = keys, \
        .values = values, \
        .size = _size, \
    }; \
static_assert(sizeof(keys) / sizeof(float) == _size, "Keys and values array size must match"); \
static_assert(_size > 1, "LUT size must be greater than 1")

float lut_lookup(const lerp_lut_t* lut, float key);

#endif // LERP_LUT_H