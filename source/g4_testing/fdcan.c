
#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_FDCAN)

#include <string.h>

#include "common/freertos/freertos.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_ANALOG(ADC1_CH1_GPIO_Port, ADC1_CH1_Pin),
    GPIO_INIT_ANALOG(ADC1_CH2_GPIO_Port, ADC1_CH2_Pin),
    GPIO_INIT_ANALOG(ADC1_CH3_GPIO_Port, ADC1_CH3_Pin),
    GPIO_INIT_ANALOG(ADC1_CH4_GPIO_Port, ADC1_CH4_Pin),

    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN3RX_PA8,
    GPIO_INIT_FDCAN3TX_PB4,
};

ADCInitConfig_t adc_config = {
    .periph         = ADC1,
    .prescaler      = ADC_CLK_PRESC_0,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .oversample     = ADC_OVERSAMPLE_16,
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
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 16000000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

static void can_tx_100hz(void);
static void can_rx_1khz(void);
static void ledblink3(void);
static void ledblink4(void);

defineThreadStack(can_tx_100hz, 10, osPriorityHigh, 256);
defineThreadStack(can_rx_1khz, 1, osPriorityHigh, 256);

defineStaticQueue(q_can_rx, CanMsgTypeDef_t, 256);

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

    // Send on CAN2, receive on CAN3
    if (!PHAL_FDCAN_init(FDCAN2, false, 500000U)) {
        HardFault_Handler();
    }
    if (!PHAL_FDCAN_init(FDCAN3, false, 500000U)) {
        HardFault_Handler();
    }
    uint32_t sids[8] = {0x300, 0x301};
    uint32_t xids[8] = {0x1ABCDE1, 0x1ABCDE2, 0x1ABCDE3};
    PHAL_FDCAN_setFilters(FDCAN2, sids, 2, xids, 3);
    PHAL_FDCAN_setFilters(FDCAN3, sids, 2, xids, 3);

    // Create threads
    createThread(can_tx_100hz);
    createThread(can_rx_1khz);

    createStaticQueue(q_can_rx, CanMsgTypeDef_t, 256);

    // NVIC
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_SetPriority(FDCAN3_IT0_IRQn, 7);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN3_IT0_IRQn);

    osKernelStart(); // Go!

    return 0;
}

void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        BaseType_t xHigherPriorityTaskWoken = 0;
        xQueueSendFromISR(q_can_rx, msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void PHAL_FDCAN_testExtended(void) {
    CanMsgTypeDef_t msg;
    msg.Bus            = FDCAN2;
    msg.IDE            = 1;
    msg.ExtId          = 0x1ABCDE0 + 1;
    uint8_t payload[8] = {'E', 'X', 'T', 'I', 'D', '_', 'T', 'X'};
    msg.DLC            = sizeof(payload);
    memcpy(msg.Data, payload, sizeof(payload));
    PHAL_FDCAN_send(&msg);
}

static void PHAL_FDCAN_testStandard(void) {
    CanMsgTypeDef_t msg;
    msg.Bus            = FDCAN3;
    msg.IDE            = 0;
    msg.StdId          = 0x300 + 4;
    uint8_t payload[8] = {'S', 'T', 'D', 'I', 'D', '_', 'T', 'X'};
    msg.DLC            = sizeof(payload);
    memcpy(msg.Data, payload, sizeof(payload));
    PHAL_FDCAN_send(&msg);
}

static void can_tx_100hz(void) {
    // PHAL_FDCAN_testStandard();
    PHAL_FDCAN_testExtended();
}

volatile CanMsgTypeDef_t rx_frame_0;

static void can_rx_1khz(void) {
    CanMsgTypeDef_t rx_frame;
    while (xQueueReceive(q_can_rx, &rx_frame, (TickType_t)0) == pdTRUE) {
        rx_frame_0 = rx_frame;
        ;
    }
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_FDCAN
