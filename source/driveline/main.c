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
#include "common/heartbeat/heartbeat.h"

/* Module Includes */
#include "pin_defs.h"
#include "config.h"

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
    GPIO_INIT_ANALOG(SHOCKPOT_RIGHT_GPIO_PORT, SHOCKPOT_RIGHT_GPIO_PIN)
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

DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048); // leave stack at 2048
DEFINE_TASK(shockpot_thread, 100, osPriorityNormal, STACK_512);
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
    if (false == PHAL_initDMA(&adc3_dma_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc4_dma_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc3_config, adc3_channel_config, sizeof(adc3_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc4_config, adc4_channel_config, sizeof(adc4_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc3_dma_config);
    PHAL_startTxfer(&adc4_dma_config);

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
    START_HEARTBEAT_TASK();

    // no way home
    osKernelStart();

    return 0;
}

// Both driveline nodes

void shockpot_thread() {
    // todo scale to physical units (mm)

    SEND_SHOCKPOTS(raw_adc3_values.shock_l, raw_adc4_values.shock_r);
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
