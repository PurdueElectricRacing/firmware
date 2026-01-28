#include "adbms6380.h"

#include <stdbool.h>

#include "common/phal/gpio.h"
#include "common/phal/spi.h"
#include "common/psched/psched.h"


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

float raw_to_cell_v(int16_t raw) {
	return (raw + 10000) * 0.000150f;
}

void adbms6830_adcv(uint8_t* cmd, uint8_t rd, uint8_t cont, uint8_t dcp, uint8_t rstf, uint8_t owcs) {
	cmd[0] = 0x02 + rd;
    cmd[1] = (cont << 7) + (dcp << 4) + (rstf << 2) + (owcs & 0x03) + 0x60;
}

void adbms6830_adsv(uint8_t* cmd, uint8_t rd, uint8_t cont, uint8_t dcp, uint8_t rstf, uint8_t owcs) {
	//! TODO: implement
}