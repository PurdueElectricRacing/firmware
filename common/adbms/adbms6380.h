#ifndef _ADBMS6380_H_
#define _ADBMS6380_H_

#include <stddef.h>
#include <stdint.h>

#include "common/phal/spi.h"

#define ADBMS6380_COMMAND_RAW_SIZE     (2)                                 // 2 bytes for command
#define ADBMS6380_COMMAND_PKT_SIZE     (ADBMS6380_COMMAND_RAW_SIZE + 2)     // 2 extra for PEC
#define ADBMS6380_SINGLE_DATA_RAW_SIZE (6)                                  // 6 data bytes
#define ADBMS6380_SINGLE_DATA_PKT_SIZE (ADBMS6380_SINGLE_DATA_RAW_SIZE + 2) // 2 extra for PEC

void adbms6380_set_cs_low(SPI_InitConfig_t *spi);
void adbms6380_set_cs_high(SPI_InitConfig_t *spi);

void adbms6380_wake(SPI_InitConfig_t *spi, size_t module_count);

uint16_t adbms6380_get_threshold_voltage_cfg(float threshold_voltage);
int16_t adbms6380_extract_i16(uint8_t *data, int idx);
float adbms6380_raw_to_cell_v(int16_t raw);

void adbms6380_prepare_command(uint8_t *buffer, const uint8_t command[ADBMS6380_COMMAND_RAW_SIZE]);
void adbms6380_prepare_data_packet(uint8_t *buffer, const uint8_t data[ADBMS6380_SINGLE_DATA_RAW_SIZE]);
void adbms6380_calculate_cfg_rega(uint8_t *cfg_rega, bool todo_fill_in); //! TODO: fill in parameters
void adbms6380_calculate_cfg_regb(uint8_t *cfg_regb, bool todo_fill_in); //! TODO: fill in parameters
void adbms6380_read_cell_voltages(const uint8_t *rx_buffer, float *cell_voltages, size_t cell_count);


// Other adbms6380 related function declarations can go here

#endif // _ADBMS6380_H_