/**
 * @file main.c
 * @brief "Driveline" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Anya Pokrovskaya (apokrovs@purdue.edu)
 */

/* System Includes */
#include "common/can_library/generated/DRIVELINE.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/adc.h"
#include "common/phal/dma.h"
#include "common/freertos/freertos.h"
#include "common/heartbeat/heartbeat.h"
#include <math.h>
/* Module Includes */
#include "pin_defs.h"
#include "config.h"
#include "source/driveline/oil_temps/oil_temps_table.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB6,
    GPIO_INIT_FDCAN2RX_PB5,

    // Shock Pots
    GPIO_INIT_ANALOG(SHOCKPOT_LEFT_GPIO_PORT , SHOCKPOT_LEFT_GPIO_PIN),
    GPIO_INIT_ANALOG(SHOCKPOT_RIGHT_GPIO_PORT, SHOCKPOT_RIGHT_GPIO_PIN),

    //Oil temps
    GPIO_INIT_ANALOG(OIL_TEMP_L_GPIO_Port, OIL_TEMP_L_Pin),
    GPIO_INIT_ANALOG(OIL_TEMP_R_GPIO_Port, OIL_TEMP_R_Pin),

};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI, // todo change to HSE
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* ADC Configuration */

// ADC 1
ADCInitConfig_t adc1_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC1,
};

ADCChannelConfig_t adc1_channel_config[] = {
 {.channel = OIL_TEMP_L_ADC_CH, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
}; 
typedef struct {
    uint16_t oil_temp_left;
} raw_adc1_values_t;
volatile raw_adc1_values_t raw_adc1_values;

dma_init_t adc1_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc1_values, sizeof(raw_adc1_values) / sizeof(uint16_t), 0b01);

ADCInitConfig_t adc2_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC2,
};

// ADC 2
ADCChannelConfig_t adc2_channel_config[] = {
{.channel = OIL_TEMP_R_ADC_CH, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};
typedef struct {
    uint16_t oil_temp_right;
} raw_adc2_values_t;
volatile raw_adc2_values_t raw_adc2_values;

dma_init_t adc2_dma_config = ADC2_DMA_CONT_CONFIG((uint32_t)&raw_adc2_values, sizeof(raw_adc2_values) / sizeof(uint16_t), 0b01);

// ADC 3

ADCInitConfig_t adc3_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC3,
};

ADCChannelConfig_t adc3_channel_config[] = {
    {.channel = SHOCKPOT_LEFT_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

typedef struct {
    uint16_t shock_l;
} raw_adc3_values_t;
volatile raw_adc3_values_t raw_adc3_values;

dma_init_t adc3_dma_config = ADC3_DMA_CONT_CONFIG((uint32_t)&raw_adc3_values, sizeof(raw_adc3_values) / sizeof(uint16_t), 0b01);

// ADC 4

ADCInitConfig_t adc4_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC4,
};

ADCChannelConfig_t adc4_channel_config[] = {
    {.channel = SHOCKPOT_RIGHT_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

typedef struct {
    uint16_t shock_r;
} raw_adc4_values_t;
volatile raw_adc4_values_t raw_adc4_values;
dma_init_t adc4_dma_config = ADC4_DMA_CONT_CONFIG((uint32_t)&raw_adc4_values, sizeof(raw_adc4_values) / sizeof(uint16_t), 0b01);

// note: this struct is the target of the DMA controller,
// it's layout must match the order and size of the ADC channels in adc_channel_config
// additonally, it must have no padding and members must be uint16_t to match the ADC resolution and data alignment


/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern void HardFault_Handler();
void shockpot_thread();
void oil_temps_thread();

DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048); // leave stack at 2048
DEFINE_TASK(shockpot_thread, FRONT_SHOCKPOTS_PERIOD_MS, osPriorityNormal, STACK_512);
DEFINE_TASK(oil_temps_thread, FRONT_OIL_TEMPS_PERIOD_MS, osPriorityNormal, STACK_512);
DEFINE_HEARTBEAT_TASK(nullptr);

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
    
    PHAL_startTxfer(&adc1_dma_config);
    PHAL_startTxfer(&adc2_dma_config);
    PHAL_startTxfer(&adc3_dma_config);
    PHAL_startTxfer(&adc4_dma_config);

    PHAL_startADC(&adc1_config);
    PHAL_startADC(&adc2_config);
    PHAL_startADC(&adc3_config);
    PHAL_startADC(&adc4_config);


    CAN_library_init();
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(shockpot_thread);
    START_TASK(oil_temps_thread);
    START_HEARTBEAT_TASK();

    // no way home
    osKernelStart();

    return 0;
}

// Both driveline nodes

// Shock pots
static_assert(FRONT_SHOCKPOTS_LAYOUT_HASH == REAR_SHOCKPOTS_LAYOUT_HASH, "Shockpot messages should be the same");
void shockpot_thread() {
    // todo scale to physical units (mm)

    SEND_SHOCKPOTS(raw_adc3_values.shock_l, raw_adc4_values.shock_r);
}

int16_t oil_temp_r_celsius;
float oil_temp_r_resistance;
float oil_r_volts;
int16_t oil_temp_l_celsius;
float oil_temp_l_resistance;
float oil_l_volts;

// Oil Temps
static_assert(FRONT_OIL_TEMPS_LAYOUT_HASH == REAR_OIL_TEMPS_LAYOUT_HASH, "Oil temp messages should be the same");
void oil_temps_thread() {
    static constexpr float ADC_MAX = 4095.0f;
    static constexpr float ADC_VREF = 3.3f;
    static constexpr float ADC_TO_VOLTS = ADC_VREF / ADC_MAX;

    uint16_t oil_temp_l = raw_adc1_values.oil_temp_left;
    uint16_t oil_temp_r = raw_adc2_values.oil_temp_right;

    oil_l_volts = oil_temp_l * ADC_TO_VOLTS;
    oil_r_volts = oil_temp_r * ADC_TO_VOLTS;

    //dont know if we need to cast to float or not. probably?
    oil_temp_l_resistance = (oil_l_volts * 220) / (ADC_VREF - oil_l_volts);
    oil_temp_r_resistance = (float) ((oil_r_volts * 220) / (ADC_VREF- oil_r_volts));
    oil_temp_l_celsius = (int16_t) oil_temps_R_to_T(oil_temp_l_resistance);
    oil_temp_r_celsius = (int16_t) oil_temps_R_to_T(oil_temp_r_resistance);
    
   SEND_OIL_TEMPS(oil_temp_l_celsius, oil_temp_r_celsius);

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
