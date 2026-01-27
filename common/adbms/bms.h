#ifndef _BMS_H_
#define _BMS_H_

#include <stdint.h>
#include <stddef.h>

#include "common/phal/spi.h"

#include "common/adbms/adbms6380.h"


constexpr size_t ADBMS_MODULE_COUNT = 6;
constexpr size_t ADBMS_MODULE_CELL_COUNT = 16;
constexpr size_t ADBMS_MODULE_THERM_COUNT = 10;

// Max SPI TX is a command + all the data packets for all the modules
constexpr size_t ADBMS_SPI_TX_BUFFER_SIZE = ADBMS6380_COMMAND_PKT_SIZE + (ADBMS_MODULE_CELL_COUNT * ADBMS6380_SINGLE_DATA_PKT_SIZE);
constexpr size_t ADBMS_SPI_RX_BUFFER_SIZE = ADBMS_SPI_TX_BUFFER_SIZE; //! Fix for READALL



typedef struct {
	float cell_voltages[MODULE_CELL_COUNT]; // in volts
	float min_voltage; // in volts
	float max_voltage; // in volts
	float avg_voltage; // in volts
	float sum_voltage; // in volts
	float thermistors[MODULE_THERM_COUNT]; // in degrees Celsius
	//! TODO: do we also want min/max/avg for thermistors?
	bool is_discharging[MODULE_CELL_COUNT]; // if a cell is being discharged for balancing

	bool error_1; //! Fill in actual errors
} ADBMS_Module;

typedef struct {
	ADBMS_Module modules[ADBMS_MODULE_COUNT];

	float min_voltage; // in volts
	float max_voltage; // in volts
	float avg_voltage; // in volts
	float sum_voltage; // in volts

	bool is_discharge_enabled; // If the BMS is allowed to discharge cells for balancing

	SPI_InitConfig_t *spi;
	uint8_t tx_buffer[ADBMS_SPI_TX_BUFFER_SIZE]; //! Change to new buffer type when #247 is done
	uint8_t rx_buffer[ADBMS_SPI_RX_BUFFER_SIZE];
} ADBMS_BMS;



#endif // _BMS_H_