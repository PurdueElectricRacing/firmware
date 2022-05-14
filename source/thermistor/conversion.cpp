#include <math.h>

// Hardware constants
#define R_REF   10000.
#define ADC_REF 3.3
#define VCC     5.
#define ADC_RES 1024
#define R25     10000.

// Parameters of the thermocouple:
// R_rel = R_t/R_25
// 1/T = A + B * ln(R_rel) + C * ln(R_rel) ^ 2 + D * ln(R_rel) ^ 3
// Low temperatures:
#define A_LOW   0.003354016
#define B_LOW   0.000256173
#define C_LOW   2.13941e-06
#define D_LOW   -7.25325e-08
// High temperatures
#define A_HIGH   0.003353045
#define B_HIGH   0.000254200
#define C_HIGH   1.14261e-06
#define D_HIGH   -6.93803e-08




float temp_calc (float a, float b, float c, float d, float r_rel) {

    float sum = a +
                b * log(r_rel) +
                c * log(r_rel) * log(r_rel) +
                d * log(r_rel) * log(r_rel) * log(r_rel);
    
    return 1 / sum;

}

float temp (int adc_meas) {

    float v_out = adc_meas * ADC_REF / ADC_RES;

    float r_rel = v_out * R_REF / (VCC - v_out) / R25;

    if (r_rel > 0.3603 && r_rel <= 3.265) {
        return temp_calc(A_LOW, B_LOW, B_LOW, B_LOW, r_rel);
    } else if (r_rel > 0.6748 && r_rel <= 0.3603) {
        return temp_calc(A_HIGH, B_HIGH, B_HIGH, B_HIGH, r_rel);
    } else {
        return 0; // We're fucked
    }
}



