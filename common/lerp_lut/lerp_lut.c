#include "lerp_lut.h"

float lut_lookup(const lerp_lut_t* lut, float key) {
    const float *keys = lut->keys;
    const float *values = lut->values;
    const int size = lut->size;

    // "out of bounds" keys get clamped to the nearest value
    if (key <= keys[0]) { return values[0]; }
    if (key >= keys[size - 1]) { return values[size - 1]; }

    // binary search for the correct interval,
    // find i such that keys[i] <= key < keys[i + 1]
    int low = 0;
    int high = size - 2;
    int i = 0;
    while (low < high) {
        int mid = low + (high - low) / 2;

        if (key < keys[mid]) {
            high = mid - 1;
        } else if (key >= keys[mid + 1]) {
            low = mid + 1;
        } else {
            i = mid;
            break;
        }
    }

    // lerp between values[i] and values[i + 1]
    float t = (key - keys[i]) / (keys[i + 1] - keys[i]);
    return values[i] + (values[i + 1] - values[i]) * t;
}
