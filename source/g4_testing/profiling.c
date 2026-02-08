#include "FreeRTOS.h"
#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_PROFILING)

#include "common/freertos/freertos.h"
#include "common/phal/adc.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "main.h"
#include "stm32g4xx.h"


GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_RED_PORT, LED_RED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_ORANGE_PORT, LED_ORANGE_PIN, GPIO_OUTPUT_LOW_SPEED),

    GPIO_INIT_ANALOG(ADC1_CH1_GPIO_Port, ADC1_CH1_Pin),
    GPIO_INIT_ANALOG(ADC1_CH2_GPIO_Port, ADC1_CH2_Pin),
    GPIO_INIT_ANALOG(ADC1_CH3_GPIO_Port, ADC1_CH3_Pin),
    GPIO_INIT_ANALOG(ADC1_CH4_GPIO_Port, ADC1_CH4_Pin),
};

ADCInitConfig_t adc_config = {
    .periph = ADC1,
    .prescaler = ADC_CLK_PRESC_0,
    .resolution = ADC_RES_12_BIT,
    .data_align = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode = ADC_DMA_CIRCULAR,
    .oversample = ADC_OVERSAMPLE_16,
};

ADCChannelConfig_t adc_channel_config[] = {
    {.channel = ADC_CHANNEL_1, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = ADC_CHANNEL_2, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = ADC_CHANNEL_3, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = ADC_CHANNEL_4, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

volatile raw_adc_values_t raw_adc_values = {0};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc_values, ADC_NUM_CHANNELS, 0b01);

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source = CLOCK_SOURCE_HSI,
    .use_pll = false,
    .vco_output_rate_target_hz = 160000000,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz = (TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

static void ledblink1(void);
static void ledblink2(void);
static void ledblink3(void);
static void ledblink4(void);
static void getDebug(void);

defineThreadStack(ledblink1, 100, osPriorityHigh1, 256);
defineThreadStack(ledblink2, 100, osPriorityAboveNormal, 256);
defineThreadStack(ledblink3, 100, osPriorityAboveNormal, 256);
defineThreadStack(ledblink4, 100, osPriorityLow, 256);
defineThreadStack(getDebug, 500, osPriorityHigh7, 512);

char pcWriteBuffer[512];  

int main() {    
    osKernelInitialize();

    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (!PHAL_initADC(&adc_config, adc_channel_config, ADC_NUM_CHANNELS)) {
        HardFault_Handler();
    }

    if (!PHAL_initDMA(&adc_dma_config)) {
        HardFault_Handler();
    }

    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(&adc_config);

    PHAL_writeGPIO(LED_GREEN_PORT, LED_GREEN_PIN, 1);
    PHAL_writeGPIO(LED_RED_PORT, LED_RED_PIN, 1);
    PHAL_writeGPIO(LED_BLUE_PORT, LED_BLUE_PIN, 1);
    PHAL_writeGPIO(LED_ORANGE_PORT, LED_ORANGE_PIN, 1);

    // Create threads
    createThread(ledblink1);
    createThread(ledblink2);
    createThread(ledblink3);
    createThread(ledblink4);
    createThread(getDebug);

    osKernelStart(); // Go!

    return 0;
}

static void ledblink1(void) {
    PHAL_toggleGPIO(LED_GREEN_PORT, LED_GREEN_PIN);
    int i = 0;
    while(i < 50000) {
        i++;
        __asm__("nop");
    }
}

static void ledblink2(void) {
    PHAL_toggleGPIO(LED_RED_PORT, LED_RED_PIN);
    int i = 0;
    while(i < 100000) {
        i++;
        __asm__("nop");
    }
}

static void ledblink3(void) {
    PHAL_toggleGPIO(LED_BLUE_PORT, LED_BLUE_PIN);
    int i = 0;
    while(i < 200000) {
        i++;
        __asm__("nop");
    }
}

static void ledblink4(void) {
    PHAL_toggleGPIO(LED_ORANGE_PORT, LED_ORANGE_PIN);
    int i = 0;
    while(i < 400000) {
        i++;
        __asm__("nop");
    }
}

static void getDebug(void) {
    vTaskGetRunTimeStats(pcWriteBuffer);
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif 