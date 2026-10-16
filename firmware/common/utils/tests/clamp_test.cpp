#include <gtest/gtest.h>

extern "C" {
#include "clamp_test_shim.h"
}

TEST(ClampTest, ClampsInt) {
    EXPECT_EQ(test_CLAMP_int(-4, 1, 3), 1);
    EXPECT_EQ(test_CLAMP_int(7, 1, 3), 3);
    EXPECT_EQ(test_CLAMP_int(2, 1, 3), 2);
}

TEST(ClampTest, ClampsUnsignedInt) {
    EXPECT_EQ(test_CLAMP_uint((unsigned int) 4, (unsigned int) 7, (unsigned int) 10), (unsigned int) 7);
    EXPECT_EQ(test_CLAMP_uint((unsigned int) 14, (unsigned int) 7, (unsigned int) 10), (unsigned int) 10);
    EXPECT_EQ(test_CLAMP_uint((unsigned int) 8, (unsigned int) 7, (unsigned int) 10), (unsigned int) 8);
}

TEST(ClampTest, ClampsLong) {
    EXPECT_EQ(test_CLAMP_long((long) -4, (long) 1, (long) 3), (long) 1);
    EXPECT_EQ(test_CLAMP_long((long) 7, (long) 1, (long) 3), (long) 3);
    EXPECT_EQ(test_CLAMP_long((long) 2, (long) 1, (long) 3), (long) 2);
}

TEST(ClampTest, ClampsUnsignedLong) {
    EXPECT_EQ(test_CLAMP_ulong((unsigned long) 4, (unsigned long) 7, (unsigned long) 10), (unsigned long) 7);
    EXPECT_EQ(test_CLAMP_ulong((unsigned long) 14, (unsigned long) 7, (unsigned long) 10), (unsigned long) 10);
    EXPECT_EQ(test_CLAMP_ulong((unsigned long) 4, (unsigned long) 8, (unsigned long) 10), (unsigned long) 8);
}

TEST(ClampTest, ClampsFloat) {
    EXPECT_FLOAT_EQ(test_CLAMP_float(-4.0, 1.0, 3.0), 1.0);
    EXPECT_FLOAT_EQ(test_CLAMP_float(7.0, 1.0, 3.0), 3.0);
    EXPECT_FLOAT_EQ(test_CLAMP_float(2.0, 1.0, 3.0), 2.0);
}