/**
 * @file main.c
 * @brief "Dashboard" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Chris Mcgalliard (cpmcgalliard@gmail.com)
 */

/* System Includes */
#include "common/can_library/faults_common.h"
#include "common/common_defs/common_defs.h"
#include "common/freertos/freertos.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/usart.h"
#include "common/strbuf/strbuf.h"
#include "common/heartbeat/heartbeat.h"

/* Module Includes */
#include "common/can_library/generated/DASHBOARD.h"
#include "lcd.h"
#include "main.h"
#include "pedals.h"

GPIOInitConfig_t gpio_config[] = {
    // Status Indicators
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(PRCHG_LED_PORT, PRCHG_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(IMD_LED_PORT, IMD_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(BMS_LED_PORT, BMS_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(START_BTN_GPIO_Port, START_BTN_Pin, GPIO_INPUT_PULL_UP),

    // VCAN
    GPIO_INIT_FDCAN2RX_PB5,
    GPIO_INIT_FDCAN2TX_PB6,
    // SCAN
    GPIO_INIT_FDCAN3RX_PA8,
    GPIO_INIT_FDCAN3TX_PB4,

    // Throttle
    GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
    GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),

    // Brake
    GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
    GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
    GPIO_INIT_ANALOG(BRAKE1_PRESSURE_PORT, BRAKE1_PRESSURE_PIN),
    GPIO_INIT_ANALOG(BRAKE2_PRESSURE_PORT, BRAKE2_PRESSURE_PIN),

    // LCD
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,

    // Buttons/Switches
    GPIO_INIT_INPUT(B_SELECT_GPIO_Port, B_SELECT_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_DOWN_GPIO_Port, B_DOWN_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_UP_GPIO_Port, B_UP_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_LEFT_GPIO_Port, B_LEFT_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_RIGHT_GPIO_Port, B_RIGHT_Pin, GPIO_INPUT_PULL_UP)
};

volatile raw_adc_values_t raw_adc_values;

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
    {.channel = THTL_1_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = THTL_2_ADC_CHNL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_1_ADC_CHNL, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_2_ADC_CHNL, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE1_PRESSURE_ADC_CHANNEL, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRAKE2_PRESSURE_ADC_CHANNEL, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480}
};

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
volatile dashboard_input_state_t input_state = {0}; // Clear all input states

/* Function Prototypes */
void LCD_tx_update();
void config_button_irqs();
void driver_interface_periodic();
void send_version();
void LCD_init(uint32_t baud_rate);
void sweep_external_leds();
void service_start_button();
extern void HardFault_Handler();

// Communication queues
ALLOCATE_STRBUF(lcd_tx_buf, 2048);

// Thread Defines
DEFINE_TASK(pedalsPeriodic, PEDALS_PERIOD_MS, osPriorityHigh, STACK_1024);
DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048); // leave stack at 2048
DEFINE_TASK(fault_library_periodic, DASHBOARD_FAULT_SYNC_PERIOD_MS, osPriorityNormal, STACK_1024);
DEFINE_TASK(updateTelemetryPages, 100, osPriorityNormal, STACK_1024);
DEFINE_TASK(service_start_button, START_BUTTON_PERIOD_MS, osPriorityLow, STACK_512);
DEFINE_TASK(driver_interface_periodic, 50, osPriorityLow, STACK_1024);
DEFINE_TASK(LCD_tx_update, 20, osPriorityLow, STACK_512);
DEFINE_HEARTBEAT_TASK(sweep_external_leds);

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initUSART(&lcd, APB2ClockRateHz)) {
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

    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 5);
    CAN_library_init();

    config_button_irqs();
    LCD_init(LCD_BAUD_RATE);

    // Software Initialization
    osKernelInitialize();

    START_TASK(pedalsPeriodic);
    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(fault_library_periodic);
    START_TASK(updateTelemetryPages);
    START_TASK(service_start_button);
    START_TASK(driver_interface_periodic);
    START_TASK(LCD_tx_update);
    START_HEARTBEAT_TASK();

    osKernelStart(); // GO!

    return 0;
}

bool start_button_pressed = false;
void service_start_button() {
    start_button_pressed = PHAL_readGPIO(START_BTN_GPIO_Port, START_BTN_Pin);
    CAN_SEND_start_button(start_button_pressed);
}

void send_version() {
    CAN_SEND_dash_version(GIT_HASH);
}

// jose was here

void EXTI9_5_IRQHandler() {
    // EXTI9 (LEFT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF9) {
        input_state.left_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF9;
    }

    // EXTI8 (RIGHT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF8) {
        input_state.right_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF8;
    }

    // EXTI7 (DOWN Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF7) {
        input_state.down_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF7;
    }

    // EXTI6 (UP Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        input_state.up_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF6;
    }
}

void EXTI15_10_IRQHandler() {
    // EXTI15 (SELECT button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF15) {
        input_state.select_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF15;
    }

    // EXTI14 (START button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF14) {
        input_state.start_button = 1;
        EXTI->PR1 |= EXTI_PR1_PIF14;
    }
}

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

/**
 * @brief Processes dashboard button flags and triggers corresponding actions
 *
 * Meant to be called periodically.
 */
void driver_interface_periodic() {
    if (input_state.up_button) {
        input_state.up_button = 0;
        moveUp();
    }

    if (input_state.down_button) {
        input_state.down_button = 0;
        moveDown();
    }

    if (input_state.left_button) {
        input_state.left_button = 0;
        backPage();
    }

    if (input_state.right_button) {
        input_state.right_button = 0;
        advancePage();
    }

    if (input_state.select_button) {
        input_state.select_button = 0;
        selectItem();
    }

    if (input_state.update_page) {
        input_state.update_page = 0;
        updatePage();
    }

    // dont update the external LEDS until we're out of preflight
    if (status_leds.state == HEARTBEAT_STATE_PREFLIGHT) {
        return;
    }

    bool precharge_incomplete = is_latched(FAULT_ID_PRECHARGE_INCOMPLETE);
    PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, !precharge_incomplete);

    // todo IMD and BMS
}

void config_button_irqs() {
    // Enable the SYSCFG clock for interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Map EXTI lines to correct GPIO ports
    // PC6, PC7 (EXTI6, 7)
    SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PC | SYSCFG_EXTICR2_EXTI7_PC);
    // PC8, PC9 (EXTI8, 9)
    SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PC | SYSCFG_EXTICR3_EXTI9_PC);
    // PB14, PB15 (EXTI14, 15)
    SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PB | SYSCFG_EXTICR4_EXTI15_PB);

    // Unmask interrupts (EXTI lines 6,7,8,9,14,15)
    EXTI->IMR1 |= (EXTI_IMR1_IM6 | EXTI_IMR1_IM7 | EXTI_IMR1_IM8 | EXTI_IMR1_IM9 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15);

    // Falling edge trigger only (pull-up buttons)
    EXTI->RTSR1 &= ~(EXTI_RTSR1_RT6 | EXTI_RTSR1_RT7 | EXTI_RTSR1_RT8 | EXTI_RTSR1_RT9 | EXTI_RTSR1_RT14 | EXTI_RTSR1_RT15);

    EXTI->FTSR1 |= (EXTI_FTSR1_FT6 | EXTI_FTSR1_FT7 | EXTI_FTSR1_FT8 | EXTI_FTSR1_FT9 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Called periodically to send commands to the Nextion LCD display via USART
 */
void LCD_tx_update() {
    if ((false == PHAL_usartTxBusy(&lcd)) && (lcd_tx_buf.length > 0)) {
        PHAL_usartTxDma(&lcd, (uint8_t *)lcd_tx_buf.data, lcd_tx_buf.length);
        strbuf_clear(&lcd_tx_buf);
    }
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
