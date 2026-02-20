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
    {.key = -55.0f, .value = 96.3f },
    {.key = -50.0f, .value = 67.01f },
    {.key = -45.0f, .value = 47.17f },
    {.key = -40.0f, .value = 33.65f },
    {.key = -35.0f, .value = 24.26f },
    {.key = -30.0f, .value = 17.7f },
    {.key = -25.0f, .value = 13.04f },
    {.key = -0.20f, .value = 9.707f },
    {.key = -0.15f, .value = 7.293f },
    {.key = -0.10f, .value = 5.533f },
    {.key = -0.5f, .value = 4.232f },
    {.key = 0.0f, .value = 3.265f },
    {.key = 5.0f, .value = 2.539f },
    {.key = 10.0f, .value = 1.99f },
    {.key = 15.0f, .value = 1.571f },
    {.key = 20.0f, .value = 1.249f },
    {.key = 25.0f, .value = 1.0000f },
    {.key = 30.0f, .value = 0.8057f },
    {.key = 35.0f, .value = 0.6531f },
    {.key = 40.0f, .value = 0.5327f },
    {.key = 45.0f, .value = 0.4369f },
    {.key = 50.0f, .value = 0.3603f },
    {.key = 55.0f, .value = 0.2986f },
    {.key = 60.0f, .value = 0.2488f },
    {.key = 65.0f, .value = 0.2083f },
    {.key = 70.0f, .value = 0.1752f },
    {.key = 75.0f, .value = 0.1481f },
    {.key = 80.0f, .value = 0.1258f },
    {.key = 85.0f, .value = 0.1072f },
    {.key = 90.0f, .value = 0.09177f },
    {.key = 95.0f, .value = 0.07885f },
    {.key = 100.0f, .value = 0.068f },
    {.key = 105.0f, .value = 0.05886f },
    {.key = 110.0f, .value = 0.05112f },
    {.key = 115.0f, .value = 0.04454f },
    {.key = 120.0f, .value = 0.03893f },
    {.key = 125.0f, .value = 0.03417f },
    {.key = 130.0f, .value = 0.03009f },
    {.key = 135.0f, .value = 0.02654f },
    {.key = 140.0f, .value = 0.02348f },
    {.key = 145.0f, .value = 0.02083f },
    {.key = 150.0f, .value = 0.01853f },
    {.key = 155.0f, .value = 0.01653f }
};

LERP_LUT_INIT(thermistor_lut, B57861S0103A039_data, B57861S0103A039_LUT_SIZE);
