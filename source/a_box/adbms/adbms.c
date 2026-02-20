/**
 * @file adbms.c
 * @brief Primary logic and interface for ADBMS battery management system driver.
 *
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "adbms.h"

#include <string.h>

#include "adbms6380.h"
#include "commands.h"
#include "common/phal/spi.h"
#include "thermistor.h"

void adbms_init(ADBMS_bms_t *bms, SPI_InitConfig_t *spi, uint8_t *tx_buf) {
    bms->spi   = spi;
    bms->state = ADBMS_STATE_IDLE;

    bms->is_discharge_enabled = false;
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
            bms->modules[i].is_discharging[j] = false;
        }
        bms->modules[i].err_rega_mismatch = false;
        bms->modules[i].err_regb_mismatch = false;
    }

    bms->err_spi           = false;
    bms->err_connect       = false;
    bms->err_rega_mismatch = false;
    bms->err_regb_mismatch = false;

    bms->tx_strbuf = (strbuf_t) {
        .data    = tx_buf,
        .length  = 0,
        .max_len = ADBMS_SPI_TX_BUFFER_SIZE,
    };
}

bool adbms_write_rega(ADBMS_bms_t *bms) {
    strbuf_clear(&bms->tx_strbuf);
    adbms6380_prepare_command(&bms->tx_strbuf, WRCFGA);
    // i must be signed
    for (int i = ADBMS_MODULE_COUNT - 1; i >= 0; i--) {
        adbms6380_calculate_cfg_rega(bms->modules[i].rega, ADBMS_REGA_REFON, ADBMS_REGA_CTH);
        adbms6380_prepare_data_packet(&bms->tx_strbuf, bms->modules[i].rega);
    }

    adbms6380_set_cs_low(bms->spi);
    if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
        adbms6380_set_cs_high(bms->spi);
        bms->err_spi = true;
        return false;
    }
    adbms6380_set_cs_high(bms->spi);

    return true;
}

bool adbms_write_regb(ADBMS_bms_t *bms) {
    strbuf_clear(&bms->tx_strbuf);
    adbms6380_prepare_command(&bms->tx_strbuf, WRCFGB);
    // i must be signed
    for (int i = ADBMS_MODULE_COUNT - 1; i >= 0; i--) {
        adbms6380_calculate_cfg_regb(bms->modules[i].regb,
                                     ADBMS_REGB_OV_THRESHOLD,
                                     ADBMS_REGB_UV_THRESHOLD,
                                     bms->modules[i].is_discharging);
        adbms6380_prepare_data_packet(&bms->tx_strbuf, bms->modules[i].regb);
    }

    adbms6380_set_cs_low(bms->spi);
    if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
        adbms6380_set_cs_high(bms->spi);
        bms->err_spi = true;
        return false;
    }
    adbms6380_set_cs_high(bms->spi);
    return true;
}

bool adbms_read_and_check_rega(ADBMS_bms_t *bms) {
    strbuf_clear(&bms->tx_strbuf);
    adbms6380_prepare_command(&bms->tx_strbuf, RDCFGA);
    if (!adbms6380_read_data(bms->spi, ADBMS_MODULE_COUNT, bms->tx_strbuf.data, bms->rx_buf)) {
        bms->err_spi = true;
        return false;
    }
    bms->err_rega_mismatch = false;
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        uint8_t *module_data = &bms->rx_buf[i * ADBMS6380_SINGLE_DATA_PKT_SIZE];
        if (memcmp(&module_data[0], bms->modules[i].rega, ADBMS6380_SINGLE_DATA_RAW_SIZE) != 0) {
            bms->err_rega_mismatch            = true;
            bms->modules[i].err_rega_mismatch = true;
        } else {
            bms->modules[i].err_rega_mismatch = false;
        }
    }
    return !bms->err_rega_mismatch;
}

bool adbms_read_and_check_regb(ADBMS_bms_t *bms) {
    strbuf_clear(&bms->tx_strbuf);
    adbms6380_prepare_command(&bms->tx_strbuf, RDCFGB);
    if (!adbms6380_read_data(bms->spi, ADBMS_MODULE_COUNT, bms->tx_strbuf.data, bms->rx_buf)) {
        bms->err_spi = true;
        return false;
    }
    bms->err_regb_mismatch = false;
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        uint8_t *module_data = &bms->rx_buf[i * ADBMS6380_SINGLE_DATA_PKT_SIZE];
        if (memcmp(&module_data[0], bms->modules[i].regb, ADBMS6380_SINGLE_DATA_RAW_SIZE) != 0) {
            bms->err_regb_mismatch            = true;
            bms->modules[i].err_regb_mismatch = true;
        } else {
            bms->modules[i].err_regb_mismatch = false;
        }
    }
    return !bms->err_regb_mismatch;
}

void adbms_connect(ADBMS_bms_t *bms) {
    if (!adbms_write_rega(bms)) {
        bms->state       = ADBMS_STATE_IDLE;
        bms->err_connect = true;
        return;
    }
    if (!adbms_write_regb(bms)) {
        bms->state       = ADBMS_STATE_IDLE;
        bms->err_connect = true;
        return;
    }
    if (!adbms_read_and_check_rega(bms)) {
        bms->state       = ADBMS_STATE_IDLE;
        bms->err_connect = true;
        return;
    }
    if (!adbms_read_and_check_regb(bms)) {
        bms->state       = ADBMS_STATE_IDLE;
        bms->err_connect = true;
        return;
    }

    // Start ADCV
    strbuf_clear(&bms->tx_strbuf);
    uint8_t adcv_cmd[2] = {0};
    adbms6380_adcv(adcv_cmd,
                   ADBMS_ADCV_RD,
                   ADBMS_ADCV_CONT,
                   ADBMS_ADCV_DCP,
                   ADBMS_ADCV_RSTF,
                   ADBMS_ADCV_OW);
    adbms6380_prepare_command(&bms->tx_strbuf, adcv_cmd);
    adbms6380_set_cs_low(bms->spi);
    if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
        adbms6380_set_cs_high(bms->spi);
        bms->state       = ADBMS_STATE_IDLE;
        bms->err_spi     = true;
        bms->err_connect = true;
        return;
    }
    adbms6380_set_cs_high(bms->spi);

    bms->err_connect = false;
    bms->state       = ADBMS_STATE_CONNECTED;
}

void adbms_calculate_balance_cells(ADBMS_bms_t *bms, float min_voltage, float min_delta) {
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
        for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
            for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
                bms->modules[i].is_discharging[j] = false;
            }
        }

        return;
    } else {
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
}

void adbms_balance_and_update_regb(ADBMS_bms_t *bms, float min_voltage, float min_delta) {
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

void adbms_read_cells(ADBMS_bms_t *bms) {
    float *cell_voltage_ptrs[ADBMS_MODULE_COUNT]        = {0};
    int16_t *cell_voltages_raw_ptrs[ADBMS_MODULE_COUNT] = {0};
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        cell_voltage_ptrs[i]      = bms->modules[i].cell_voltages;
        cell_voltages_raw_ptrs[i] = bms->modules[i].cell_voltages_raw;
    }

    if (!adbms6380_read_cell_voltages(bms->spi,
                                      &bms->tx_strbuf,
                                      bms->rx_buf,
                                      cell_voltage_ptrs,
                                      cell_voltages_raw_ptrs,
                                      ADBMS_MODULE_COUNT)) {
        bms->state   = ADBMS_STATE_IDLE;
        bms->err_spi = true;
        return;
    }

    bms->max_voltage = cell_voltage_ptrs[0][0];
    bms->min_voltage = cell_voltage_ptrs[0][0];
    bms->sum_voltage = 0.0f;
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        bms->modules[i].max_voltage = cell_voltage_ptrs[i][0];
        bms->modules[i].min_voltage = cell_voltage_ptrs[i][0];
        bms->modules[i].sum_voltage = 0.0f;
        for (size_t j = 0; j < ADBMS6380_CELL_COUNT; j++) {
            float cell_v = cell_voltage_ptrs[i][j];
            if (cell_v > bms->modules[i].max_voltage) {
                bms->modules[i].max_voltage = cell_v;
            }
            if (cell_v < bms->modules[i].min_voltage) {
                bms->modules[i].min_voltage = cell_v;
            }
            bms->modules[i].sum_voltage += cell_v;
        }
        bms->modules[i].avg_voltage = bms->modules[i].sum_voltage / ADBMS6380_CELL_COUNT;

        if (bms->modules[i].max_voltage > bms->max_voltage) {
            bms->max_voltage = bms->modules[i].max_voltage;
        }
        if (bms->modules[i].min_voltage < bms->min_voltage) {
            bms->min_voltage = bms->modules[i].min_voltage;
        }
        bms->sum_voltage += bms->modules[i].sum_voltage;
    }
    bms->avg_voltage = bms->sum_voltage / (ADBMS_MODULE_COUNT * ADBMS6380_CELL_COUNT);
}

void adbms_read_therms(ADBMS_bms_t *bms) {
    float *gpio_voltage_ptrs[ADBMS_MODULE_COUNT] = {0};
    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        gpio_voltage_ptrs[i] = bms->modules[i].therms_voltages;
    }

    // Start ADAX
    strbuf_clear(&bms->tx_strbuf);
    uint8_t adax_cmd[2] = {0};
    adbms6380_adax(adax_cmd, ADBMS_ADAX_OW, ADBMS_ADAX_PUP, ADBMS_ADAX_CH);
    adbms6380_prepare_command(&bms->tx_strbuf, adax_cmd);
    adbms6380_set_cs_low(bms->spi);
    if (!PHAL_SPI_transfer_noDMA(bms->spi, bms->tx_strbuf.data, bms->tx_strbuf.length, 0, NULL)) {
        adbms6380_set_cs_high(bms->spi);
        bms->err_spi = true;
        return;
    }
    adbms6380_set_cs_high(bms->spi);

    if (!adbms6380_read_gpio_voltages(bms->spi,
                                      &bms->tx_strbuf,
                                      bms->rx_buf,
                                      gpio_voltage_ptrs,
                                      ADBMS_MODULE_COUNT)) {
        bms->state   = ADBMS_STATE_IDLE;
        bms->err_spi = true;
        return;
    }

    for (size_t i = 0; i < ADBMS_MODULE_COUNT; i++) {
        for (size_t j = 0; j < ADBMS6380_GPIO_COUNT; j++) {
            bms->modules[i].therms_resistances[j] =
                thermistor_R_to_T(bms->modules[i].therms_voltages[j]);
        }
    }
}

void adbms_periodic(ADBMS_bms_t *bms, float min_voltage_for_balance, float min_delta_for_balance) {
    adbms6380_wake(bms->spi, ADBMS_MODULE_COUNT);
    switch (bms->state) {
        case ADBMS_STATE_IDLE: {
            adbms_connect(bms);
            break;
        }
        case ADBMS_STATE_CONNECTED: {
            adbms_read_cells(bms);
            adbms_read_therms(bms);
            adbms_balance_and_update_regb(bms, min_voltage_for_balance, min_delta_for_balance);
            break;
        }
    }
}