#ifndef _TEMP_H_
#define _TEMP_H_

// Includes
#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include "common/phal_L4/i2c/i2c.h"
#include "common_defs.h"
#include "bms.h"
#include "math.h"

// Defines
#define B_VALUE                 3977            // Beta value 25*C to 85*C temp range
#define ADC_RES                 0xFFF           // ADC resolution
#define THERM_RESIST            10000           // 10K Ohm resistor
#define VOLTAGE_REF             3.3             // Refernce voltage into temp MCU
#define AMBIENT_TEMP            298.15          // Degrees C
#define VOLTAGE_TOP             3.3             // Top of the voltage divider
#define R_INF_3977              0.016106        // r_inf = R0 * exp(-Beta/t0) beta = 3977
#define KELVIN_2_CELSIUS        273.15          // Conversion constant
#define DT_CRIT                 50              // Max cell temp increase (in C) per 15ms (hope to spot battery fires)

// Prototypes
int checkTempMaster(void);
void procTemps(void);

#endif