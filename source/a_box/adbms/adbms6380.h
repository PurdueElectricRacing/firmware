#ifndef _ADBMS6380_H_
#define _ADBMS6380_H_

#include <stddef.h>
#include <stdint.h>

#include "common/phal/spi.h"
#include "common/strbuf/strbuf.h"

#define ADBMS6380_CELL_COUNT           (16)

#define ADBMS6380_COMMAND_RAW_SIZE     (2)                                 // 2 bytes for command
#define ADBMS6380_COMMAND_PKT_SIZE     (ADBMS6380_COMMAND_RAW_SIZE + 2)     // 2 extra for PEC
#define ADBMS6380_SINGLE_DATA_RAW_SIZE (6)                                  // 6 data bytes
#define ADBMS6380_SINGLE_DATA_PKT_SIZE (ADBMS6380_SINGLE_DATA_RAW_SIZE + 2) // 2 extra for PEC

#define ADBMS6380_WAKE_DELAY_MS (1) // 1 millisecond. Must be a multiple of 100 microseconds


void adbms6380_set_cs_low(SPI_InitConfig_t* spi);
void adbms6380_set_cs_high(SPI_InitConfig_t* spi);

void adbms6380_wake(SPI_InitConfig_t* spi, size_t module_count);

uint16_t adbms6380_get_threshold_voltage_cfg(float threshold_voltage);
int16_t adbms6380_extract_i16(uint8_t* data, int idx);
float adbms6380_raw_to_cell_v(int16_t raw);


void adbms6830_adcv(uint8_t* output_cmd, uint8_t rd, uint8_t cont, uint8_t dcp, uint8_t rstf, uint8_t ow);
void adbms6830_adsv(uint8_t* output_cmd, uint8_t cont, uint8_t dcp, uint8_t ow);

void adbms6380_prepare_command(strbuf_t* output_buffer, const uint8_t command[ADBMS6380_COMMAND_RAW_SIZE]);
void adbms6380_prepare_data_packet(strbuf_t* output_buffer, const uint8_t data[ADBMS6380_SINGLE_DATA_RAW_SIZE]);

/**
 * @brief Calculate the REG_A configuration register value
 * @param output_cfg_rega Pointer to output buffer for REG_A configuration (6 bytes)
 * @param refon 1 = reference remains powered up until watchdog timeout
 * @param cth C-ADC vs. S-ADC comparison voltage threshold (0b110 = 25.05 mV)
 */
void adbms6380_calculate_cfg_rega(uint8_t output_cfg_rega[ADBMS6380_SINGLE_DATA_RAW_SIZE], bool refon, uint8_t cth);
void adbms6380_calculate_cfg_regb(
	uint8_t output_cfg_regb[ADBMS6380_SINGLE_DATA_RAW_SIZE],
	float overvoltage_threshold,
	float undervoltage_threshold,
	const bool* is_discharging
);

bool adbms6380_read(SPI_InitConfig_t* spi, size_t module_count, const uint8_t cmd_buffer[ADBMS6380_COMMAND_PKT_SIZE], uint8_t* rx_buffer);
void adbms6380_read_cell_voltages(const uint8_t* rx_buffer, float* cell_voltages);
void adbms6380_read_therms(const uint8_t* rx_buffer, float* thermistor_temps, size_t therm_count);

// Other adbms6380 related function declarations can go here

#endif // _ADBMS6380_H_