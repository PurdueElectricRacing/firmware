#include "temp.h"

int checkTempMaster(void)
{

}

void procTemps(void)
{
    uint8_t i, error_temp, error_dt;
    float   voltage;
    double  temperature;

    static uint16_t temp_last[TEMP_MAX];

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