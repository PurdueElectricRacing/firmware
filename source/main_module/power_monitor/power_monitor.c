/**
 * @file power_monitor.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Monitor voltage and current present on the board
 * @version 0.1
 * @date 2023-02-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "power_monitor.h"
#include "car.h"

static uint16_t calcCurrent(uint16_t adc_raw);
static uint16_t calcVoltage(uint16_t adc_raw, uint16_t r1, uint16_t r2, uint16_t cal);

PowerMonitor_t power_monitor;

void initPowerMonitor()
{
    power_monitor = (PowerMonitor_t) {0};
    // Interrupt for full transfer (testing frequency of ADC)
    // DMA2_Channel3->CCR |= DMA_CCR_TCIE;
    // NVIC_EnableIRQ(DMA2_Channel3_IRQn);
}

void updatePowerMonitor()
{
    power_monitor.lv_24_v_sense_mV  = calcVoltage(adc_readings.lv_24_v_sense, LV_24V_R1, LV_24V_R2, LV_24V_CAL);
    power_monitor.lv_24_i_sense_mA  = calcCurrent(adc_readings.lv_24_i_sense);
    power_monitor.lv_12_v_sense_mV  = calcVoltage(adc_readings.lv_12_v_sense, LV_12V_R1, LV_12V_R2, LV_12V_CAL);
    power_monitor.lv_5_v_sense_mV   = calcVoltage(adc_readings.lv_5_v_sense, LV_5V_R1, LV_5V_R2, LV_5V_CAL);
    power_monitor.lv_5_i_sense_mA   = calcCurrent(adc_readings.lv_5_i_sense);
    power_monitor.lv_3v3_v_sense_mV = calcVoltage(adc_readings.lv_3v3_v_sense, LV_3V3_R1, LV_3V3_R2, LV_3V3_CAL);

    // TODO: add in faults

}

// void DMA2_Channel3_IRQHandler()
// {
//     if (DMA2->ISR & DMA_ISR_TCIF3)
//     {
//         PHAL_toggleGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin);
//     }
//     DMA2->IFCR = DMA_IFCR_CGIF3;
// }

/**
 * @brief Calculates the low voltage system current
 *        draw based on the output of a current
 *        sense amplifier
 * 
 * @param adc_raw 12-bit adc reading
 * @return uint16_t current in mA
 */
static uint16_t calcCurrent(uint16_t adc_raw)
{
    uint32_t tmp;
    // V_out = adc_raw * adc_v_ref / adc_max
    tmp = ((uint32_t) adc_raw * ADC_REF_mV) / 0xFFFUL;
    // V_out = (I_load * R_sense * gain) + V_ref (0)
    // I_load = (V_out / R_sense / gain)
    return (uint16_t) tmp * 1000 / LV_I_SENSE_GAIN / LV_I_SENSE_R;
}

/**
 * @brief Calculate the rail voltage given the adc 
 *        reading from a voltage divider
 * 
 * @param adc_raw 12-bit adc reading
 * @param r1      Top resistor (Ohms)
 * @param r2      Bottom resistor (Ohms)
 * @param cal     Constant calibration factor
 * @return uint16_t V_out (mV)
 */
static uint16_t calcVoltage(uint16_t adc_raw, uint16_t r1, uint16_t r2, uint16_t cal)
{
    uint32_t tmp;
    // V_out = adc_raw * adc_v_ref / adc_max
    tmp = ((uint32_t) adc_raw * ((ADC_REF_mV * (uint32_t) cal) / 1000)) / 0xFFFUL;
    if (r2 == 0) return tmp;
    // V_out = V_in * (r2 / (r1 + r2))
    // V_in = (V_out * (r1 + r2)) / r2
    return (uint16_t) (tmp * ((uint32_t) r1 + (uint32_t) r2) / r2);
}