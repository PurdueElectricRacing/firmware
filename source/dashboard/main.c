/**
 * @file main.c
 * @brief "Dashboard" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Chris Mcgalliard (cpmcgalliard@gmail.com)
 */

/* System Includes */
#include "can_library/faults_common.h"
#include "can_library/generated/DASHBOARD.h"
#include "common/common_defs/common_defs.h"
#include "common/freertos/freertos.h"
#include "common/heartbeat/heartbeat.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/usart.h"
#include "common/strbuf/strbuf.h"
#include "common/utils/countof.h"

/* Module Includes */
#include "driver_interface.h"
#include "lcd.h"
#include "main.h"
#include "pedals.h"
#include "telemetry.h"

GPIOInitConfig_t gpio_config[] = {
    // On-board LEDs
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // External LEDs
    GPIO_INIT_OUTPUT_OPEN_DRAIN(PRCHG_LED_PORT, PRCHG_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(IMD_LED_PORT, IMD_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(BMS_LED_PORT, BMS_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // Main Button inputs
    GPIO_INIT_INPUT(SELECT_BUTTON_PORT, SELECT_BUTTON_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(DOWN_BUTTON_PORT, DOWN_BUTTON_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(UP_BUTTON_PORT, UP_BUTTON_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(LEFT_BUTTON_PORT, LEFT_BUTTON_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(START_BUTTON_PORT, START_BUTTON_PIN, GPIO_INPUT_PULL_UP),

    // Steering Wheel buttons
    GPIO_INIT_INPUT(REGEN_TOGGLE_PORT, REGEN_TOGGLE_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(MARK_DATA_PORT, MARK_DATA_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(EBB_MINUS_PORT, EBB_MINUS_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(EBB_PLUS_PORT, EBB_PLUS_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(TV1_PLUS_PORT, TV1_PLUS_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(TV1_MINUS_PORT, TV1_MINUS_PIN, GPIO_INPUT_PULL_UP),

    // VCAN
    GPIO_INIT_FDCAN2RX_PB5,
    GPIO_INIT_FDCAN2TX_PB6,
    // SCAN
    GPIO_INIT_FDCAN3RX_PA8,
    GPIO_INIT_FDCAN3TX_PB4,

    // Throttle
    GPIO_INIT_ANALOG(THROTTLE1_PORT, THROTTLE1_PIN),
    GPIO_INIT_ANALOG(THROTTLE2_PORT, THROTTLE2_PIN),

    // Brake
    GPIO_INIT_ANALOG(BRAKE1_PORT, BRAKE1_PIN),
    GPIO_INIT_ANALOG(BRAKE2_PORT, BRAKE2_PIN),

    // Brake Pressure
    GPIO_INIT_ANALOG(BRAKE1_PRESSURE_PORT, BRAKE1_PRESSURE_PIN),
    GPIO_INIT_ANALOG(BRAKE2_PRESSURE_PORT, BRAKE2_PRESSURE_PIN),

    // LCD
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,
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
    {.channel = THROTTLE1_ADC_CHANNEL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = THROTTLE2_ADC_CHANNEL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE1_ADC_CHANNEL, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE2_ADC_CHANNEL, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE1_PRESSURE_ADC_CHANNEL, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE2_PRESSURE_ADC_CHANNEL, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480}
};

static_assert(
    (sizeof(raw_adc_values_t) / sizeof(uint16_t)) ==
    (sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t)),
    "ADC channel config and raw ADC values struct must have the same number of channels"
);

volatile raw_adc_values_t raw_adc_values; // DMA target

dma_init_t adc_dma_config =
ADC1_DMA_CONT_CONFIG(
    (uint32_t)&raw_adc_values,
    sizeof(raw_adc_values) / sizeof(raw_adc_values.t1), 0b01
);

// USART Configuration for LCD
dma_init_t usart_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
static constexpr uint32_t LCD_BAUD_RATE = 115'200;
usart_init_t lcd = {
    .baud_rate        = LCD_BAUD_RATE,
    .word_length      = WORD_8,
    .stop_bits        = SB_ONE,
    .parity           = PT_NONE,
    .hw_flow_ctl      = HW_DISABLE,
    .ovsample         = OV_16,
    .obsample         = OB_DISABLE,
    .periph           = USART1,
    .wake_addr        = false,
    .usart_active_num = USART1_ACTIVE_IDX,
    .tx_dma_cfg       = &usart_tx_dma_config,
    .rx_dma_cfg       = &usart_rx_dma_config,
};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI,
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

// LCD Variables
extern page_t curr_page;

/* Function Prototypes */
void sweep_external_leds();
void service_start_button();
extern void HardFault_Handler();

// Thread Defines
DEFINE_TASK(pedals_periodic, PEDALS_PERIOD_MS, osPriorityHigh, STACK_1024);
DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 1, osPriorityNormal, STACK_2048); // leave stack at 2048
DEFINE_TASK(fault_library_periodic, DASHBOARD_FAULT_SYNC_PERIOD_MS, osPriorityNormal, STACK_1024);
DEFINE_TASK(driver_interface_periodic, DRIVER_INTERFACE_PERIOD_MS, osPriorityLow, STACK_1024);
DEFINE_TASK(service_start_button, START_BUTTON_PERIOD_MS, osPriorityLow, STACK_512);
DEFINE_TASK(report_telemetry_02hz, TELEMETRY_02HZ_PERIOD_MS, osPriorityLow, STACK_512);
DEFINE_HEARTBEAT_TASK(sweep_external_leds);

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }
    if (false == PHAL_initUSART(&lcd, APB2ClockRateHz)) {
        HardFault_Handler();
    }
    if (false == PHAL_initADC(&adc_config, adc_channel_config, countof(adc_channel_config))) {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc_dma_config)) {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(&adc_config);

    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN3, false, SCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    CAN_library_init();
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_SetPriority(FDCAN3_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN3_IT0_IRQn);

    driver_interface_init();
    LCD_init(LCD_BAUD_RATE);

    // Software Initialization
    osKernelInitialize();

    START_TASK(pedals_periodic);
    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(fault_library_periodic);
    START_TASK(service_start_button);
    START_TASK(driver_interface_periodic);
    START_HEARTBEAT_TASK();

    osKernelStart(); // GO!

    return 0;
}

bool start_button_pressed = false;
void service_start_button() {
    start_button_pressed = PHAL_readGPIO(START_BUTTON_PORT, START_BUTTON_PIN);
    CAN_SEND_start_button(start_button_pressed);
}

// jose was here

void sweep_external_leds() {
    static uint32_t sweep_index = 0;

    switch (sweep_index++ % 4) {
        case 0:
            PHAL_writeGPIO(IMD_LED_PORT, IMD_LED_PIN, 1);
            PHAL_writeGPIO(BMS_LED_PORT, BMS_LED_PIN, 0);
            PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, 0);
            PHAL_writeGPIO(REGEN_LED_PORT, REGEN_LED_PIN, 0);
            break;
        case 1:
            PHAL_writeGPIO(IMD_LED_PORT, IMD_LED_PIN, 0);
            PHAL_writeGPIO(BMS_LED_PORT, BMS_LED_PIN, 1);
            PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, 0);
            PHAL_writeGPIO(REGEN_LED_PORT, REGEN_LED_PIN, 0);
            break;
        case 2:
            PHAL_writeGPIO(IMD_LED_PORT, IMD_LED_PIN, 0);
            PHAL_writeGPIO(BMS_LED_PORT, BMS_LED_PIN, 0);
            PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, 1);
            PHAL_writeGPIO(REGEN_LED_PORT, REGEN_LED_PIN, 0);
            break;
        case 3:
            PHAL_writeGPIO(IMD_LED_PORT, IMD_LED_PIN, 0);
            PHAL_writeGPIO(BMS_LED_PORT, BMS_LED_PIN, 0);
            PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, 0);
            PHAL_writeGPIO(REGEN_LED_PORT, REGEN_LED_PIN, 1);
            break;
    }
}

void zero_lws() {
    // CCW = command code word
    static constexpr uint8_t CONFIG_CCW_ZERO = 0x3;
    CAN_SEND_LWS_Config(CONFIG_CCW_ZERO, 0, 0);
}

void reset_lws() {
    // CCW = command code word
    static constexpr uint8_t CONFIG_CCW_RESET = 0x5;
    CAN_SEND_LWS_Config(CONFIG_CCW_RESET, 0, 0);
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
