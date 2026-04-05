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
#include "common/heartbeat/heartbeat.h"

/* Module Includes */
#include "common/phal_G4/adc/adc.h"
#include "common/phal_G4/dma/dma.h"
#include "pin_defs.h"
#include "config.h"
#include "oil_temp_table.h"
#include "amb_temp_table.h"
#include "water_temp_table.h"

#define ADC_RESOLUTION  4096.0f
#define V_REF           3.3f      
#define R_PULLUP        1300.0f
#define BRAKE_PRESSURE_OFFSET_V     0.5f
#define BRAKE_PRESSURE_SENSITIVITY  0.01538f
/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

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

DEFINE_TASK(shockpot_thread, 100, osPriorityNormal, 512);
DEFINE_HEARTBEAT_TASK(nullptr);

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

    CAN_library_init();
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    START_TASK(shockpot_thread);
    START_HEARTBEAT_TASK();

    // no way home
    osKernelStart();

    return 0;
}

// Shockpots
int16_t shock_r_raw;
int16_t shock_l_raw;

void shockpot_thread() {

    int16_t shock_r_raw = raw_adc4_values.shock_r;
    int16_t shock_l_raw = raw_adc3_values.shock_l;
    
    float shock_l_parsed = -1.0 * ((POT_MAX_DIST - ((shock_l_raw / (POT_VOLT_MIN_L - POT_VOLT_MAX_L)) * POT_MAX_DIST)) - POT_DIST_DROOP_L);
    float shock_r_parsed = -1.0 * ((POT_MAX_DIST - ((shock_r_raw / (POT_VOLT_MIN_R - POT_VOLT_MAX_R)) * POT_MAX_DIST)) - POT_DIST_DROOP_R);

    int16_t shock_l_scaled = (int16_t)(shock_l_parsed * PACK_COEFF_FRONT_SHOCKPOTS_LEFT);
    int16_t shock_r_scaled = (int16_t)(shock_r_parsed * PACK_COEFF_FRONT_SHOCKPOTS_RIGHT);

    #ifdef SEND_SHOCKPOTS
    SEND_SHOCKPOTS(shock_l_scaled, shock_r_scaled);
    #endif
}

// Load Cells

void loadcell_thread() {
    int16_t load_fl = raw_adc4_values.load_fl;
    int16_t load_fr = raw_adc5_values.load_fr;
    int16_t load_rl = raw_adc2_values.load_rl;
    int16_t load_rr = raw_adc5_values.load_rr;

    float load_fl_voltage = ((float)load_fl / ADC_RESOLUTION) * V_REF;
    float load_fr_voltage = ((float)load_fr / ADC_RESOLUTION) * V_REF;
    float load_rl_voltage = ((float)load_rl / ADC_RESOLUTION) * V_REF;
    float load_rr_voltage = ((float)load_rr / ADC_RESOLUTION) * V_REF;

    // Calculation is (ADC_reading)/5V * calibrated weight
    int16_t load_fl_N = (int16_t)((load_fl_voltage / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION * 9.81f);
    int16_t load_fr_N = (int16_t)((load_fr_voltage / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION * 9.81f);
    int16_t load_rl_N = (int16_t)((load_rl_voltage / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION * 9.81f);
    int16_t load_rr_N = (int16_t)((load_rr_voltage / LOAD_VOLT_MAX) * LOAD_CELL_CALIBRATION * 9.81f);


    SEND_LOAD(load_fl_N,load_fr_N,load_rl_N, load_rr_N);
}

// Brake Temps

void braketemp_thread() { 

    int16_t brake_temp_l = (int16_t) raw_adc2_values.brake_temp_left;
    int16_t brake_temp_r = (int16_t) raw_adc2_values.brake_temp_right;

    float brake_temp_l_voltage = ((float)brake_temp_l / ADC_RESOLUTION) * V_REF;
    float brake_temp_r_voltage = ((float)brake_temp_r / ADC_RESOLUTION) * V_REF;

    int16_t brake_temp_l_c = (int16_t) (200 * brake_temp_l_voltage - 100);
    int16_t brake_temp_r_c = (int16_t) (200 * brake_temp_r_voltage - 100);

    SEND_BRAKE_TEMPS(brake_temp_l_c, brake_temp_r_c);

}


// Oil Temps

void oil_temp_thread() {
    int16_t oil_temp_l = (int16_t) raw_adc1_values.oil_temp_left;
    int16_t oil_temp_r = (int16_t) raw_adc2_values.oil_temp_right;

    float oil_l_volts = ((float)oil_temp_l / ADC_RESOLUTION) * V_REF;
    float oil_r_volts = ((float)oil_temp_r / ADC_RESOLUTION) * V_REF;


    int16_t oil_temp_l_resistance = (R_PULLUP * oil_l_volts) / (V_REF - oil_l_volts);
    int16_t oil_temp_r_resistance = (R_PULLUP * oil_r_volts) / (V_REF - oil_r_volts);

    int16_t oil_temp_l_celsius = (int16_t) oil_temp_ohms_to_celsius(oil_temp_l_resistance);
    int16_t oil_temp_r_celsius = (int16_t) oil_temp_ohms_to_celsius(oil_temp_r_resistance);
    
    SEND_OIL_TEMPS(oil_temp_l_celsius, oil_temp_r_celsius);

}

// Ambient Temp

void amb_temp_thread() {
    int16_t amb_temp = raw_adc3_values.amb_temp;
    float voltage = ((float)amb_temp / ADC_RESOLUTION) * V_REF;
    float amb_temp_resistance = (R_PULLUP * voltage) / (V_REF - voltage);

    int16_t amb_temp_celsius = (int16_t) amb_temp_ohms_to_celsius(amb_temp_resistance);

    SEND_AMB_TEMP(amb_temp_celsius);
}

// Water Temps

void water_temp_thread() {

     int16_t water_temp_fl = raw_adc2_values.water_temp_fl;
     int16_t water_temp_rl = raw_adc2_values.water_temp_rl;
     int16_t water_temp_fr = raw_adc2_values.water_temp_fr;
     int16_t water_temp_rr = raw_adc3_values.water_temp_rr;

     float water_temp_fl_voltage = ((float)water_temp_fl / ADC_RESOLUTION) * V_REF;
     float water_temp_fr_voltage = ((float)water_temp_fr / ADC_RESOLUTION) * V_REF;
     float water_temp_rl_voltage = ((float)water_temp_rl / ADC_RESOLUTION) * V_REF;
     float water_temp_rr_voltage = ((float)water_temp_rr / ADC_RESOLUTION) * V_REF;

     float water_temp_fl_ohms = (R_PULLUP * water_temp_fl_voltage) / (V_REF - water_temp_fl_voltage);
     float water_temp_fr_ohms = (R_PULLUP * water_temp_fr_voltage) / (V_REF - water_temp_fr_voltage);
     float water_temp_rl_ohms = (R_PULLUP * water_temp_rl_voltage) / (V_REF - water_temp_rl_voltage);
     float water_temp_rr_ohms = (R_PULLUP * water_temp_rr_voltage) / (V_REF - water_temp_rr_voltage);

     int16_t water_temp_fl_celsius = (int16_t) water_temp_ohms_to_celsius(water_temp_fl_ohms);
     int16_t water_temp_fr_celsius = (int16_t) water_temp_ohms_to_celsius(water_temp_fr_ohms);
     int16_t water_temp_rl_celsius = (int16_t) water_temp_ohms_to_celsius(water_temp_rl_ohms);
     int16_t water_temp_rr_celsius = (int16_t) water_temp_ohms_to_celsius(water_temp_rr_ohms);


     SEND_WATER_TEMPS(water_temp_fl_celsius, water_temp_fr_celsius, water_temp_rl_celsius, water_temp_rr_celsius);
}

// Brake Pressures

void brake_pressure_thread() {

    int16_t brake_pressure_l = (int16_t) raw_adc2_values.brake_pressure_left;
    int16_t brake_pressure_r = (int16_t) raw_adc1_values.brake_pressure_right;

    float brake_pressure_l_volts = ((float)brake_pressure_l / ADC_RESOLUTION) * V_REF;
    float brake_pressure_r_volts = ((float)brake_pressure_r / ADC_RESOLUTION) * V_REF;

    int16_t brake_pressure_l_bar = (int16_t) ((brake_pressure_l_volts - BRAKE_PRESSURE_OFFSET_V) / BRAKE_PRESSURE_SENSITIVITY);
    int16_t brake_pressure_r_bar = (int16_t) ((brake_pressure_r_volts - BRAKE_PRESSURE_OFFSET_V) / BRAKE_PRESSURE_SENSITIVITY);

    SEND_BRAKE_PRESSURE(brake_pressure_l_bar, brake_pressure_r_bar);


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
