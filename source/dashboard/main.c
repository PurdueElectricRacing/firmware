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
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/usart.h"
#include "common/freertos/freertos.h"

/* Module Includes */
#include "common/can_library/generated/DASHBOARD.h"
#include "lcd.h"
#include "main.h"
#include "nextion.h"
#include "pedals.h"

GPIOInitConfig_t gpio_config[] = {
    // Status Indicators
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEART_LED_GPIO_Port, HEART_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(IMD_LED_GPIO_Port, IMD_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(BMS_LED_GPIO_Port, BMS_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(START_BTN_GPIO_Port, START_BTN_Pin, GPIO_INPUT_PULL_UP),

    // CAN
    GPIO_INIT_FDCAN2RX_PB5,
    GPIO_INIT_FDCAN2TX_PB6,

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

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.t1), 0b01);

// USART Configuration for LCD
dma_init_t usart_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);

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

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
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
void preflight_sequence(void);
void preflightAnimation(void);
void heartBeatLED();
void lcdTxUpdate();
void enableInterrupts();
void encoderISR();
void handleDashboardInputs();
void sendBrakeStatus();
void sendVersion();
extern void HardFault_Handler();

// Communication queues
q_handle_t q_tx_usart;
bool g_is_preflight_complete = false;

void preflight_task();
void can_worker_task();

defineThreadStack(preflight_task, 10, osPriorityHigh, 1024);
defineThreadStack(pedalsPeriodic, FILT_THROTTLE_BRAKE_PERIOD_MS, osPriorityHigh, 512);
defineThreadStack(can_worker_task, 20, osPriorityNormal, 512);

defineThreadStack(updateFaultDisplay, 500, osPriorityLow, 256);
defineThreadStack(heartBeatLED, 500, osPriorityLow, 128);
defineThreadStack(handleDashboardInputs, 50, osPriorityLow, 1024);
defineThreadStack(sendVersion, DASH_VERSION_PERIOD_MS, osPriorityLow, 256);
defineThreadStack(updateTelemetryPages, 200, osPriorityLow, 1024);
defineThreadStack(sendTVParameters, DASHBOARD_VCU_PARAMETERS_PERIOD_MS, osPriorityLow, 256);

void preflight_task() {
    static uint8_t counter = 0;

    // run animation until preflight complete for at least 1.5 seconds
    if (g_is_preflight_complete && (counter >= 150)) {
        PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 0);
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
        PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);
        osThreadExit(); // Self delete
        return;
    }

    if (counter % 10 == 0) { // Run every 100ms
        preflightAnimation();
    }

    preflight_sequence();
    
    counter++;
}

void can_worker_task() {
    // Process all received CAN messages
    CAN_rx_update();

    // Drain all CAN transmit queues
    CAN_tx_update();

    lcdTxUpdate();
}

int main(void) {
    osKernelInitialize();

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // show signs of life
    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);
    PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 1);
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    
    qConstruct(&q_tx_usart, NXT_STR_SIZE);

    // Start preflight task
    createThread(preflight_task);

    osKernelStart(); // GO!

    return 0;
}

/**
 * @brief Performs sequential initialization and setup of system peripherals and modules.
 *
 * @note Called repeatedly until preflight is registered as complete
 */
void preflight_sequence(void) {
    static uint8_t step_10ms = 0;

    switch (step_10ms++) {
        case 0:
            if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
            break;
        case 1:
            if (false == PHAL_initUSART(&lcd, APB2ClockRateHz)) {
                HardFault_Handler();
            }
            break;
        case 2:
            if (false == PHAL_initADC(&adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
                HardFault_Handler();
            }
            if (false == PHAL_initDMA(&adc_dma_config)) {
                HardFault_Handler();
            }
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(&adc_config);
            break;
        case 3:
            /* Module Initialization */
            CAN_library_init();
            break;
        case 4:
            enableInterrupts();
            break;
        case 5:
            initLCD();
            break;
        case 6: {
            // create the other tasks here
            createThread(pedalsPeriodic);
            createThread(can_worker_task);

            createThread(updateFaultDisplay)
            createThread(heartBeatLED)
            createThread(handleDashboardInputs)
            createThread(sendVersion)
            createThread(updateTelemetryPages)
            createThread(sendTVParameters)
            break;
        }
        default: {
            g_is_preflight_complete = true;
            step_10ms = 255; // prevent wrap around
            break;
        }
    }
}

void preflightAnimation(void) {
    // Controls external LEDs since they are more visible when dash is in car
    static uint32_t external_index;
    static uint32_t sweep_index;

    switch (sweep_index++ % 3) { // Creates a sweeping pattern
        case 0:
            PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 1);
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
            PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);
            break;
        case 1:
            PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 0);
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
            PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);
            break;
        case 2:
            PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 0);
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
            PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
            break;
    }

    switch (external_index++ % 2) { // Creates a 50/50 blinking pattern
        case 0:
            PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
            PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
            PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
            break;
        case 1:
            PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
            PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
            PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
            break;
    }
}

void sendVersion() {
    CAN_SEND_dash_version(GIT_HASH);
}

// jose was here

/**
 * @brief Updates system LED indicators and CAN stats
 *
 * Controls heartbeat, connection, precharge, IMD and BMS status LEDs.
 * Handles periodic CAN statistics transmission.
 */
void heartBeatLED() {
    static uint8_t imd_prev_latched;
    static uint8_t bms_prev_latched;

    PHAL_toggleGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin);

    if ((xTaskGetTickCount() - last_can_rx_time_ms) >= CONN_LED_MS_THRESH) {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    } else {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    }

    if (!can_data.main_hb.stale && can_data.main_hb.precharge_state) {
        PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
    } else {
        PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);
    }

    if (!can_data.precharge_hb.stale) {
        if (can_data.precharge_hb.IMD) {
            imd_prev_latched = 1;
        }

        if (can_data.precharge_hb.BMS) {
            bms_prev_latched = 1;
        }
    } else {
        PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
        PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
    }

    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, !imd_prev_latched);
    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, !bms_prev_latched);
}

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

/**
 * @brief Processes dashboard button flags and triggers corresponding actions
 *
 * Meant to be called periodically.
 */
void handleDashboardInputs() {
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

    if (input_state.start_button) {
        input_state.start_button = 0;
        CAN_SEND_start_button(true);
    }
}

void enableInterrupts() {
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
    EXTI->IMR1 |= (EXTI_IMR1_IM6 | EXTI_IMR1_IM7 | EXTI_IMR1_IM8 |
                   EXTI_IMR1_IM9 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15);

    // Falling edge trigger only (pull-up buttons)
    EXTI->RTSR1 &= ~(EXTI_RTSR1_RT6 | EXTI_RTSR1_RT7 | EXTI_RTSR1_RT8 |
                     EXTI_RTSR1_RT9 | EXTI_RTSR1_RT14 | EXTI_RTSR1_RT15);

    EXTI->FTSR1 |= (EXTI_FTSR1_FT6 | EXTI_FTSR1_FT7 | EXTI_FTSR1_FT8 |
                    EXTI_FTSR1_FT9 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Called periodically to send commands to the Nextion LCD display via USART
 *
 * @note The queue holds a max of 10 commands. Design your LCD page updates with this in mind.
 */
char cmd[NXT_STR_SIZE] = {'\0'}; // Buffer for Nextion LCD commands
void lcdTxUpdate() {
    if ((false == PHAL_usartTxBusy(&lcd)) && (SUCCESS_G == qReceive(&q_tx_usart, cmd))) {
        PHAL_usartTxDma(&lcd, (uint8_t *)cmd, strlen(cmd));
    }
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL = 0;
    ERROR_LED_GPIO_Port->BSRR = ERROR_LED_Pin;
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
