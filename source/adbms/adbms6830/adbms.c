#include "adbms.h"
#include <assert.h>
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "external/STM32CubeG4/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.h"
#include "faults.h"
#include <stdint.h>
#include <string.h>

#include "common/phal/spi.h"
#include "common/phal/gpio.h"
#include "common/freertos/freertos.h"

#include "commands.h"
#include "pec.h"


typedef struct {
    uint8_t data[256];
    size_t length;
} string_t;

void clear_string(string_t *str) {
    str->length = 0;
}

void push_back_string(string_t *str, uint8_t *buf, size_t len) {
    memcpy(&(str->data[str->length]), buf, len);
    str->length += len;
}

string_t txstring = {0};

void adbms_periodic(adbms_t *bms) {
    bms->curr_state = bms->next_state;

    //check errors

    switch (bms->curr_state) {
        case ADBMS_IDLE: {
            // idle tasks

            if ((xTaskGetTickCount() - bms->last_connection_time_ms) > ADBMS_CONNECT_RETRY_PERIOD_MS) {
                bms->next_state = ADBMS_CONNECTING;
                bms->last_connection_time_ms = xTaskGetTickCount();
            }

            break;
        }
        case ADBMS_CONNECTING: {
            if (adbms_connect(bms)) { // try connection
                bms->next_state = ADBMS_DISCHARGING;
            } else {
                bms->next_state = ADBMS_IDLE;
            }
            break;
        }
        case ADBMS_DISCHARGING: {
            // adbms_read_cell_voltages(bms);


            // if (bms->enable_balance) {
            //     adbms_passive_balance(bms);
            // }
            // break;
        }
        case ADBMS_CHARGING: {
            // adbms_read_cell_voltages(bms);

            // if (bms->enable_balance) {
            //     adbms_passive_balance(bms);
            // }
            break;
        }
    }
}
void adbms_set_cs(adbms_t *bms, bool status) {
    GPIO_TypeDef *cs_port = bms->spi->nss_gpio_port;
    uint32_t cs_pin = bms->spi->nss_gpio_pin;
    PHAL_writeGPIO(cs_port, cs_pin, status);
}

bool adbms_connect(adbms_t *bms) {
    adbms_cfg_rega(bms);
    osDelay(1);
    adbms_cfg_regb(bms);

    osDelay(2);

    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, RDCFGA)) {
    //     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[13] = xTaskGetTickCount();
    //     return false;
    // }
    // if (false == adbms_receive_response(bms)) {
    //     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[14] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);
    // if (0 != memcmp(bms->rx_buffer, bms->rega_cfg, 6)) {
    // 	bms->last_fault_time[15] = xTaskGetTickCount();
    // 	return false;
    // };

    clear_string(&txstring);
    adbms_send_command(bms, RDCFGA);

    adbms_set_cs(bms, 0);
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, &(txstring.data), txstring.length, RX_DATA_LEN, bms->rx_buffer)) {
    	bms->last_fault_time[13] = xTaskGetTickCount();
        adbms_set_cs(bms, 1);
        return false;
    }
    size_t rx_len = RX_DATA_LEN * 2; // 2 BMS
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, NULL, 0, rx_len, bms->rx_buffer)) {
    	bms->last_fault_time[14] = xTaskGetTickCount();
        adbms_set_cs(bms, 1);
        return false;
    }
    adbms_set_cs(bms, 1);

    // Check configs
    if (0 != memcmp(bms->rx_buffer, bms->rega_cfg, 6)) {
    	bms->last_fault_time[15] = xTaskGetTickCount();
    	adbms_set_cs(bms, 1);
    	return false;
    };
    if (0 != memcmp(bms->rx_buffer + 8, bms->regb_cfg, 6)) {
    	bms->last_fault_time[16] = xTaskGetTickCount();
    	adbms_set_cs(bms, 1);
    	return false;
    };

    bms->last_connection_time_ms = xTaskGetTickCount();
    return true;
}


// IDK calls wake twice RN because we might have to wake once per BMS in a chain.
// IDK but now this can't be the issue right
void adbms_wake(adbms_t *bms) {
    for (int i = 0; i < 2; i++) {
        GPIO_TypeDef *cs_port = bms->spi->nss_gpio_port;
        uint32_t cs_pin = bms->spi->nss_gpio_pin;

        PHAL_writeGPIO(cs_port, cs_pin, 0);
        osDelay(ADBMS_WAKE_DELAY_MS);
        PHAL_writeGPIO(cs_port, cs_pin, 1);
        osDelay(ADBMS_WAKE_DELAY_MS);
    }
}


void adbms_send_command(adbms_t *bms, const uint8_t cmd[2]) {
    adbms_cmd_pkt_t outgoing = {0};
    outgoing.CMD[0] = cmd[0];
    outgoing.CMD[1] = cmd[1];

    uint16_t pec = adbms_get_pec15(2, outgoing.CMD);
    outgoing.CMD[2] = (pec >> 8) & 0xFF;
    outgoing.CMD[3] = pec & 0xFF;

    // spi transfer
    // if (false == PHAL_SPI_transfer_noDMA(bms->spi, (uint8_t *)&outgoing, sizeof(adbms_cmd_pkt_t), 0, NULL)) {
    // 	bms->last_fault_time[0] = xTaskGetTickCount();
    //     return false;
    // }

    // return true;
    //
    // ptxstring
    push_back_string(&txstring, (uint8_t *)&outgoing, sizeof(adbms_cmd_pkt_t));
}

void adbms_send_data(adbms_t *bms, const uint8_t data[TX_DATA_LEN]) {
    adbms_tx_data_t outgoing = {0};
    outgoing.DATA[0] = data[0];
    outgoing.DATA[1] = data[1];
    outgoing.DATA[2] = data[2];
    outgoing.DATA[3] = data[3];
    outgoing.DATA[4] = data[4];
    outgoing.DATA[5] = data[5];
    uint16_t pec = adbms_get_pec10(false, TX_DATA_LEN, outgoing.DATA);
    outgoing.DATA[6] = (pec >> 8) & 0xFF;
    outgoing.DATA[7] = pec & 0xFF;

    // spi transfer
    // if (false == PHAL_SPI_transfer_noDMA(bms->spi, (uint8_t *)&outgoing, sizeof(adbms_tx_data_t), 0, NULL)) {
    // 	bms->last_fault_time[1] = xTaskGetTickCount();
    //     return false;
    // }

    // return true;

    push_back_string(&txstring, (uint8_t *)&outgoing, sizeof(adbms_tx_data_t));
}

bool adbms_receive_response(adbms_t *bms) {
    // if (false == PHAL_SPI_transfer_noDMA(bms->spi, NULL, 0, RX_DATA_LEN, bms->rx_buffer)) {
    // 	bms->last_fault_time[2] = xTaskGetTickCount();
    //     return false;
    // }

    // return true;
}

bool adbms_cfg_rega(adbms_t *bms) {
    uint8_t rega_cfg[6] = {0};
    uint8_t refon = 0b1; // 1 = reference remains powered up until watchdog timeout. TODO determine
    uint8_t cth = 0b110; // C-ADC vs. S-ADC comparison voltage threshold (0b110 = 25.05 mV)
	rega_cfg[0] = (refon << 7) | (cth & 0x07);
    // rega_cfg[0] = cth;

    // all flags 0
    uint8_t flag_d = 0b00000000;
	rega_cfg[1] = flag_d;

    uint8_t soak_settings = 0;
    rega_cfg[2] = soak_settings;

    // All GPIO pull down off
	rega_cfg[3] = 0b11111111; // GPIOs [8:0] pull down OFF
	rega_cfg[4] = 0b00000011; // GPIOs [10:9] pull down OFF

    rega_cfg[5] = 0;

    memcpy(bms->rega_cfg, rega_cfg, 6);

    adbms_wake(bms);

    // TEST:
    // rega_cfg[0] = 0x0F;
    // rega_cfg[1] = 0x0F;
    // rega_cfg[2] = 0x0F;
    // rega_cfg[3] = 0x0F;
    // rega_cfg[4] = 0x0F;
    // rega_cfg[5] = 0x0F;
    // memcpy(bms->rega_cfg, rega_cfg, 6);

    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, WRCFGA)) {
	   //  adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[2] = xTaskGetTickCount();
    //     return false;
    // }

    // if (false == adbms_send_data(bms, rega_cfg)) {
	   //  adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[3] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);

    // DAIY x2: send command once, send data twice
    clear_string(&txstring);
    adbms_send_command(bms, WRCFGA);
    adbms_send_data(bms, rega_cfg);
    adbms_send_data(bms, rega_cfg);

    adbms_set_cs(bms, 0);
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, (const uint8_t*) &txstring.data, txstring.length, 0, NULL)) {
    	bms->last_fault_time[1] = xTaskGetTickCount();
        return false;
    }
    adbms_set_cs(bms, 1);

    return true;
}

/*
 * @param threshold_voltage between -3.4152 and 6.418
 */
uint16_t get_threshold_voltage_cfg(float threshold_voltage)
{
    uint16_t cfg;
    uint8_t rbits = 12;
    threshold_voltage = (threshold_voltage - 1.5);
    threshold_voltage = threshold_voltage / (16 * 0.000150);
    cfg = (uint16_t )(threshold_voltage + 2 * (1 << (rbits - 1)));
    cfg &= 0xFFF;
    return cfg;
}

bool adbms_cfg_regb(adbms_t *bms) {
    uint8_t regb_cfg[6] = {0};

    // todo load up regb config
    const uint16_t overvoltage_threshold = get_threshold_voltage_cfg(4.2);
    const uint16_t undervoltage_threshold = get_threshold_voltage_cfg(2.0);
    // 12 bits vov
    regb_cfg[0] = overvoltage_threshold & 0xFF;
    regb_cfg[1] |= (overvoltage_threshold >> 8) & 0xF;
    // 12 bits vuv
    regb_cfg[1] |= (undervoltage_threshold << 4) & 0xF0;
    regb_cfg[2] |= (undervoltage_threshold >> 4) & 0xFF;
    // all else default
    regb_cfg[3] = 0;
    // regb_cfg[4] = 0b00000100;
    regb_cfg[4] = 0;
    regb_cfg[5] = 0b00000100;

    memcpy(bms->regb_cfg, regb_cfg, 6);

    adbms_wake(bms);

    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, WRCFGB)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[4] = xTaskGetTickCount();
    //     return false;
    // }
    // if (false == adbms_send_data(bms, regb_cfg)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[5] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);

    // return true;

    // DAIY x2: send command once, send data twice
    adbms_send_command(bms, WRCFGB);
    adbms_send_data(bms, regb_cfg);
    adbms_send_data(bms, regb_cfg);
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, (const uint8_t*) &txstring.data, txstring.length, 0, NULL)) {
    	bms->last_fault_time[5] = xTaskGetTickCount();
        return false;
    }
    return true;
}


int16_t extract_i16(uint8_t *data, int idx) {
    return (int16_t)(data[idx * 2 + 0] & 0xff) | ((int16_t)(data[idx * 2 + 1] & 0xff) << 8);
}
float raw_to_cell_v(int16_t raw) {
	return (raw + 10000) * 0.000150f;
}

void adBms6830_Adcv(uint8_t *cmd, uint8_t rd, uint8_t cont, uint8_t dcp, uint8_t rstf, uint8_t owcs)
{
    cmd[0] = 0x02 + rd;
    cmd[1] = (cont << 7) + (dcp << 4) + (rstf << 2) + (owcs & 0x03) + 0x60;
}

bool adbms_read_cell_voltages(adbms_t *bms) {
	// adbms_connect(bms);

	// osDelay(5);

    // uint8_t cmd[2] = { 0 };
    // adBms6830_Adcv(cmd, 0, 0, 0, 0, 0);

    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, cmd)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[6] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);

    // // dont delay 7 or else isoSPI sleeps
    // osDelay(20);
    // adbms_wake(bms);

    // const uint8_t *cmd_list[6] = {RDCVA, RDCVB, RDCVC, RDCVD, RDCVE, RDCVF};

    // bms->any_skipped = false;

    // for (int group = 0; group < 6; group++) {
	//     adbms_set_cs(bms, 0);
    //     if (false == adbms_send_command(bms, cmd_list[group])) {
	// 	    adbms_set_cs(bms, 1);
	//     	bms->last_fault_time[7] = xTaskGetTickCount();
    //         return false;
    //     }
    //     if (false == adbms_receive_response(bms)) {
	// 	    adbms_set_cs(bms, 1);
	//     	bms->last_fault_time[8] = xTaskGetTickCount();
    //         return false;
    //     }
	//     adbms_set_cs(bms, 1);

    //     size_t cells_read = (group <= 4) ? 3 : 1;
    //    	size_t cell_idx_base = group * 3;
    //    	for (int j = 0; j < cells_read; j++) {
	// 		int16_t raw = extract_i16(bms->rx_buffer, j);
    //   		size_t idx = cell_idx_base + j;
	// 		float cell_v = raw_to_cell_v(raw);
	// 		// bms->cell_voltages[idx] = (float)raw;
	// 		if (cell_v >= 0) {
	// 			bms->cell_voltages[idx] = cell_v;
	// 		} else {
	// 			bms->any_skipped = true;
	// 		}
    //    	}
    // }

    return true;
}

#define BALANCING_MIN_VOLTAGE 3.2f // V
#define BALANCING_MIN_DELTA 0.03f // V

bool adbms_passive_balance(adbms_t *bms) {
	osDelay(1);
    // float lowest_voltage = bms->cell_voltages[0];

    // // find lowest voltage
    // for (int i = 0; i < NUM_CELLS; i++) {
    //     float curr_voltage = bms->cell_voltages[i];

    //     if (curr_voltage < lowest_voltage) {
    //         lowest_voltage = curr_voltage;
    //     }
    // }

    // if (lowest_voltage < BALANCING_MIN_VOLTAGE) {
    //     return false;
    // }

    // const float balance_threshold = lowest_voltage + BALANCING_MIN_DELTA;
    // for (int i = 0; i < NUM_CELLS; i++) {
    //     float curr_voltage = bms->cell_voltages[i];

    //     if (curr_voltage > balance_threshold) {
    //         bms->cell_pwms[i] = PWM_66_0_PCT; // 66% duty cycle
    //     } else {
    //         bms->cell_pwms[i] = PWM_0_0_PCT; // turn it off
    //     }
    // }

    bms->cell_pwms[2] = PWM_66_0_PCT;

    // TODO transmit the commands
    uint8_t pwma_data[6] = { 0 };
    uint8_t pwmb_data[6] = { 0 };
    const size_t cells_per_reg = 12;
    const size_t bits_per_pwm = 4;
    for (size_t i = 0; i < NUM_CELLS; i++) {
        size_t bit_pos   = i * bits_per_pwm;
        size_t byte_idx  = bit_pos / 8;
        size_t bit_shift = bit_pos % 8;
        uint8_t pwm_nibble = bms->cell_pwms[i] & 0x0F;
        uint8_t raw = pwm_nibble << bit_shift;

        // Don't need to worry about some of the bits overlapping into a different byte
        if (i < cells_per_reg) {
            pwma_data[byte_idx] |= raw;
        } else {
        	// TODO: fix byte idx
            // ignore for now, it's okay
	        // pwmb_data[byte_idx] |= raw;
        }
    }

    // Send A
    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, WRPWMA)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[9] = xTaskGetTickCount();
    //     return false;
    // }
    // if (false == adbms_send_data(bms, pwma_data)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[10] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);

    // // Send B
    // adbms_set_cs(bms, 0);
    // if (false == adbms_send_command(bms, WRPWMB)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[11] = xTaskGetTickCount();
    //     return false;
    // }
    // if (false == adbms_send_data(bms, pwmb_data)) {
	//     adbms_set_cs(bms, 1);
    // 	bms->last_fault_time[12] = xTaskGetTickCount();
    //     return false;
    // }
    // adbms_set_cs(bms, 1);


    return true;
}