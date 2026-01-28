#ifndef _BMS_H_
#define _BMS_H_

#include <stdint.h>
#include <stddef.h>

#include "common/phal/spi.h"

#include "common/adbms/adbms6380.h"


#define ADBMS_MODULE_COUNT       (6)
#define ADBMS_MODULE_CELL_COUNT  (16)
#define ADBMS_MODULE_THERM_COUNT (10)

// Max SPI TX is a command + all the data packets for all the modules
#define ADBMS_SPI_TX_BUFFER_SIZE (ADBMS6380_COMMAND_PKT_SIZE + (ADBMS_MODULE_CELL_COUNT * ADBMS6380_SINGLE_DATA_PKT_SIZE))
#define ADBMS_SPI_RX_BUFFER_SIZE (ADBMS_SPI_TX_BUFFER_SIZE) //! Fix for READALL

typedef enum {
	ADBMS_STATE_IDLE = 0,
	ADBMS_STATE_CONNECTING,
	ADBMS_STATE_CONNECTED,
	ADBMS_STATE_CHARING,
} ADBMS_state_t;


typedef struct {
	float cell_voltages[ADBMS_MODULE_CELL_COUNT]; // in volts
	float min_voltage; // in volts
	float max_voltage; // in volts
	float avg_voltage; // in volts
	float sum_voltage; // in volts
	float thermistors[ADBMS_MODULE_THERM_COUNT]; // in degrees Celsius
	//! TODO: do we also want min/max/avg for thermistors?
	bool is_discharging[ADBMS_MODULE_CELL_COUNT]; // if a cell is being discharged for balancing

	uint8_t rega[ADBMS6380_SINGLE_DATA_RAW_SIZE];
	uint8_t regb[ADBMS6380_SINGLE_DATA_RAW_SIZE];

	bool error_1; //! Fill in actual errors
} ADBMS_module_t;

typedef struct {
	ADBMS_state_t state;

	ADBMS_module_t modules[ADBMS_MODULE_COUNT];

	float min_voltage; // in volts
	float max_voltage; // in volts
	float avg_voltage; // in volts
	float sum_voltage; // in volts

	// If the BMS is allowed to discharge cells for balancing. Controlled by higher level logic.
	bool is_discharge_enabled;

	SPI_InitConfig_t* spi; // Note: must not use auto CS pin, bms driver controls CS manually
	uint8_t tx_buffer[ADBMS_SPI_TX_BUFFER_SIZE]; //! Change to new buffer type when #247 is done
	uint8_t rx_buffer[ADBMS_SPI_RX_BUFFER_SIZE];
} ADBMS_bms_t;

void adbms_periodic(ADBMS_bms_t* bms);

void adbms_init(ADBMS_bms_t* bms, SPI_InitConfig_t* spi);
void adbms_connect(ADBMS_bms_t* bms);
void adbms_balance_cells(ADBMS_bms_t* bms, float min_voltage, float min_delta);



#endif // _BMS_H_