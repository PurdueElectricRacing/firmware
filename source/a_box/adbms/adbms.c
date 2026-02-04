#include "adbms.h"#include "adbms.h"

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

