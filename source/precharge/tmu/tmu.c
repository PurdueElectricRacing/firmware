#include "tmu.h"
#include "common/phal_L4//gpio/gpio.h"
#include "main.h"
#include "common/common_defs/common_defs.h"
#include "can_parse.h"
#include "common/faults/faults.h"


extern q_handle_t q_tx_can;
uint8_t i;
uint8_t num_bad1, num_bad2, num_bad3, num_bad4;
bool overtemp = false;

//Function defs not needed by any other file
void resistance_to_temp(float resistance, uint16_t *temp);
static double native_log_computation(const double n);


bool initTMU(tmu_handle_t *tmu) {
   uint8_t spi_tx_buffer[3] = {0};
   uint8_t spi_rx_buffer[3] = {0};
   while(PHAL_SPI_busy(tmu->spi))
       ;
   spi_tx_buffer[0] = TMU_PRODID_ADDR;
   PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 3, spi_rx_buffer);
   while (PHAL_SPI_busy(tmu->spi))
       ;
   if (((spi_rx_buffer[0] | spi_rx_buffer[1]) == 0) && (spi_rx_buffer[2] == 129)) {
        i = 0;
       return true;
   }
   num_bad1 = 0;
   num_bad2 = 0;
   num_bad3 = 0;
   num_bad4 = 2;
    PHAL_writeGPIO(MUX_A_NON_ISO_Port, MUX_A_NON_ISO_Pin, 0);
    PHAL_writeGPIO(MUX_B_NON_ISO_Port, MUX_B_NON_ISO_Pin, 0);
    PHAL_writeGPIO(MUX_C_NON_ISO_Port, MUX_C_NON_ISO_Pin, 0);
    PHAL_writeGPIO(MUX_D_NON_ISO_Port, MUX_D_NON_ISO_Pin, 0);
   return false;
}


void readTemps(tmu_handle_t *tmu) {
   uint8_t spi_tx_buffer[11] = {0};
   uint8_t spi_rx_buffer[11] = {0};


    //Read the therm value
    while(PHAL_SPI_busy(tmu->spi))
        ;
    spi_tx_buffer[0] = (TMU_FILTERED_DATA_CMD << 2) | (0x1);
    PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 11, spi_rx_buffer);
    while (PHAL_SPI_busy(tmu->spi))
        ;
    tmu->tmu1_volts = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[1] << 8) | spi_rx_buffer[2])));
    tmu->tmu2_volts = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[3] << 8) | spi_rx_buffer[4])));
    tmu->tmu3_volts = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[5] << 8) | spi_rx_buffer[6])));
    tmu->tmu4_volts = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[7] << 8) | spi_rx_buffer[8])));

    if (tmu->tmu1_volts > 1.40)
        num_bad1++;
    if (tmu->tmu2_volts > 1.40)
        num_bad2++;
    if (tmu->tmu3_volts > 1.40)
        num_bad3++;
    if (tmu->tmu4_volts > 1.40)
        num_bad4++;

    //Resistance of Thermistor (Bottom resistor)
    float tmu1_r2 = (tmu->tmu1_volts * R1) / (TMU_VIN - tmu->tmu1_volts);
    float tmu2_r2 = (tmu->tmu2_volts * R1) / (TMU_VIN - tmu->tmu2_volts);
    float tmu3_r2 = (tmu->tmu3_volts * R1) / (TMU_VIN - tmu->tmu3_volts);
    float tmu4_r2 = (tmu->tmu4_volts * R1) / (TMU_VIN - tmu->tmu4_volts);

    //Subtract old value at this index from average
    tmu->tmu1_avg -= tmu->tmu1[i];
    tmu->tmu2_avg -= tmu->tmu2[i];
    tmu->tmu3_avg -= tmu->tmu3[i];
    tmu->tmu4_avg -= tmu->tmu4[i];

    if (i != 0) {

    }

    if (i != 1 && i != 2) {
        resistance_to_temp(tmu4_r2, &tmu->tmu4[i]);
        tmu->tmu4_max = MAX(tmu->tmu4_max, tmu->tmu4[i]);
        tmu->tmu4_min = MIN(tmu->tmu4_min, tmu->tmu4[i]);
        tmu->tmu4_avg += tmu->tmu4[i];
    }

    if (i != 8) {
        if (i != 3) {
            resistance_to_temp(tmu2_r2, &tmu->tmu2[i]);
            tmu->tmu2_max = MAX(tmu->tmu2_max, tmu->tmu2[i]);
            tmu->tmu2_min = MIN(tmu->tmu2_min, tmu->tmu2[i]);
            tmu->tmu2_avg += tmu->tmu2[i];
        }
    }
    resistance_to_temp(tmu3_r2, &tmu->tmu3[i]);
    tmu->tmu3_max = MAX(tmu->tmu3_max, tmu->tmu3[i]);
    tmu->tmu3_min = MIN(tmu->tmu3_min, tmu->tmu3[i]);
    tmu->tmu3_avg += tmu->tmu3[i];
    if (i != 0) {
        resistance_to_temp(tmu1_r2, &tmu->tmu1[i]);
        tmu->tmu1_max = MAX(tmu->tmu1_max, tmu->tmu1[i]);
        tmu->tmu1_min = MIN(tmu->tmu1_min, tmu->tmu1[i]);
        tmu->tmu1_avg += tmu->tmu1[i];
    }

    //Convert the read resistance to temperature
    // resistance_to_temp(tmu1_r2, &tmu->tmu1[i]);
    // if (i != 3 && i != 8)
    //     resistance_to_temp(tmu2_r2, &tmu->tmu2[i]);
    // if (i != 0 && i != 8)
    //     resistance_to_temp(tmu3_r2, &tmu->tmu3[i]);
    // if (i != 1 && i != 2)
    //     resistance_to_temp(tmu4_r2, &tmu->tmu4[i]);

    //Check whether this is a new maximum value
    // tmu->tmu1_max = MAX(tmu->tmu1_max, tmu->tmu1[i]);
    // if (i != 3 && i != 8)
    //     tmu->tmu2_max = MAX(tmu->tmu2_max, tmu->tmu2[i]);
    // if (i != 0 && i != 8)
    //     tmu->tmu3_max = MAX(tmu->tmu3_max, tmu->tmu3[i]);
    // if (i != 1 && i != 2)
    //     tmu->tmu4_max = MAX(tmu->tmu4_max, tmu->tmu4[i]);

    // tmu->tmu1_min = MIN(tmu->tmu1_min, tmu->tmu1[i]);
    // if (i != 3 && i != 8)
    //     tmu->tmu2_min = MIN(tmu->tmu2_min, tmu->tmu2[i]);
    // if (i != 0& i != 8)
    //     tmu->tmu3_min = MIN(tmu->tmu3_min, tmu->tmu3[i]);
    // if (i != 1 && i != 2)
    //     tmu->tmu4_min = MIN(tmu->tmu4_min, tmu->tmu4[i]);


    //Add new thermistor value to moving average
    // tmu->tmu1_avg += tmu->tmu1[i];
    // if (i != 3 && i != 8)
    //     tmu->tmu2_avg += tmu->tmu2[i];
    // if (i != 0 && i != 8)
    //     tmu->tmu3_avg += tmu->tmu3[i];
    // if (i != 1 && i != 2)
    //     tmu->tmu4_avg += tmu->tmu4[i];

    // tmu->tmu1[i] = tmu->tmu1_volts * 100;
    // if (i != 3 && i != 8)
    //     tmu->tmu2[i] = tmu->tmu2_volts * 100;
    // if (i != 0 && i != 8)
    //     tmu->tmu3[i] = tmu->tmu3_volts * 100;
    // if (i != 1 && i != 2)
    //     tmu->tmu4[i] = tmu->tmu4_volts * 100;




    SEND_MOD_CELL_TEMP_AVG(q_tx_can, (tmu->tmu1_avg / NUM_THERM), (tmu->tmu2_avg / (NUM_THERM - 2)), (tmu->tmu3_avg / (NUM_THERM)), (tmu->tmu4_avg / (NUM_THERM - 2)));
    SEND_RAW_CELL_TEMP(q_tx_can, i, tmu->tmu1[i], tmu->tmu2[i], tmu->tmu3[i], tmu->tmu4[i]);

    if ((i + 1) < NUM_THERM) {
        i++;
            //Select the MUX pin on each TMU board to read the thermistor value
    }
    else {
        SEND_MOD_CELL_TEMP_MAX(q_tx_can, tmu->tmu1_max, tmu->tmu2_max, tmu->tmu3_max, tmu->tmu4_max);
        SEND_MOD_CELL_TEMP_MIN(q_tx_can, tmu->tmu1_min, tmu->tmu2_min, tmu->tmu3_min, tmu->tmu4_min);
        uint16_t max_temp = MAX(MAX(tmu->tmu1_max, tmu->tmu2_max), MAX(tmu->tmu3_max, tmu->tmu4_max));
        uint16_t min_temp =  MIN(MIN(tmu->tmu1_min, tmu->tmu2_min), MIN(tmu->tmu3_min, tmu->tmu4_min));
        SEND_MAX_CELL_TEMP(q_tx_can, max_temp);
        SEND_NUM_THEM_BAD(q_tx_can, num_bad1, num_bad2, num_bad3, num_bad4);
        num_bad1 = 0;
        num_bad2 = 0;
        num_bad3 = 0;
        num_bad4 = 2;
        setFault(ID_PACK_TEMP_FAULT, max_temp);
        setFault(ID_PACK_TEMP_EXCEEDED_FAULT, max_temp);
        // setFault(ID_MIN_PACK_TEMP_FAULT, min_temp);

        i = 0;
        tmu->tmu1_max = 0;
        tmu->tmu2_max = 0;
        tmu->tmu3_max = 0;
        tmu->tmu4_max = 0;
        tmu->tmu1_min = 0xFFFF;
        tmu->tmu2_min = 0xFFFF;
        tmu->tmu3_min = 0xFFFF;
        tmu->tmu4_min = 0xFFFF;
    }


    PHAL_writeGPIO(MUX_A_NON_ISO_Port, MUX_A_NON_ISO_Pin, (i & 0x1));
    PHAL_writeGPIO(MUX_B_NON_ISO_Port, MUX_B_NON_ISO_Pin, (i & 0x2));
    PHAL_writeGPIO(MUX_C_NON_ISO_Port, MUX_C_NON_ISO_Pin, (i & 0x4));
    PHAL_writeGPIO(MUX_D_NON_ISO_Port, MUX_D_NON_ISO_Pin, (i & 0x8));

}


void resistance_to_temp(float resistance, uint16_t *temp) {
   float resistance_scaled = resistance / R25;
    double native_log = native_log_computation(resistance_scaled);
   if (COMP(resistance_scaled, LOW_RANGE_MIN, LOW_RANGE_MAX)) {
       *temp = (uint16_t) 100 * ((1 / (LOW_RANGE_A + ((LOW_RANGE_B * native_log)) + (LOW_RANGE_C * native_log * native_log)
               + (LOW_RANGE_D * native_log * native_log * native_log)) - 273.5));
   }
   else if (COMP(resistance_scaled, HIGH_RANGE_MIN, HIGH_RANGE_MAX)) {
       *temp = (uint16_t) 100 * ((1 / (HIGH_RANGE_A + ((HIGH_RANGE_B * native_log) + (HIGH_RANGE_C * native_log * native_log)
               + (HIGH_RANGE_D * native_log * native_log * native_log))) - 273.5));
   }
   else {
       *temp = 0;
   }
}


// https://stackoverflow.com/questions/9800636/calculating-natural-logarithm-and-exponent-by-core-c-for-embedded-system
//(From PDU Code)


double native_log_computation(const double n) {
   // Basic logarithm computation.
   static const double euler = 2.7182818284590452354 ;
   unsigned a = 0, d;
   double b, c, e, f;
   if (n > 0) {
       for (c = n < 1 ? 1 / n : n; (c /= euler) > 1; ++a);
       c = 1 / (c * euler - 1), c = c + c + 1, f = c * c, b = 0;
       for (d = 1, c /= 2; e = b, b += 1 / (d * c), b - e/* > 0.0000001 */;)
           d += 2, c *= f;
   } else b = (n == 0) / 0.;
   return n < 1 ? -(a + b) : a + b;
}
