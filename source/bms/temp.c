#include "temp.h"

int checkTempMaster(uint8_t addr)
{
    uint8_t buff;

    // Write cfg
    PHAL_I2C_gen_start(I2C1, addr, 1, PHAL_I2C_MODE_TX);
    PHAL_I2C_write(I2C1, addr);
    PHAL_I2C_gen_stop(I2C1);

    // Read cfg
    PHAL_I2C_gen_start(I2C1, addr, 1, PHAL_I2C_MODE_TX);
    PHAL_I2C_read(I2C1, &buff);
    PHAL_I2C_gen_stop(I2C1);

    // If read data is bad, we're not master
    if (buff != addr)
    {
        return 0;
    }

    // Reset to pull temps next read
    PHAL_I2C_gen_start(I2C1, addr, 1, PHAL_I2C_MODE_TX);
    PHAL_I2C_write(I2C1, 0);
    PHAL_I2C_gen_stop(I2C1);

    bms.temp_master = 1;

    return 1;
}

void tempTask(void)
{
    uint8_t  i;
    uint8_t  ret = 0;
    uint8_t  buff[TEMP_MAX];
    uint16_t addr;
    static uint8_t side;

    // Clear buffer for debug
    for (i = 0; i < TEMP_MAX; i++) {
        buff[i] = 0;
    }

    // Set addr based on side
    if (!side) {
        addr = TEMP_ID1;
    } else {
        addr = TEMP_ID2;
    }

    // Get values from set device
    ret += PHAL_I2C_gen_start(I2C1, addr << 1, bms.temp_count, PHAL_I2C_MODE_RX);

    if (ret) {
        ret = PHAL_I2C_read_multi(I2C1, &buff[side ? bms.temp_count : 0], bms.temp_count);

        if (ret){
            PHAL_I2C_gen_stop(I2C1);
        }
    }

    // Store raw values
    for (i = (side ? bms.temp_count / 2 : 0); i < (side ? bms.temp_count : bms.temp_count / 2); i++)
    {
        bms.cells.chan_temps_raw[i] = (((uint16_t) buff[i * 2 + 1]) << 8) | (uint16_t) buff[(i * 2)];
    }

    // Invert side for next run
    side = !side;

    // Check if we had a transmission error
    if (ret != 3) {
        bms.error |= 1U << E_I2C_CONN;
    } else {
        bms.error &= ~(1U << E_I2C_CONN);
    }
}

void procTemps(void)
{
    uint8_t i, error_temp, error_dt;
    float   dt_new;
    float   ddt;
    float   voltage;
    double  temperature;

    static uint16_t temp_last[TEMP_MAX];
    static float    dt[TEMP_MAX];

    error_temp = error_dt = 0;

    for (i = 0; i < bms.temp_count; i++)
    {
        voltage = VOLTAGE_REF * ((float) bms.cells.chan_temps_raw[i]) / 0xFFFF;
        voltage = (voltage * THERM_RESIST) / (VOLTAGE_TOP - voltage);
        temperature = B_VALUE / log(voltage / R_INF_3977) - KELVIN_2_CELSIUS;

        bms.cells.chan_temps_conv[i] = temperature * 10;

        if (bms.cells.chan_temps_conv[i] >= TEMP_MAX_C)
        {
            error_temp = 1;
        }

        dt_new = (bms.cells.chan_temps_conv[i] - temp_last[i]) / 0.015f;
        temp_last[i] = bms.cells.chan_temps_conv[i];
        ddt = (dt_new - dt[i]) / 0.015f;

        if (dt_new > DT_CRIT)
        {
            error_temp = 1;
        }
    }

    if (error_temp)
    {
        bms.error |= (1U << 4);
    }
    else
    {
        bms.error &= ~(1U << 4);
    }
}