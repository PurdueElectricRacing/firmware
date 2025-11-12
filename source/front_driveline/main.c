/*System Includes*/

#include "source/daq/main.h"

#include "common/daq/can_parse_base.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/queue/queue.h"
#include "common_defs.h"
#include "external/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include/stm32f407xx.h"
#include "source/front_driveline/can/can_parse.h"

/* Module Includes */
#include <stdint.h>

#include "can_parse.h"
#include "main.h"

/* Pin Intialization */

GPIOInitConfig_t gpio_config[] = {
    // Shock Pots
    GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
    GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),

    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,

    //Magnometer i2c

    //IR Temperature
    GPIO_INIT_ANALOG(BREAK_TEMP_L_GPIO_Port, BREAK_TEMP_L_Pin),
    GPIO_INIT_ANALOG(BREAK_TEMP_R_GPIO_Port, BREAK_TEMP_R_Pin),

    //Load Cells
    GPIO_INIT_ANALOG(LOAD_FL_GPIO_Port, LOAD_FL_Pin),
    GPIO_INIT_ANALOG(LOAD_FR_GPIO_Port, LOAD_FR_Pin)

};

/* ADC Configuration */

volatile raw_adc_values_t raw_adc_values;

ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_2,
    .resolution = ADC_RES_12_BIT,
    .data_align = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode = ADC_DMA_CIRCULAR,
    .adc_number = 1,
};

ADCChannelConfig_t adc_channel_config[] = {

    {.channel = SHOCK_POT_L_ADC_CH, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = SHOCK_POT_R_ADC_CH, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LOAD_FL_ADC_CH, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LOAD_FR_ADC_CH, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BREAK_TEMP_L_ADC_CH, .rank = 9, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BREAK_TEMP_R_ADC_CH, .rank = 10, .sampling_time = ADC_CHN_SMP_CYCLES_480},

};

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.load_left), 0b01);



/* Clock Configuration */
#define TargetCoreClockrateHz 144000000
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

ClockRateConfig_t clock_config =
    {
        .clock_source = CLOCK_SOURCE_HSI,
        .use_pll = true,
        .pll_src = PLL_SRC_HSI16,
        .vco_output_rate_target_hz = 288000000,
        .system_clock_target_hz = TargetCoreClockrateHz,
        .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
        .apb1_clock_target_hz = (TargetCoreClockrateHz / 4),
        .apb2_clock_target_hz = (TargetCoreClockrateHz / 4),
};

// Communication queues
q_handle_t q_tx_usart;

int main(void) {
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);
    void sendShockpots();
    void sendLoadCells();
    void sendBreakTemps();

    schedStart();

    return 0;
}

void can_worker_task() {
    // Process all received CAN messages
    while (qIsEmpty(&q_rx_can) == false) {
        canRxUpdate();
    }

    // Drain all CAN transmit queues
    canTxUpdate();
}

/**
 * @brief Processes and sends shock potentiometer readings
 *
 * Converts raw ADC values from left and right shock potentiometers into parsed displacement values
 * and sends them through CAN bus. Values are scaled linearly and adjusted for droop.
 */
int16_t shock_l_displacement;
int16_t shock_r_displacement;

// convert voltage to kg - yash
void sendLoadCells() {
    //Loading Data from struct
    uint16_t load_l = raw_adc_values.load_left;
    uint16_t load_r = raw_adc_values.load_right;

    //Calculation is (ADC_reading)/5V * calibrated weight
    float load_l_kg = (load_l / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 
    float load_r_kg = (load_r / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 

    SEND_LOAD_SENSOR_READINGS_DRIVELINE(load_l_kg,load_r_kg);
}

// convert voltage to celsius?
void sendBreakTemps() {
}

void sendShockpots() {
    uint16_t shock_l = raw_adc_values.shock_left;
    uint16_t shock_r = raw_adc_values.shock_right;

    //left calculation variables

    //compute total voltage range
    float shock_l_range = (float)POT_VOLT_MIN_L - POT_VOLT_MAX_L;

    //get percentage
    float shock_l_percent = shock_l / shock_l_range;

    //scale percentage to distance
    float shock_l_scaled = shock_l_percent * POT_MAX_DIST;

    //adjust bc of droop
    float shock_l_adjusted = POT_MAX_DIST - shock_l_scaled - POT_DIST_DROOP_L;

    //convert to int and switch sign
    shock_l_displacement = (int16_t)(-shock_l_adjusted);

    //right calculation variables
    float shock_r_range = (float)POT_VOLT_MIN_R - POT_VOLT_MAX_R;
    float shock_r_percent = shock_r / shock_r_range;
    float shock_r_scaled = shock_r_percent * POT_MAX_DIST;
    float shock_r_adjusted = POT_MAX_DIST - shock_r_scaled - POT_DIST_DROOP_R;
    shock_r_displacement = (int16_t)(-shock_r_adjusted);

    SEND_SHOCK_FRONT_DRIVELINE(shock_l_displacement, shock_r_displacement);
}

void preflightChecks(void) {
    static uint8_t state;

    switch (state++) {
        case 0:
            /* VCAN */
            if (false == PHAL_initCAN(CAN1, false, VCAN_BPS)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 1:
            /* MCAN */
            if (false == PHAL_initCAN(CAN2, false, TCAN_BPS)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN2_RX0_IRQn);
            break;
        case 2:
            /* ADC */
            if (false == PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
                HardFault_Handler();
            }
            if (false == PHAL_initDMA(&adc_dma_config)) {
                HardFault_Handler();
            }
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
            break;
        case 3:
            break;
        default:
            registerPreflightComplete(1);
            state = 255; /* prevent wrap around */
    }
}