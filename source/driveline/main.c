/**
 * @file main.c
 * @brief "Driveline" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

/* System Includes */
#include <stdint.h>
#include "common/can_library/generated/DRIVELINE.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/adc.h"
#include "common/phal/dma.h"
#include "common/freertos/freertos.h"

/* Module Includes */
#include "common/phal_G4/adc/adc.h"
#include "common/phal_G4/dma/dma.h"
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
    GPIO_INIT_ANALOG(LOAD_FR_GPIO_Port, LOAD_FR_Pin),
    GPIO_INIT_ANALOG(LOAD_RL_GPIO_Port, LOAD_RL_Pin),
    GPIO_INIT_ANALOG(LOAD_RR_GPIO_Port, LOAD_RR_Pin),

    GPIO_INIT_ANALOG(BRAKE_TEMP_L_GPIO_Port, BRAKE_TEMP_L_Pin),
    GPIO_INIT_ANALOG(BRAKE_TEMP_R_GPIO_Port, BRAKE_TEMP_R_Pin),

    GPIO_INIT_ANALOG(OIL_TEMP_L_GPIO_Port, OIL_TEMP_L_Pin),
    GPIO_INIT_ANALOG(OIL_TEMP_R_GPIO_Port, OIL_TEMP_R_Pin),

    GPIO_INIT_ANALOG(BRAKE_PRESSURE_R_GPIO_Port, BRAKE_PRESSURE_R_Pin),
    GPIO_INIT_ANALOG(BRAKE_PRESSURE_L_GPIO_Port, BRAKE_PRESSURE_L_Pin),

    GPIO_INIT_ANALOG(WATER_TEMP_FL_GPIO_Port, WATER_TEMP_FL_Pin),
    GPIO_INIT_ANALOG(WATER_TEMP_RL_GPIO_Port, WATER_TEMP_RL_Pin),
    GPIO_INIT_ANALOG(WATER_TEMP_FR_GPIO_Port, WATER_TEMP_FR_Pin),
    GPIO_INIT_ANALOG(WATER_TEMP_RR_GPIO_Port, WATER_TEMP_RR_Pin),

    GPIO_INIT_ANALOG(AMB_TEMP_GPIO_Port, AMB_TEMP_Pin),

};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI,
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* ADC Configuration */
ADCInitConfig_t adc1_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC1,
};

ADCInitConfig_t adc2_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC2,
};

ADCInitConfig_t adc3_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC3,
};


ADCInitConfig_t adc4_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC4,
};

ADCInitConfig_t adc5_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC5,
};

// ADC 1
ADCChannelConfig_t adc1_channel_config[] = {
 {.channel = OIL_TEMP_L_ADC_CH, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
 {.channel = BRAKE_PRESSURE_R_ADC_CH, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
}; 
typedef struct {
    uint16_t oil_temp_left;
    uint16_t brake_pressure_right;
} raw_adc1_values_t; 
volatile raw_adc1_values_t raw_adc1_values;
dma_init_t adc1_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc1_values, sizeof(raw_adc1_values) / sizeof(uint16_t), 0b01);

// ADC 2
ADCChannelConfig_t adc2_channel_config[] = {
{.channel = BRAKE_PRESSURE_L_ADC_CH, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},    
{.channel = WATER_TEMP_FL_ADC_CH, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = WATER_TEMP_RL_ADC_CH, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = WATER_TEMP_FR_ADC_CH, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = BRAKE_TEMP_R_ADC_CH, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = BRAKE_TEMP_L_ADC_CH, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = LOAD_RL_ADC_CH, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
{.channel = OIL_TEMP_R_ADC_CH, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};
typedef struct {
    uint16_t brake_pressure_left;
    uint16_t water_temp_fl;
    uint16_t water_temp_rl;
    uint16_t water_temp_fr;
    uint16_t brake_temp_right;
    uint16_t brake_temp_left;
    uint16_t load_rl;
    uint16_t oil_temp_right;

} raw_adc2_values_t; 
volatile raw_adc2_values_t raw_adc2_values;
dma_init_t adc2_dma_config = ADC2_DMA_CONT_CONFIG((uint32_t)&raw_adc2_values, sizeof(raw_adc2_values) / sizeof(uint16_t), 0b01);

// ADC 3
ADCChannelConfig_t adc3_channel_config[] = {
 {.channel = SHOCKPOT_LEFT_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
 {.channel = WATER_TEMP_RR_ADC_CH, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
 {.channel = AMB_TEMP_ADC_CH, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
}; 
typedef struct {
    uint16_t shock_l;
    uint16_t water_temp_rr;
    uint16_t amb_temp;
   
} raw_adc3_values_t;
volatile raw_adc3_values_t raw_adc3_values;
dma_init_t adc3_dma_config = ADC3_DMA_CONT_CONFIG((uint32_t)&raw_adc3_values, sizeof(raw_adc3_values) / sizeof(uint16_t), 0b01);

// ADC 4
ADCChannelConfig_t adc4_channel_config[] = {
 {.channel = LOAD_FL_ADC_CH, .rank =1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
 {.channel = SHOCKPOT_RIGHT_ADC_CHNL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480}
};
typedef struct {
    uint16_t load_fl;
    uint16_t shock_r;
} raw_adc4_values_t;
volatile raw_adc4_values_t raw_adc4_values;
dma_init_t adc4_dma_config = ADC4_DMA_CONT_CONFIG((uint32_t)&raw_adc4_values, sizeof(raw_adc4_values) / sizeof(uint16_t), 0b01);

// ADC 5
ADCChannelConfig_t adc5_channel_config[] = {
 {.channel = LOAD_FR_ADC_CH, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
 {.channel = LOAD_RR_ADC_CH, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};
typedef struct {
    uint16_t load_fr;
    uint16_t load_rr;
} raw_adc5_values_t;
volatile raw_adc5_values_t raw_adc5_values;
dma_init_t adc5_dma_config = ADC5_DMA_CONT_CONFIG((uint32_t)&raw_adc5_values, sizeof(raw_adc5_values) / sizeof(uint16_t), 0b01);

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern void HardFault_Handler();

void shockpot_thread();
void loadcell_thread();
void braketemp_thread();
void oil_temp_thread();
void brake_pressure_thread();
void water_temp_thread();
void amb_temp_thread();

defineThreadStack(shockpot_thread, 100, osPriorityNormal, 512);
defineThreadStack(loadcell_thread, 100, osPriorityNormal, 512);
defineThreadStack(braketemp_thread, 100, osPriorityNormal, 512);
defineThreadStack(oil_temp_thread, 100, osPriorityNormal, 512);
defineThreadStack(brake_pressure_thread, 100, osPriorityNormal, 512);
defineThreadStack(water_temp_thread, 100, osPriorityNormal, 512);
defineThreadStack(amb_temp_thread, 100, osPriorityNormal, 512);

int main(void) {
    //Hardware Initilization
    if (false == PHAL_initDMA(&adc1_dma_config)) {
        HardFault_Handler();
    }

    if (false == PHAL_initDMA(&adc2_dma_config)) {
        HardFault_Handler();
    }
     if (false == PHAL_initDMA(&adc3_dma_config)) {
        HardFault_Handler();
    }
     if (false == PHAL_initDMA(&adc4_dma_config)) {
        HardFault_Handler();
    }
     if (false == PHAL_initDMA(&adc5_dma_config)) {
        HardFault_Handler();
    }
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc1_config, adc1_channel_config, sizeof(adc1_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc2_config, adc2_channel_config, sizeof(adc2_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc3_config, adc3_channel_config, sizeof(adc3_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc4_config, adc4_channel_config, sizeof(adc4_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc5_config, adc5_channel_config, sizeof(adc5_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }

    PHAL_startTxfer(&adc1_dma_config);
    PHAL_startTxfer(&adc2_dma_config);
    PHAL_startTxfer(&adc3_dma_config);
    PHAL_startTxfer(&adc4_dma_config);
    PHAL_startTxfer(&adc5_dma_config);

    PHAL_startADC(&adc1_config);
    PHAL_startADC(&adc2_config);
    PHAL_startADC(&adc3_config);
    PHAL_startADC(&adc4_config);
    PHAL_startADC(&adc5_config);

    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    createThread(shockpot_thread);
    createThread(loadcell_thread);
    createThread(braketemp_thread);
    createThread(oil_temp_thread);
    createThread(brake_pressure_thread);
    createThread(water_temp_thread);
    createThread(amb_temp_thread);

    // no way home
    osKernelStart();

    return 0;
}

// Shockpots
int16_t shock_l_scaled;
int16_t shock_r_scaled;
int16_t shock_l_raw;
int16_t shock_r_raw;

void shockpot_thread() {

    shock_r_raw = (int16_t) raw_adc4_values.shock_r;
    shock_l_raw = (int16_t) raw_adc3_values.shock_l;
    
   float shock_l_parsed = -1.0 * ((POT_MAX_DIST - ((shock_l_raw / (POT_VOLT_MIN_L - POT_VOLT_MAX_L)) * POT_MAX_DIST)) - POT_DIST_DROOP_L);
   float shock_r_parsed = -1.0 * ((POT_MAX_DIST - ((shock_r_raw / (POT_VOLT_MIN_R - POT_VOLT_MAX_R)) * POT_MAX_DIST)) - POT_DIST_DROOP_R);

    shock_l_scaled = (int16_t)(shock_l_parsed * PACK_COEFF_FRONT_SHOCKPOTS_LEFT);
    shock_r_scaled = (int16_t)(shock_r_parsed * PACK_COEFF_FRONT_SHOCKPOTS_RIGHT);

    #ifdef SEND_SHOCKPOTS
    SEND_SHOCKPOTS(shock_l_scaled, shock_r_scaled);
    #endif
}

// Oil Temps
int16_t oil_temp_l;
int16_t oil_temp_r;
int16_t brake_pressure_l;
int16_t brake_pressure_r;

void oil_temp_thread() {
    oil_temp_l = (int16_t) raw_adc1_values.oil_temp_left;
    oil_temp_r = (int16_t) raw_adc2_values.oil_temp_right;


}

// Brake Pressures
int16_t brake_pressure_l;
int16_t brake_pressure_r;

void brake_pressure_thread() {
    brake_pressure_l = (int16_t) raw_adc2_values.brake_pressure_left;
    brake_pressure_r = (int16_t) raw_adc1_values.brake_pressure_right;
}

// Water Temps
int16_t water_temp_fl;
int16_t water_temp_rl;
int16_t water_temp_fr;
int16_t water_temp_rr;

void water_temp_thread() {
     water_temp_fl = raw_adc2_values.water_temp_fl;
     water_temp_rl = raw_adc2_values.water_temp_rl;
     water_temp_fr = raw_adc2_values.water_temp_fr;
     water_temp_rr = raw_adc3_values.water_temp_rr;

    // SEND_WATER_TEMPS(water_temp_fl, water_temp_rl, water_temp_fr, water_temp_rr);
}

// Ambient Temp
int16_t amb_temp;
void amb_temp_thread() {
    amb_temp = raw_adc3_values.amb_temp;

    // SEND_AMB_TEMP(amb_temp);
}

// Load Cells
float load_fl_kg;
float load_fr_kg;
float load_rl_kg;
float load_rr_kg;
uint16_t load_fl;
uint16_t load_fr;
uint16_t load_rl;
uint16_t load_rr;

//convert voltage to kg - yash
void loadcell_thread() {
    //Loading Data from struct
     load_fl = raw_adc4_values.load_fl;
     load_fr = raw_adc5_values.load_fr;
     load_rl = raw_adc2_values.load_rl;
     load_rr = raw_adc5_values.load_rr;


    // Calculation is (ADC_reading)/5V * calibrated weight
    load_fl_kg = (load_fl / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 
    load_fr_kg = (load_fr / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION; 
    load_rl_kg = (load_rl / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION;
    load_rr_kg = (load_rr / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION;

    SEND_LOAD(load_fl_kg,load_fr_kg,load_rl_kg, load_rr_kg);
}

// convert voltage to celsius?
// Brake Temps
int16_t brake_temp_l;
int16_t brake_temp_r;

void braketemp_thread() {

    //get raw voltage from brake temp sensors
    brake_temp_l = (int16_t) raw_adc2_values.brake_temp_left;
    brake_temp_r = (int16_t) raw_adc2_values.brake_temp_right;


    double brake_temp_v_l = brake_temp_l;
    double brake_temp_v_r = brake_temp_r;
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
