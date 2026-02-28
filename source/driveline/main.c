/**
 * @file main.c
 * @brief "Driveline" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

/* System Includes */
#include "common/can_library/generated/DRIVELINE.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/adc.h"
#include "common/phal/dma.h"
#include "common/freertos/freertos.h"

/* Module Includes */
#include "pin_defs.h"
#include "config.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13,

     // Shock Pots
    GPIO_INIT_ANALOG(SHOCKPOT_LEFT_GPIO_PORT , SHOCKPOT_LEFT_GPIO_PIN),
    GPIO_INIT_ANALOG(SHOCKPOT_RIGHT_GPIO_PORT, SHOCKPOT_RIGHT_GPIO_PIN),

    //Load Cells
    GPIO_INIT_ANALOG(LOAD_FL_GPIO_Port, LOAD_FL_Pin),
    GPIO_INIT_ANALOG(LOAD_FR_GPIO_Port, LOAD_FR_Pin)


};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI, // todo change to HSE
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC1,
};
ADCChannelConfig_t adc_channel_config[] = {
    {.channel = SHOCKPOT_LEFT_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = SHOCKPOT_RIGHT_ADC_CHNL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LOAD_FL_ADC_CH, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LOAD_FR_ADC_CH, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
     {.channel = LOAD_RL_ADC_CH, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LOAD_RR_ADC_CH, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE_TEMP_L_ADC_CH, .rank = 9, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE_TEMP_R_ADC_CH, .rank = 10, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

// note: this struct is the target of the DMA controller,
// it's layout must match the order and size of the ADC channels in adc_channel_config
// additonally, it must have no padding and members must be uint16_t to match the ADC resolution and data alignment
typedef struct {
    uint16_t shock_l;
    uint16_t shock_r;
    uint16_t load_left;
    uint16_t load_right;
    uint16_t brake_temp_left;
    uint16_t brake_temp_right;
} raw_adc_values_t;
volatile raw_adc_values_t raw_adc_values;
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc_values, sizeof(raw_adc_values) / sizeof(uint16_t), 0b01);

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern void HardFault_Handler();
void shockpot_thread();
void loadcell_thread();
void braketemp_thread();


defineThreadStack(shockpot_thread, 100, osPriorityNormal, 512);
defineThreadStack(loadcell_thread, 100, osPriorityNormal, 512);
defineThreadStack(braketemp_thread, 100, osPriorityNormal, 512);

int main(void) {
    // Hardware Initilization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc_dma_config)) {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(&adc_config);

    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    createThread(shockpot_thread);

    // no way home
    osKernelStart();

    return 0;
}

// Both driveline nodes

void shockpot_thread() {
    float shock_l_parsed = -1.0 * ((POT_MAX_DIST - ((raw_adc_values.shock_l / (POT_VOLT_MIN_L - POT_VOLT_MAX_L)) * POT_MAX_DIST)) - POT_DIST_DROOP_L);
    float shock_r_parsed = -1.0 * ((POT_MAX_DIST - ((raw_adc_values.shock_r / (POT_VOLT_MIN_R - POT_VOLT_MAX_R)) * POT_MAX_DIST)) - POT_DIST_DROOP_R);

    int16_t shock_l_scaled = (int16_t)(shock_l_parsed * PACK_COEFF_FRONT_SHOCKPOTS_LEFT);
    int16_t shock_r_scaled = (int16_t)(shock_r_parsed * PACK_COEFF_FRONT_SHOCKPOTS_RIGHT);

    #ifdef SEND_SHOCKPOTS
    SEND_SHOCKPOTS(shock_l_scaled, shock_r_scaled);
    #endif
}

float load_l_kg;
float load_r_kg;
// convert voltage to kg - yash
void loadcell_thread() {
    //Loading Data from struct
    uint16_t load_l = raw_adc_values.load_left;
    uint16_t load_r = raw_adc_values.load_right;

    //Calculation is (ADC_reading)/5V * calibrated weight
    load_l_kg = (load_l / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 
    load_r_kg = (load_r / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 

    SEND_LOAD(load_l_kg,load_r_kg);
}

// convert voltage to celsius?
int16_t brake_temp_l;
int16_t brake_temp_r;

void braketemp_thread() {

    //get raw voltage from brake temp sensors
    double brake_temp_v_l = raw_adc_values.brake_temp_left;
    double brake_temp_v_r = raw_adc_values.brake_temp_right;
    brake_temp_l = (int16_t) (brake_temp_v_l - 0.5)/0.005;
    brake_temp_r = (int16_t) (brake_temp_v_r - 0.5)/0.005;
    SEND_BRAKE_TEMPS(brake_temp_l, brake_temp_r);

}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL = 0;
    ERROR_LED_PORT->BSRR = ERROR_LED_PIN;
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
