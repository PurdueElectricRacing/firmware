#include "clamp_test_shim.h"

// GCC reports the draft C2x value while compiling its C23 implementation.

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202000L
#error "abs tests must compile this shim as C23 or newer"
#endif

#include "clamp.h"

int test_CLAMP_i(int input, int lower_bound, int upper_bound) {
    return CLAMP(input, lower_bound, upper_bound);
}

unsigned int test_CLAMP_ui(unsigned int input, unsigned int lower_bound, unsigned int upper_bound) {
    return CLAMP(input, lower_bound, upper_bound);
}

long test_CLAMP_l(long input, long lower_bound, long upper_bound) {
    return CLAMP(input, lower_bound, upper_bound);
}

unsigned long test_CLAMP_ul(unsigned long input, unsigned long lower_bound, unsigned long upper_bound) {
    return CLAMP(input, lower_bound, upper_bound);
}

float test_CLAMP_f(float input, float lower_bound, float upper_bound) {
    return CLAMP(input, lower_bound, upper_bound);
}