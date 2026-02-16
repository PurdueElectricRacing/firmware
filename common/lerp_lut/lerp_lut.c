#include "lerp_lut.h"

float lut_lookup(const lerp_lut_t* lut, float key) {
    const lut_entry_t *entries = lut->entries;
    const int size = lut->size;

    // "out of bounds" keys get clamped to the nearest value
    if (key <= entries[0].key) { return entries[0].value; }
    if (key >= entries[size - 1].key) { return entries[size - 1].value; }

    // binary search for the correct interval
    int low = 1;
    int high = size - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;

        if (key < entries[mid].key) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    // keys[i] <= key < keys[i + 1]
    int i = low - 1;

    // lerp between values[i] and values[i + 1]
    float t = (key - entries[i].key) / (entries[i + 1].key - entries[i].key);
    return entries[i].value + (entries[i + 1].value - entries[i].value) * t;
}
