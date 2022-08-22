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
    if (!PHAL_readGPIO(GPIOB, 6) || !PHAL_readGPIO(GPIOB, 7)) {
        bms.error |= 1U << E_TEMP;
    } else {
        bms.error &= ~(1U << E_TEMP);
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