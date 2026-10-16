#include <gtest/gtest.h>

extern "C" {
#include "clamp_test_shim.h"
}

TEST(ClampTest, ClampsInt) {
    EXPECT_EQ(test_CLAMP_i(-4, 1, 3), 1);
    EXPECT_EQ(test_CLAMP_i(7, 1, 3), 3);
    EXPECT_EQ(test_CLAMP_i(2, 1, 3), 2);
}

TEST(ClampTest, ClampsUnsignedInt) {
    EXPECT_EQ(test_CLAMP_ui((unsigned int) 4, (unsigned int) 7, (unsigned int) 10), (unsigned int) 7);
    EXPECT_EQ(test_CLAMP_ui((unsigned int) 14, (unsigned int) 7, (unsigned int) 10), (unsigned int) 10);
    EXPECT_EQ(test_CLAMP_ui((unsigned int) 8, (unsigned int) 7, (unsigned int) 10), (unsigned int) 8);
}

TEST(ClampTest, ClampsLong) {
    EXPECT_EQ(test_CLAMP_l((long) -4, (long) 1, (long) 3), (long) 1);
    EXPECT_EQ(test_CLAMP_l((long) 7, (long) 1, (long) 3), (long) 3);
    EXPECT_EQ(test_CLAMP_l((long) 2, (long) 1, (long) 3), (long) 2);
}

TEST(ClampTest, ClampsUnsignedLong) {
    EXPECT_EQ(test_CLAMP_ul((unsigned long) 4, (unsigned long) 7, (unsigned long) 10), (unsigned long) 7);
    EXPECT_EQ(test_CLAMP_ul((unsigned long) 14, (unsigned long) 7, (unsigned long) 10), (unsigned long) 10);
    EXPECT_EQ(test_CLAMP_ul((unsigned long) 4, (unsigned long) 8, (unsigned long) 10), (unsigned long) 8);
}

TEST(ClampTest, ClampsFloat) {
    EXPECT_FLOAT_EQ(test_CLAMP_f(-4.0, 1.0, 3.0), 1.0);
    EXPECT_FLOAT_EQ(test_CLAMP_f(7.0, 1.0, 3.0), 3.0);
    EXPECT_FLOAT_EQ(test_CLAMP_f(2.0, 1.0, 3.0), 2.0);
}