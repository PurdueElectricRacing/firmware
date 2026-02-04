#ifndef _BMS_H_
#define _BMS_H_

#include <stdint.h>
#include <stddef.h>

#include "common/phal/spi.h"
#include "common/strbuf/strbuf.h"

#include "adbms6380.h"


#define ADBMS_MODULE_COUNT       (8)

// Max SPI TX is a command + all the data packets for all the modules
#define ADBMS_SPI_TX_BUFFER_SIZE (ADBMS6380_COMMAND_PKT_SIZE + (ADBMS6380_CELL_COUNT * ADBMS6380_SINGLE_DATA_PKT_SIZE))
#define ADBMS_SPI_RX_BUFFER_SIZE (ADBMS_SPI_TX_BUFFER_SIZE) //! Fix for READALL

#define ADBMS_REFON		   (true)  // REGA
#define ADBMS_CTH		   (0b110) // REGA - 25.05 mV
#define ADBMS_OV_THRESHOLD (4.2f)  // REGB - in volts
#define ADBMS_UV_THRESHOLD (3.0f)  // REGB - in volts
#define ADBMS_RD           (false) // ADCV
#define ADBMS_CONT         (true)  // ADCV/ADSV
#define ADBMS_DCP          (false) // ADCV/ADSV
#define ADBMS_RSTF         (true)  // ADCV
#define ADBMS_OW           (0b00)  // ADCV/ADSV


typedef enum {
	ADBMS_STATE_IDLE = 0,
	ADBMS_STATE_CONNECTED,
} ADBMS_state_t;


typedef struct {
	float cell_voltages[ADBMS6380_CELL_COUNT]; // in volts
	float min_voltage; // in volts
	float max_voltage; // in volts
	float avg_voltage; // in volts
	float sum_voltage; // in volts
	float thermistors[ADBMS6380_GPIO_COUNT]; // in degrees Celsius
	//! TODO: do we also want min/max/avg for thermistors?
	bool is_discharging[ADBMS6380_CELL_COUNT]; // if a cell is being discharged for balancing

	uint8_t rega[ADBMS6380_SINGLE_DATA_RAW_SIZE];
	uint8_t regb[ADBMS6380_SINGLE_DATA_RAW_SIZE];

	bool err_rega_mismatch;
	bool err_regb_mismatch;
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
	strbuf_t tx_strbuf;
	uint8_t rx_buf[ADBMS_SPI_RX_BUFFER_SIZE];

	bool err_spi;
	bool err_connect;
	bool err_rega_mismatch;
	bool err_regb_mismatch;
} ADBMS_bms_t;


void adbms_init(ADBMS_bms_t* bms, SPI_InitConfig_t* spi, uint8_t* tx_buf);

bool adbms_write_rega(ADBMS_bms_t* bms);
bool adbms_write_regb(ADBMS_bms_t* bms);
bool adbms_read_and_check_rega(ADBMS_bms_t* bms);
bool adbms_read_and_check_regb(ADBMS_bms_t* bms);

void adbms_connect(ADBMS_bms_t* bms);

void adbms_read_cells(ADBMS_bms_t* bms);
void adbms_read_therms(ADBMS_bms_t* bms);

void adbms_calculate_balance_cells(ADBMS_bms_t* bms, float min_voltage, float min_delta);
void adbms_balance_and_update_regb(ADBMS_bms_t* bms, float min_voltage, float min_delta);

void adbms_periodic(ADBMS_bms_t* bms, float min_voltage_for_balance, float min_delta_for_balance);


#endif // _BMS_H_