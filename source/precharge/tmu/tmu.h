#ifndef _TMU_H
#define _TMU_H_


#include "common/phal_L4/spi/spi.h"
#include <stdint.h>
#include <inttypes.h>


#define TMU_FILTERED_DATA_CMD (0x5U)
#define TMU_PRODID_ADDR (0x0U)


#define TMU_ADDR_SIZE 0xFFFU
#define TMU_VREF 1.8F

//Changing this value will change how many thermistor slots are read.
#define NUM_THERM 10




#define TMU_VIN 5.0F
//Top resistor of voltage divider
#define R1 68000
//Based on datasheet p. 69- https://www.amphenol-sensors.com/hubfs/Documents/AAS-913-318C-Temperature-resistance-curves-071816-web.pdf
#define R25 10000
#define LOW_RANGE_MIN 3.277F
#define LOW_RANGE_MAX 0.3599F
#define LOW_RANGE_A 0.003354016F
#define LOW_RANGE_B 0.000256173F
#define LOW_RANGE_C 2.13941E-6
#define LOW_RANGE_D -7.25325E-8
#define HIGH_RANGE_MIN 0.3598F //Ensure that 0.3599 doesn't get cut off from range
#define HIGH_RANGE_MAX 0.06816F
#define HIGH_RANGE_A 0.003353045F
#define HIGH_RANGE_B 0.0002542F
#define HIGH_RANGE_C 1.14261E-6
#define HIGH_RANGE_D -6.93803E-8


#define COMP(val, min, max) ((val < min && val > max) ? 1 : 0)




typedef struct {
   SPI_InitConfig_t *spi;
   float tmu1_volts;
   float tmu2_volts;
   float tmu3_volts;
   float tmu4_volts;
   uint16_t tmu1_max;
   uint16_t tmu2_max;
   uint16_t tmu3_max;
   uint16_t tmu4_max;
   uint16_t tmu1_min;
   uint16_t tmu2_min;
   uint16_t tmu3_min;
   uint16_t tmu4_min;
   uint16_t tmu1_avg;
   uint16_t tmu2_avg;
   uint16_t tmu3_avg;
   uint16_t tmu4_avg;
   uint16_t tmu1[15];
   uint16_t tmu2[15];
   uint16_t tmu3[15];
   uint16_t tmu4[15];
} tmu_handle_t;
void readTemps(tmu_handle_t *tmu);
bool initTMU(tmu_handle_t *tmu);






#endif
