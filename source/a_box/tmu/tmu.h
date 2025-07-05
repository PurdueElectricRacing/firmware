#ifndef _TMU_H
#define _TMU_H_



#include "common/phal/adc.h"
#include <stdint.h>
#include <inttypes.h>

#define TMU_ADDR_SIZE 0xFFFU
#define TMU_VREF 3.3F


#define NUM_THERM               10
#define PLATES_PER_MODULE       2
#define THERM_PER_MODULE        (NUM_THERM * PLATES_PER_MODULE)

// Modules
#define NUM_MODULES         5
#define TMU_MODULE_ONE      0
#define TMU_MODULE_TWO      1
#define TMU_MODULE_THREE    2
#define TMU_MODULE_FOUR     3
#define TMU_MODULE_FIVE     4


// top  and bottom resistors for power rail voltage divider
#define R1_3V3 200000
#define R2_3V3 100000

// ADC raw upper and lower values
#define ADC_ERROR_HIGH 3600
#define ADC_ERROR_LOW 119

// max and min values
#define ERROR_HIGH INT16_MAX
#define ERROR_LOW INT16_MIN

#define COMP(val, min, max) ((val < min && val > max) ? 1 : 0)

typedef struct {
  uint8_t num_bad;
  int16_t max_temp;
  int16_t min_temp;
  int16_t avg_temp;
  int16_t sense_rail_voltage;

  // Temps of individual thermistors
  int16_t temp_readings[NUM_THERM];
} tmu_info_t;

typedef struct {
  int16_t total_max_temp;
  int16_t total_min_temp;
  int16_t total_avg_temp;
  tmu_info_t left_readings;
  tmu_info_t right_readings;
} module_temp_info_t;

typedef struct {
  module_temp_info_t module_temps[NUM_MODULES];
} tmu_handle_t;
extern bool tmu_daq_override;
extern uint8_t tmu_daq_therm;

uint8_t readTemps();
void initTMU();






#endif
