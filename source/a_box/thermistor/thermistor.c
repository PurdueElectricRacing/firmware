/**
 * @file thermistor.c
 * @brief Thermistor lookup table from datasheet.
 *
 * @author Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
 */

#include "common/lerp_lut/lerp_lut.h"

// https://www.tdk-electronics.tdk.com/inf/50/db/ntc/NTC_Mini_sensors_S861.pdf
// from pg 6: R/T No. 8016
static constexpr size_t B57861S0103A039_LUT_SIZE = 43;
const lut_entry_t B57861S0103A039_data[B57861S0103A039_LUT_SIZE] = {
    {.key = -55, .value = 96.3 },
    {.key = -50, .value = 67.01 },
    {.key = -45, .value = 47.17 },
    {.key = -40, .value = 33.65 },
    {.key = -35, .value = 24.26 },
    {.key = -30, .value = 17.7 },
    {.key = -25, .value = 13.04 },
    {.key = -20, .value = 9.707 },
    {.key = -15, .value = 7.293 },
    {.key = -10, .value = 5.533 },
    {.key = -5, .value = 4.232 },
    {.key = 0, .value = 3.265 },
    {.key = 5, .value = 2.539 },
    {.key = 10, .value = 1.99 },
    {.key = 15, .value = 1.571 },
    {.key = 20, .value = 1.249 },
    {.key = 25, .value = 1.0000 },
    {.key = 30, .value = 0.8057 },
    {.key = 35, .value = 0.6531 },
    {.key = 40, .value = 0.5327 },
    {.key = 45, .value = 0.4369 },
    {.key = 50, .value = 0.3603 },
    {.key = 55, .value = 0.2986 },
    {.key = 60, .value = 0.2488 },
    {.key = 65, .value = 0.2083 },
    {.key = 70, .value = 0.1752 },
    {.key = 75, .value = 0.1481 },
    {.key = 80, .value = 0.1258 },
    {.key = 85, .value = 0.1072 },
    {.key = 90, .value = 0.09177 },
    {.key = 95, .value = 0.07885 },
    {.key = 100, .value = 0.068 },
    {.key = 105, .value = 0.05886 },
    {.key = 110, .value = 0.05112 },
    {.key = 115, .value = 0.04454 },
    {.key = 120, .value = 0.03893 },
    {.key = 125, .value = 0.03417 },
    {.key = 130, .value = 0.03009 },
    {.key = 135, .value = 0.02654 },
    {.key = 140, .value = 0.02348 },
    {.key = 145, .value = 0.02083 },
    {.key = 150, .value = 0.01853 },
    {.key = 155, .value = 0.01653 }
};

LERP_LUT_INIT(thermistor_lut, B57861S0103A039_data, B57861S0103A039_LUT_SIZE);