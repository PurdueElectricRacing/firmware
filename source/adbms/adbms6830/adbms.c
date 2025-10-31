#include "adbms.h"
#include <assert.h>
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "faults.h"
#include <stdint.h>

#include "common/phal/spi.h"
#include "common/phal/gpio.h"
#include "common/freertos/freertos.h"

#include "commands.h"
#include "pec.h"

void adbms_periodic(adbms_t *bms) {
    bms->curr_state = bms->next_state;

    //check errors

    switch (bms->curr_state) {
        case ADBMS_IDLE: {
            // idle tasks

            if ((xTaskGetTickCount() - bms->last_connection_time_ms) < ADBMS_CONNECT_RETRY_PERIOD_MS) {
                bms->next_state = ADBMS_DISCHARGING;
            }

            break;
        }
        case ADBMS_CONNECTING: {
            // if (adbms_connect(bms)) { // try connection
            //     bms->next_state = ADBMS_DISCHARGING;
            // } else {
            //     bms->next_state = ADBMS_IDLE;
            // }
        }
        case ADBMS_DISCHARGING: {
            adbms_read_cell_voltages(bms);

            if (bms->enable_balance) {
                adbms_passive_balance(bms);
            }
            break;
        }
        case ADBMS_CHARGING: {
            adbms_read_cell_voltages(bms);

            if (bms->enable_balance) {
                adbms_passive_balance(bms);
            }
            break;
        }
    }
}

void adbms_set_cs(adbms_t *bms, bool status) {
    GPIO_TypeDef *cs_port = bms->spi->nss_gpio_port;
    uint32_t cs_pin = bms->spi->nss_gpio_pin;
    PHAL_writeGPIO(cs_port, cs_pin, status);
}

void adbms_wake(adbms_t *bms) {
    GPIO_TypeDef *cs_port = bms->spi->nss_gpio_port;
    uint32_t cs_pin = bms->spi->nss_gpio_pin;

    PHAL_writeGPIO(cs_port, cs_pin, 0);
    osDelay(ADBMS_WAKE_DELAY_MS);
    PHAL_writeGPIO(cs_port, cs_pin, 1);
    osDelay(ADBMS_WAKE_DELAY_MS);
}



bool adbms_send_command(adbms_t *bms, const uint8_t cmd[2]) {
    adbms_cmd_pkt_t outgoing = {0};
    outgoing.CMD[0] = cmd[0];
    outgoing.CMD[1] = cmd[1];
    outgoing.PEC15 = adbms_get_pec15(2, outgoing.CMD);

    // spi transfer
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, (uint8_t *)&outgoing, sizeof(adbms_cmd_pkt_t), TX_CMD_LEN, bms->rx_buffer)) {
    	bms->last_fault_time[0] = xTaskGetTickCount();
        return false;
    }

    return true;
}

bool adbms_send_data(adbms_t *bms, const uint8_t data[TX_DATA_LEN]) {
    adbms_tx_data_t outgoing = {0};
    outgoing.DATA[0] = data[0];
    outgoing.DATA[1] = data[1];
    outgoing.DATA[2] = data[3];
    outgoing.DATA[4] = data[4];
    outgoing.DATA[5] = data[5];
    outgoing.DPEC10 = adbms_get_pec10(false, TX_DATA_LEN, outgoing.DATA);

    // spi transfer
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, (uint8_t *)&outgoing, sizeof(adbms_tx_data_t), 0, NULL)) {
    	bms->last_fault_time[1] = xTaskGetTickCount();
        return false;
    }

    return true;
}

bool adbms_receive_response(adbms_t *bms, size_t response_len) {
    if (false == PHAL_SPI_transfer_noDMA(bms->spi, NULL, 0, RX_DATA_LEN, bms->rx_buffer)) {
    	bms->last_fault_time[2] = xTaskGetTickCount();
        return false;
    }

    return true;
}

bool adbms_cfg_rega(adbms_t *bms) {
    uint8_t rega_cfg[6] = {0};
    uint8_t refon = 0b1; // 1 = reference remains powered up until watchdog timeout. TODO determine
    uint8_t cth = 0b110; // C-ADC vs. S-ADC comparison voltage threshold (0b110 = 25.05 mV)
	rega_cfg[0] = (refon << 7) | (cth & 7);
    rega_cfg[0] = cth;

    // all flags 0
    uint8_t flag_d = 0b00000000;
	rega_cfg[1] = flag_d;

    uint8_t soak_settings = 0;
    rega_cfg[2] = soak_settings;

    // All GPIO pull down off
	rega_cfg[3] = 0b11111111; // GPIOs [8:0] pull down OFF
	rega_cfg[4] = 0b00000011; // GPIOs [10:9] pull down OFF

    rega_cfg[5] = 0;

    adbms_wake(bms);

    adbms_set_cs(bms, 0);
    if (false == adbms_send_command(bms, WRCFGA)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[2] = xTaskGetTickCount();
        return false;
    }
    if (false == adbms_send_data(bms, rega_cfg)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[3] = xTaskGetTickCount();
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
    regb_cfg[4] = 0;
    regb_cfg[5] = 0;

    adbms_wake(bms);

    adbms_set_cs(bms, 0);
    if (false == adbms_send_command(bms, WRCFGA)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[4] = xTaskGetTickCount();
        return false;
    }
    if (false == adbms_send_data(bms, regb_cfg)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[5] = xTaskGetTickCount();
        return false;
    }
    adbms_set_cs(bms, 1);

    return true;
}


int16_t extract_i16(uint8_t *data, int idx) {
    return (int16_t)(data[idx * 2 + 0] & 0xff) | ((int16_t)(data[idx * 2 + 1] & 0xff) << 8);
}
float raw_to_cell_v(int16_t raw) {
	return (raw + 10000) * 0.000150f;
}

bool adbms_read_cell_voltages(adbms_t *bms) {
    adbms_wake(bms);


    // adBms6830_Adcv(ADCV_RD_ON, ADCV_CONT_CONTINUOUS, DCP_OFF, RSTF_OFF, OW_OFF_ALL_CH);
    uint8_t cmd[2];
    cmd[0] = 0x02 + (0x01);
    cmd[1] = ((0x01) << 7) + ((0x00) << 4) + ((0x00) << 2) + ((0x00) & 0x03) + 0x60;
    adbms_set_cs(bms, 0);
    if (false == adbms_send_command(bms, cmd)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[6] = xTaskGetTickCount();
        return false;
    }
    adbms_set_cs(bms, 1);
    osDelay(1);

    const uint8_t *cmd_list[6] = {RDCVA, RDCVB, RDCVC, RDCVD, RDCVE, RDCVF};

    for (int group = 0; group < CELL_GROUPS; group++) {
	    adbms_set_cs(bms, 0);
        if (false == adbms_send_command(bms, cmd_list[group])) {
		    adbms_set_cs(bms, 1);
    	bms->last_fault_time[7] = xTaskGetTickCount();
            return false;
        }
        if (false == adbms_receive_response(bms, TX_DATA_LEN)) {
		    adbms_set_cs(bms, 1);
    	bms->last_fault_time[8] = xTaskGetTickCount();
            return false;
        }
	    adbms_set_cs(bms, 1);

        size_t cells_read = (group <= 4) ? 3 : 1;
       	size_t cell_idx_base = group * 3;
       	for (int j = 0; j < cells_read; j++) {
			int16_t raw = extract_i16(bms->rx_buffer, j);
      		size_t idx = cell_idx_base + j;
			float cell_v = raw_to_cell_v(raw);
			bms->cell_voltages[idx] = (float)raw;
       	}
    }
}

#define BALANCING_MIN_VOLTAGE 3.2f // V
#define BALANCING_MIN_DELTA 0.03f // V

bool adbms_passive_balance(adbms_t *bms) {
    float lowest_voltage = bms->cell_voltages[0];

    // find lowest voltage
    for (int i = 0; i < NUM_CELLS; i++) {
        float curr_voltage = bms->cell_voltages[i];

        if (curr_voltage < lowest_voltage) {
            lowest_voltage = curr_voltage;
        }
    }

    if (lowest_voltage < BALANCING_MIN_VOLTAGE) {
        return false;
    }

    const float balance_threshold = lowest_voltage + BALANCING_MIN_DELTA;
    for (int i = 0; i < NUM_CELLS; i++) {
        float curr_voltage = bms->cell_voltages[i];

        if (curr_voltage > balance_threshold) {
            bms->cell_pwms[i] = PWM_66_0_PCT; // 66% duty cycle
        } else {
            bms->cell_pwms[i] = PWM_0_0_PCT; // turn it off
        }
    }

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
            pwmb_data[byte_idx] |= raw;
        }
    }

    // Send A
    adbms_set_cs(bms, 0);
    if (false == adbms_send_command(bms, WRPWMA)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[9] = xTaskGetTickCount();
        return false;
    }
    if (false == adbms_send_data(bms, pwma_data)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[10] = xTaskGetTickCount();
        return false;
    }
    adbms_set_cs(bms, 1);

    // Send B
    adbms_set_cs(bms, 0);
    if (false == adbms_send_command(bms, WRPWMB)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[11] = xTaskGetTickCount();
        return false;
    }
    if (false == adbms_send_data(bms, pwmb_data)) {
	    adbms_set_cs(bms, 1);
    	bms->last_fault_time[12] = xTaskGetTickCount();
        return false;
    }
    adbms_set_cs(bms, 1);
}
