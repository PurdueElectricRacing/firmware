/**
 * @file adbms6380.h
 * @brief Low level BMS driver specific to ADBMS6380 chip.
 *
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "adbms6380.h"

#include <stdbool.h>

#include "common/phal/gpio.h"
#include "common/phal/spi.h"
#include "common/psched/psched.h"

#include "commands.h"
#include "pec.h"


void adbms6380_set_cs_low(SPI_InitConfig_t* spi) {
	PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, false);
}

void adbms6380_set_cs_high(SPI_InitConfig_t* spi) {
	PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, true);
}


void adbms6380_wake(SPI_InitConfig_t* spi, size_t module_count) {
	//! TODO: this is really janky + should not be a full blocking wait?

	size_t sleep_pulses = (ADBMS6380_WAKE_DELAY_MS * 1000) / 100; // Number of 100us pulses needed
	for (size_t i = 0; i < module_count; i++) {
		adbms6380_set_cs_low(spi);
		for (size_t j = 0; j < sleep_pulses; j++) waitMicros(100);
		adbms6380_set_cs_high(spi);
		for (size_t j = 0; j < sleep_pulses; j++) waitMicros(100);
	}
}


uint16_t adbms6380_get_threshold_voltage_cfg(float threshold_voltage) {
    uint8_t rbits = 12;
    threshold_voltage = (threshold_voltage - 1.5);
    threshold_voltage = threshold_voltage / (16 * 0.000150);
    uint16_t cfg = (uint16_t)(threshold_voltage + 2 * (1 << (rbits - 1)));
    cfg &= 0xFFF;
    return cfg;
}

int16_t adbms6380_extract_i16(uint8_t* data, int idx) {
	return (int16_t)(data[idx * 2 + 0] & 0xff) | ((int16_t)(data[idx * 2 + 1] & 0xff) << 8);
}

float adbms6380_raw_to_cell_v(int16_t raw) {
	return (raw + 10000) * 0.000150f;
}

float adbms6380_raw_to_gpio_v(int16_t raw) {
	return raw * 0.0001f;
}

void adbms6380_adcv(uint8_t output_cmd[ADBMS6380_COMMAND_RAW_SIZE], bool rd, bool cont, bool dcp, bool rstf, uint8_t ow) {
	// 10-0: 0, 1, RD, CONT, 1, 1, DCP, 0, RSTF, OW[1], OW[0]
	output_cmd[0] = 0x02 + (uint8_t)rd;
    output_cmd[1] = ((uint8_t)cont << 7) + ((uint8_t)dcp << 4) + ((uint8_t)rstf << 2) + (ow & 0x03) + 0x60;
}

void adbms6380_adsv(uint8_t output_cmd[ADBMS6380_COMMAND_RAW_SIZE], bool cont, bool dcp, uint8_t ow) {
	// 10-0: 0, 0, 1, CONT, 1, 1, DCP, 1, 0, OW[1], OW[0]
	output_cmd[0] = 0x01;
	output_cmd[1] = ((uint8_t)cont << 7) + ((uint8_t)dcp << 4) + (ow & 0x03) + 0x68;
}


void adbms6380_prepare_command(strbuf_t* output_buffer, const uint8_t command[ADBMS6380_COMMAND_RAW_SIZE]) {
	strbuf_append(output_buffer, command, ADBMS6380_COMMAND_RAW_SIZE);
	uint16_t pec = adbms_pec_get_pec15(ADBMS6380_COMMAND_RAW_SIZE, command);
	uint8_t pec_bytes[2] = { (uint8_t)((pec >> 8) & 0xFF), (uint8_t)(pec & 0xFF) };
	strbuf_append(output_buffer, pec_bytes, 2);
}

void adbms6380_prepare_data_packet(strbuf_t* output_buffer, const uint8_t data[ADBMS6380_SINGLE_DATA_RAW_SIZE]) {
	strbuf_append(output_buffer, data, ADBMS6380_SINGLE_DATA_RAW_SIZE);
	uint16_t pec = adbms_pec_get_pec10(false, ADBMS6380_SINGLE_DATA_RAW_SIZE, data);
	uint8_t pec_bytes[2] = { (uint8_t)((pec >> 8) & 0xFF), (uint8_t)(pec & 0xFF) };
	strbuf_append(output_buffer, pec_bytes, 2);
}


void adbms6380_calculate_cfg_rega(uint8_t output_cfg_rega[ADBMS6380_SINGLE_DATA_RAW_SIZE], bool refon, uint8_t cth) {
	output_cfg_rega[0] = (refon << 7) | (cth & 0x07);
	// all flags 0
	output_cfg_rega[1] = 0b00000000;
	// no soak
	output_cfg_rega[2] = 0;
	
    // All GPIO pull down off
	output_cfg_rega[3] = 0b11111111; // GPIOs [8:0] pull down OFF
	output_cfg_rega[4] = 0b00000011; // GPIOs [10:9] pull down OFF

	// No SNAP_ST MUTE_ST COMM_BK FC
    output_cfg_rega[5] = 0;
}

void adbms6380_calculate_cfg_regb(
	uint8_t output_cfg_regb[ADBMS6380_SINGLE_DATA_RAW_SIZE],
	float overvoltage_threshold,
	float undervoltage_threshold,
	const bool is_discharging[ADBMS6380_CELL_COUNT]
) {
	uint16_t overvoltage_cfg = adbms6380_get_threshold_voltage_cfg(overvoltage_threshold);
    uint16_t undervoltage_cfg = adbms6380_get_threshold_voltage_cfg(undervoltage_threshold);
    // 12 bits vov
    output_cfg_regb[0] = overvoltage_cfg & 0xFF;
    output_cfg_regb[1] |= (overvoltage_cfg >> 8) & 0xF;
    // 12 bits vuv
    output_cfg_regb[1] |= (undervoltage_cfg << 4) & 0xF0;
    output_cfg_regb[2] |= (undervoltage_cfg >> 4) & 0xFF;
    // all else default
    output_cfg_regb[3] = 0;
	
	// DCC
    output_cfg_regb[4] = 0;
    output_cfg_regb[5] = 0;
	for (size_t i = 0; i < ADBMS6380_CELL_COUNT; i++) {
		if (is_discharging[i]) {
			size_t byte_idx = 4 + (i / 8);
			size_t bit_idx = i % 8;
			output_cfg_regb[byte_idx] |= (1 << bit_idx);
		}
	}
}


bool adbms6380_read_data(SPI_InitConfig_t* spi, size_t module_count, const uint8_t cmd_buffer[ADBMS6380_COMMAND_PKT_SIZE], uint8_t* rx_buffer) {
	size_t rx_length = module_count * ADBMS6380_SINGLE_DATA_PKT_SIZE;
	return adbms6380_read(spi, module_count, cmd_buffer, rx_buffer, rx_length);
}

bool adbms6380_read(SPI_InitConfig_t* spi, size_t module_count, const uint8_t cmd_buffer[ADBMS6380_COMMAND_PKT_SIZE], uint8_t* rx_buffer, size_t rx_length) {
	// First send command. Command is passed to all modules in the daisy chain.
	if (!PHAL_SPI_transfer_noDMA(spi, cmd_buffer, ADBMS6380_COMMAND_PKT_SIZE, 0, NULL)) {
		return false;
	}
	// Then read data back. Data is in order of module 0 ... module N-1
	if (!PHAL_SPI_transfer_noDMA(spi, NULL, 0, rx_length, rx_buffer)) {
		return false;
	}

	return true;
}

bool adbms6380_read_cell_voltages(
	SPI_InitConfig_t* spi,
	strbuf_t* cmd_buffer,
	uint8_t* rx_buffer,
	float** cell_voltages,
	size_t module_count
) {
	strbuf_clear(cmd_buffer);
	adbms6380_prepare_command(cmd_buffer, RDCVALL);
	size_t rx_length = module_count * ADBMS6380_RDCVALL_DATA_PKT_SIZE;
	if (!adbms6380_read(spi, module_count, cmd_buffer->data, rx_buffer, rx_length)) {
		return false;
	}
	// Data comes back as: module 0, module 1, ..., module N-1
	for (size_t module_idx = 0; module_idx < module_count; module_idx++) {
		uint8_t* module_data = &rx_buffer[module_idx * ADBMS6380_RDCVALL_DATA_PKT_SIZE];
		for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
			int16_t raw = adbms6380_extract_i16(module_data, j);
			float cell_v = adbms6380_raw_to_cell_v(raw);
			cell_voltages[module_idx][j] = cell_v;
		}
	}

	return true;
}

bool adbms6380_read_gpio_voltages(
	SPI_InitConfig_t* spi,
	strbuf_t* cmd_buffer,
	uint8_t* rx_buffer,
	float** gpio_voltages,
	size_t module_count
) {
	const uint8_t* cmd_list[4] = {RDAUXA, RDAUXB, RDAUXC, RDAUXD};

	for (size_t cmd_idx = 0; cmd_idx < 4; cmd_idx++) {
		strbuf_clear(cmd_buffer);
		adbms6380_prepare_command(cmd_buffer, cmd_list[cmd_idx]);

		if (!adbms6380_read_data(spi, module_count, cmd_buffer->data, rx_buffer)) {
			return false;
		}

		// AUXA/B/C: 3 GPIOs each, AUXD: GPIO10
		size_t gpios_read = (cmd_idx < 3) ? 3 : 1;
		size_t gpio_idx_base = cmd_idx * 3;

		// Data comes back as: module 0, module 1, ..., module N-1
		for (size_t module_idx = 0; module_idx < module_count; module_idx++) {
			uint8_t* module_data = &rx_buffer[module_idx * ADBMS6380_SINGLE_DATA_PKT_SIZE];

			for (size_t j = 0; j < gpios_read; j++) {
				size_t gpio_idx = gpio_idx_base + j;
				int16_t raw = adbms6380_extract_i16(module_data, j);
				float gpio_v = adbms6380_raw_to_gpio_v(raw);
				gpio_voltages[module_idx][gpio_idx] = gpio_v;
			}
		}
	}

	return true;
}
