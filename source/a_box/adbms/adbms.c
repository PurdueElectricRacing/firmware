#include "adbms.h"

#include <string.h>

#include "common/phal/spi.h"

#include "adbms6380.h"
#include "commands.h"


void adbms_init(ADBMS_bms_t* bms, SPI_InitConfig_t* spi) {
	bms->spi = spi;
	bms->state = ADBMS_STATE_IDLE;

	bms->is_discharge_enabled = false;
	for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
		for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
			bms->modules[i].is_discharging[j] = false;
		}
	}

	// TODO: strbuf init??
}

bool adbms_write_rega(ADBMS_bms_t* bms) {
	strbuf_clear(&bms->tx_strbuf);
	adbms6380_prepare_command(&bms->tx_strbuf, WRCFGA);
	// i must be signed
	for (int i = ADBMS_MODULE_COUNT - 1; i >= 0; i--) {
		adbms6380_calculate_cfg_rega(bms->modules[i].rega, ADBMS_REFON, ADBMS_CTH);
		adbms6380_prepare_data_packet(&bms->tx_strbuf, bms->modules[i].rega);
	}

	return PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL);
}

bool adbms_write_regb(ADBMS_bms_t* bms) {
	strbuf_clear(&bms->tx_strbuf);
	adbms6380_prepare_command(&bms->tx_strbuf, WRCFGB);
	// i must be signed
	for (int i = ADBMS_MODULE_COUNT - 1; i >= 0; i--) {
		adbms6380_calculate_cfg_regb(bms->modules[i].regb, ADBMS_OV_THRESHOLD, ADBMS_UV_THRESHOLD, bms->modules[i].is_discharging);
		adbms6380_prepare_data_packet(&bms->tx_strbuf, bms->modules[i].regb);
	}

	return PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL);
}

bool adbms_read_and_check_rega(ADBMS_bms_t* bms) {
	strbuf_clear(&bms->tx_strbuf);
	adbms6380_prepare_command(&bms->tx_strbuf, RDCFGA);
	if (!adbms6380_read(bms->spi, ADBMS_MODULE_COUNT, bms->tx_strbuf.data, bms->rx_buf)) {
		return false;
	}
	for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
		uint8_t* module_data = &bms->rx_buf[i * ADBMS6380_SINGLE_DATA_PKT_SIZE];
		if (memcmp(&module_data[0], bms->modules[i].rega, ADBMS6380_SINGLE_DATA_RAW_SIZE) != 0) {
			// TODO: set errors
			return false;
		}
	}
	return true;
}

bool adbms_read_and_check_regb(ADBMS_bms_t* bms) {
	strbuf_clear(&bms->tx_strbuf);
	adbms6380_prepare_command(&bms->tx_strbuf, RDCFGB);
	if (!adbms6380_read(bms->spi, ADBMS_MODULE_COUNT, bms->tx_strbuf.data, bms->rx_buf)) {
		return false;
	}
	for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
		uint8_t* module_data = &bms->rx_buf[i * ADBMS6380_SINGLE_DATA_PKT_SIZE];
		if (memcmp(&module_data[0], bms->modules[i].regb, ADBMS6380_SINGLE_DATA_RAW_SIZE) != 0) {
			// TODO: set errors
			return false;
		}
	}
	return true;
}

void adbms_connect(ADBMS_bms_t* bms) {
	// Wake, write REGA, REGB, read back to verify, ADCV, ADSV start
	adbms6380_wake(bms->spi, ADBMS_MODULE_COUNT);

	if (!adbms_write_rega(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}
	if (!adbms_write_regb(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}
	if (!adbms_read_and_check_rega(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}
	if (!adbms_read_and_check_regb(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}

	// Start ADCV
	strbuf_clear(&bms->tx_strbuf);
	uint8_t adcv_cmd[2];
	adbms6830_adcv(adcv_cmd, ADBMS_RD, ADBMS_CONT, ADBMS_DCP, ADBMS_RSTF, ADBMS_OW);
	adbms6380_prepare_command(&bms->tx_strbuf, adcv_cmd);
	if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}

	// Start ADSV
	strbuf_clear(&bms->tx_strbuf);
	uint8_t adsv_cmd[2];
	adbms6830_adsv(adsv_cmd, ADBMS_CONT, ADBMS_DCP, ADBMS_OW);
	adbms6380_prepare_command(&bms->tx_strbuf, adsv_cmd);
	if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}

	bms->state = ADBMS_STATE_CONNECTED;
}

void adbms_calculate_balance_cells(ADBMS_bms_t* bms, float min_voltage, float min_delta) {
	if (!bms->is_discharge_enabled) {
		// Disable all discharging
		for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
			for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
				bms->modules[i].is_discharging[j] = false;
			}
		}
		return;
	}
	if (bms->min_voltage < min_voltage) {
		return;
	}

	float balance_threshold = bms->min_voltage + min_delta;
	for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
		for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
			if (bms->modules[i].cell_voltages[j] > balance_threshold) {
				bms->modules[i].is_discharging[j] = true;
			} else {
				bms->modules[i].is_discharging[j] = false;
			}
		}
	}
}

void adbms_balance_and_update_regb(ADBMS_bms_t* bms, float min_voltage, float min_delta) {
	adbms_calculate_balance_cells(bms, min_voltage, min_delta);

	if (!adbms_write_regb(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}
	if (!adbms_read_and_check_regb(bms)) {
		bms->state = ADBMS_STATE_IDLE;
		return;
	}
}