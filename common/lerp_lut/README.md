# lerp_lut
The `lerp_lut` module provides a simple way to create and use lookup tables (LUTs) for linear interpolation in C.
- keys outside the LUT range will be clamped to the nearest endpoint
- the LUT must have at least 2 entries to be valid
- the keys array must be unique and sorted in ascending order (but not necessarily uniformly spaced)
- binary search is used to find the interval for interpolation to remain efficient for larger LUTs
- of course, accuracy depends on the size of the LUT and the range of keys.

#### Usage Example:
This example demonstrates a LUT being created for the exponential function. It is much faster to look up values from the LUT than to compute the exponential function directly, especially on our target platforms (Cortex-M4 with FPU).
```c++
#include "lerp_lut.h"

static constexpr int LUT_SIZE = 12;

const lut_entry_t exp_data[LUT_SIZE] = {
    {.key = -5.0f, .value = 0.007f},
    {.key = -4.0f, .value = 0.018f},
    {.key = -3.0f, .value = 0.05f},
    {.key = -2.0f, .value = 0.135f},
    {.key = -1.5f, .value = 0.223f},
    {.key = -1.0f, .value = 0.368f},
    {.key = -0.5f, .value = 0.607f},
    {.key = 0.0f, .value = 1.0f},
    {.key = 0.5f, .value = 1.649f},
    {.key = 1.0f, .value = 2.718f},
    {.key = 1.5f, .value = 4.482f},
    {.key = 2.0f, .value = 7.389f},
};

LERP_LUT_INIT(exp_lut, exp_data, LUT_SIZE);

int main() {
    float value = lut_lookup(&exp_lut, 0.75f);

    return 0;
}
```

![Example LUT](lerp12_vs_exp.png)