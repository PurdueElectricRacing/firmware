#include "tmu.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "main.h"
#include "common/common_defs/common_defs.h"
#include "can_parse.h"
#include "common/faults/faults.h"
#include <string.h>
#include "temp_conversion.h"



uint8_t num_bad1, num_bad2, num_bad3, num_bad4;
bool overtemp = false;
bool tmu_daq_override = false;
uint8_t tmu_daq_therm = 0;

tmu_handle_t tmu;


static bool init_avg_fill;

void initTMU() {
  memset(&tmu, 0, sizeof(tmu));

  // Reset max and min temp readings per plate
  for (uint8_t module = 0; module < NUM_MODULES; module++)
  {
    // Grab the left plate and right plate addresses
    tmu_info_t *left_readings = &tmu.module_temps[module].left_readings;
    tmu_info_t *right_readings = &tmu.module_temps[module].right_readings;

    left_readings->min_temp = ERROR_HIGH;
    left_readings->max_temp = ERROR_LOW;

    right_readings->min_temp = ERROR_HIGH;
    right_readings->max_temp = ERROR_LOW;
  }
  init_avg_fill = true;
  PHAL_writeGPIO(MUX_A_Port, MUX_A_Pin, 0);
  PHAL_writeGPIO(MUX_B_Port, MUX_B_Pin, 0);
  PHAL_writeGPIO(MUX_C_Port, MUX_C_Pin, 0);
  PHAL_writeGPIO(MUX_D_Port, MUX_D_Pin, 0);
}


uint8_t readTemps()
{

  static int curr_therm; // current thermistor counter variable

  for (uint8_t module = 0; module < NUM_MODULES; module++)
  {
    // Grab the left plate and right plate addresses
    tmu_info_t *left_readings = &tmu.module_temps[module].left_readings;
    tmu_info_t *right_readings = &tmu.module_temps[module].right_readings;

    /* Storing ADC readings for current module's left and right plate */
    uint16_t left_raw_adc_reading = *(&adc_readings.tmu_1_1 + module);
    uint16_t right_raw_adc_reading = *(&adc_readings.tmu_1_1 + (module + 1));

    //Subtract old value at this index from average
    if (!init_avg_fill)
    {
      left_readings->avg_temp -= left_readings->temp_readings[curr_therm];
      right_readings->avg_temp -= right_readings->temp_readings[curr_therm];
    }
    else
    {
        init_avg_fill = false;
    }

    left_readings->temp_readings[curr_therm] = left_raw_adc_reading > ADC_ERROR_HIGH ? ERROR_HIGH : left_raw_adc_reading < ADC_ERROR_LOW ? ERROR_LOW : ADC_to_temp[left_raw_adc_reading - ADC_ERROR_LOW];
    right_readings->temp_readings[curr_therm] = right_raw_adc_reading > ADC_ERROR_HIGH ? ERROR_HIGH : right_raw_adc_reading < ADC_ERROR_LOW ? ERROR_LOW : ADC_to_temp[right_raw_adc_reading - ADC_ERROR_LOW];

    if (left_raw_adc_reading > ADC_ERROR_HIGH || left_raw_adc_reading < ADC_ERROR_LOW)
    {
      left_readings->num_bad++;
    }
    if (right_raw_adc_reading > ADC_ERROR_HIGH || right_raw_adc_reading < ADC_ERROR_LOW)
    {
      right_readings->num_bad++;
    }

    // Populate max temp of current run
    left_readings->max_temp = MAX(left_readings->max_temp, left_readings->temp_readings[curr_therm]);
    right_readings->max_temp = MAX(right_readings->max_temp, right_readings->temp_readings[curr_therm]);

    left_readings->min_temp = MIN(left_readings->min_temp, left_readings->temp_readings[curr_therm]);
    right_readings->min_temp = MIN(right_readings->min_temp, right_readings->temp_readings[curr_therm]);

    left_readings->avg_temp += left_readings->temp_readings[curr_therm];
    right_readings->avg_temp += right_readings->temp_readings[curr_therm];
  }
  uint8_t tempError = 0;

  module_temp_info_t *module_one = &tmu.module_temps[TMU_MODULE_ONE];
  module_temp_info_t *module_two = &tmu.module_temps[TMU_MODULE_TWO];
  module_temp_info_t *module_three = &tmu.module_temps[TMU_MODULE_THREE];
  module_temp_info_t *module_four = &tmu.module_temps[TMU_MODULE_FOUR];
  module_temp_info_t *module_five = &tmu.module_temps[TMU_MODULE_FIVE];

  // send temperatures over CAN (sent multiplied by 10, so 221 would be 22.1 deg C)
  SEND_RAW_CELL_TEMP_MODULE1(curr_therm, module_one->left_readings.temp_readings[curr_therm], module_one->left_readings.temp_readings[curr_therm]);
  SEND_RAW_CELL_TEMP_MODULE2(curr_therm, module_two->left_readings.temp_readings[curr_therm], module_two->left_readings.temp_readings[curr_therm]);
  SEND_RAW_CELL_TEMP_MODULE3(curr_therm, module_three->left_readings.temp_readings[curr_therm], module_three->left_readings.temp_readings[curr_therm]);
  SEND_RAW_CELL_TEMP_MODULE4(curr_therm, module_four->left_readings.temp_readings[curr_therm], module_four->left_readings.temp_readings[curr_therm]);
  SEND_RAW_CELL_TEMP_MODULE5(curr_therm, module_five->left_readings.temp_readings[curr_therm], module_five->left_readings.temp_readings[curr_therm]);

  if (curr_therm < NUM_THERM ) {
      curr_therm++;
  }
  else
  {
      // Start by populating statistics across a module
      for (uint8_t module = 0; module < NUM_MODULES; module++)
      {
        // Grab the left plate and right plate addresses
        tmu_info_t *left_readings = &tmu.module_temps[module].left_readings;
        tmu_info_t *right_readings = &tmu.module_temps[module].right_readings;

        tmu.module_temps[module].total_max_temp = MAX(left_readings->max_temp, right_readings->min_temp);
        tmu.module_temps[module].total_min_temp = MIN(left_readings->min_temp, right_readings->min_temp);
        tmu.module_temps[module].total_avg_temp = (left_readings->avg_temp + right_readings->avg_temp) / THERM_PER_MODULE;
      }

      module_temp_info_t *module_one = &tmu.module_temps[TMU_MODULE_ONE];
      module_temp_info_t *module_two = &tmu.module_temps[TMU_MODULE_TWO];
      module_temp_info_t *module_three = &tmu.module_temps[TMU_MODULE_THREE];
      module_temp_info_t *module_four = &tmu.module_temps[TMU_MODULE_FOUR];
      module_temp_info_t *module_five = &tmu.module_temps[TMU_MODULE_FIVE];

      // finished incrementing, sending max and min values, sending averages, setting faults
      SEND_MOD_CELL_TEMP_AVG_A_B_C(module_one->total_avg_temp, module_two->total_avg_temp, module_three->total_avg_temp);
      SEND_MOD_CELL_TEMP_AVG_D_E(module_four->total_avg_temp, module_five->total_avg_temp);
      SEND_MOD_CELL_TEMP_MAX_A_B_C(module_one->total_max_temp, module_two->total_max_temp, module_three->total_max_temp);
      SEND_MOD_CELL_TEMP_MAX_D_E(module_four->total_max_temp, module_five->total_max_temp);
      SEND_MOD_CELL_TEMP_MIN_A_B_C(module_one->total_min_temp, module_two->total_min_temp, module_three->total_min_temp);
      SEND_MOD_CELL_TEMP_MIN_D_E(module_four->total_min_temp, module_five->total_min_temp);
      int16_t max_temp = MAX(MAX(MAX(module_one->total_max_temp, module_two->total_max_temp), MAX(module_three->total_max_temp, module_four->total_max_temp)), module_five->total_max_temp);
      int16_t min_temp =  MIN(MIN(MIN(module_one->total_max_temp, module_two->total_max_temp), MAX(module_three->total_max_temp, module_four->total_max_temp)), module_five->total_max_temp);
      SEND_MAX_CELL_TEMP(max_temp);
      SEND_NUM_THERM_BAD(module_one->left_readings.num_bad, module_one->left_readings.num_bad,
                           module_two->left_readings.num_bad, module_two->left_readings.num_bad,
                           module_three->left_readings.num_bad, module_three->left_readings.num_bad,
                           module_four->left_readings.num_bad, module_four->left_readings.num_bad,
                           module_five->left_readings.num_bad, module_five->left_readings.num_bad);
      setFault(ID_PACK_TEMP_FAULT, max_temp);
      setFault(ID_PACK_TEMP_EXCEEDED_FAULT, max_temp);
      setFault(ID_MIN_PACK_TEMP_FAULT, min_temp);

      // resetting
      curr_therm = 0;

      // Reset num bad readings
      for (uint8_t module = 0; module < NUM_MODULES; module++)
      {
        // Grab the left plate and right plate addresses
        tmu_info_t *left_readings = &tmu.module_temps[module].left_readings;
        tmu_info_t *right_readings = &tmu.module_temps[module].right_readings;

        left_readings->num_bad = 0;
        right_readings->num_bad = 0;

        left_readings->min_temp = ERROR_HIGH;
        left_readings->max_temp = ERROR_LOW;

        right_readings->min_temp = ERROR_HIGH;
        right_readings->max_temp = ERROR_LOW;
      }
  }

    // allowing DAQ override if necessary
    uint8_t therm = curr_therm;
    if (tmu_daq_override) therm = tmu_daq_therm;
    //Select the MUX pin on each TMU board to read the thermistor value
    PHAL_writeGPIO(MUX_A_Port, MUX_A_Pin, (therm & 0x1));
    PHAL_writeGPIO(MUX_B_Port, MUX_B_Pin, (therm & 0x2));
    PHAL_writeGPIO(MUX_C_Port, MUX_C_Pin, (therm & 0x4));
    PHAL_writeGPIO(MUX_D_Port, MUX_D_Pin, (therm & 0x8));

    // checking if faults have latched
    return checkFault(ID_PACK_TEMP_EXCEEDED_FAULT) |
                checkFault(ID_MIN_PACK_TEMP_FAULT);
}
