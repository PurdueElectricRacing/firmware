#ifndef CLAMP_TEST_SHIM_H
#define CLAMP_TEST_SHIM_H

int test_CLAMP_int(int input, int lower_bound, int upper_bound);
unsigned int test_CLAMP_uint(unsigned int input, unsigned int lower_bound, unsigned int upper_bound);
long test_CLAMP_long(long input, long lower_bound, long upper_bound);
unsigned long test_CLAMP_ulong(unsigned long input, unsigned long lower_bound, unsigned long upper_bound);
float test_CLAMP_float(float input, float lower_bound, float upper_bound);

#endif //CLAMP_TEST_SHIM_H