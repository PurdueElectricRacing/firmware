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
// Resistance -> temperature (Celsius)
const lut_entry_t B57861S0103A039_data[B57861S0103A039_LUT_SIZE] = {
    {.key = 0.01653f, .value = 155.0f },
    {.key = 0.01853f, .value = 150.0f },
    {.key = 0.02083f, .value = 145.0f },
    {.key = 0.02348f, .value = 140.0f },
    {.key = 0.02654f, .value = 135.0f },
    {.key = 0.03009f, .value = 130.0f },
    {.key = 0.03417f, .value = 125.0f },
    {.key = 0.03893f, .value = 120.0f },
    {.key = 0.04454f, .value = 115.0f },
    {.key = 0.05112f, .value = 110.0f },
    {.key = 0.05886f, .value = 105.0f },
    {.key = 0.068f, .value = 100.0f },
    {.key = 0.07885f, .value = 95.0f },
    {.key = 0.09177f, .value = 90.0f },
    {.key = 0.1072f, .value = 85.0f },
    {.key = 0.1258f, .value = 80.0f },
    {.key = 0.1481f, .value = 75.0f },
    {.key = 0.1752f, .value = 70.0f },
    {.key = 0.2083f, .value = 65.0f },
    {.key = 0.2488f, .value = 60.0f },
    {.key = 0.2986f, .value = 55.0f },
    {.key = 0.3603f, .value = 50.0f },
    {.key = 0.4369f, .value = 45.0f },
    {.key = 0.5327f, .value = 40.0f },
    {.key = 0.6531f, .value = 35.0f },
    {.key = 0.8057f, .value = 30.0f },
    {.key = 1.0000f, .value = 25.0f },
    {.key = 1.249f, .value = 20.0f },
    {.key = 1.571f, .value = 15.0f },
    {.key = 1.99f, .value = 10.0f },
    {.key = 2.539f, .value = 5.0f },
    {.key = 3.265f, .value = 0.0f },
    {.key = 4.232f, .value = -5.0f },
    {.key = 5.533f, .value = -10.0f },
    {.key = 7.293f, .value = -15.0f },
    {.key = 9.707f, .value = -20.0f },
    {.key = 13.04f, .value = -25.0f },
    {.key = 17.7f, .value = -30.0f },
    {.key = 24.26f, .value = -35.0f },
    {.key = 33.65f, .value = -40.0f },
    {.key = 47.17f, .value = -45.0f },
    {.key = 67.01f, .value = -50.0f },
    {.key = 96.3f, .value = -55.0f }
};

LERP_LUT_INIT(thermistor_lut, B57861S0103A039_data, B57861S0103A039_LUT_SIZE);
