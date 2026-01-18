/* System Includes */
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/can_library/faults_common.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/usart.h"
#include "common/psched/psched.h"

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
    GPIO_INIT_INPUT(BRK_STAT_TAP_GPIO_Port, BRK_STAT_TAP_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(BRK_FAIL_TAP_GPIO_Port, BRK_FAIL_TAP_Pin, GPIO_INPUT_OPEN_DRAIN),

    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,

    // Throttle
    GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
    GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),

    // Brake
    GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
    GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),

    // LCD
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,

    // Buttons/Switches
    GPIO_INIT_INPUT(B_SELECT_GPIO_Port, B_SELECT_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_DOWN_GPIO_Port, B_DOWN_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_UP_GPIO_Port, B_UP_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_LEFT_GPIO_Port, B_LEFT_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(B_RIGHT_GPIO_Port, B_RIGHT_Pin, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(DAQ_SWITCH_GPIO_Port, DAQ_SWITCH_Pin, GPIO_INPUT_OPEN_DRAIN),

    GPIO_INIT_INPUT(BRK_1_DIG_GPIO_Port, BRK_1_DIG_GPIO_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(BRK_2_DIG_GPIO_Port, BRK_2_DIG_GPIO_Pin, GPIO_INPUT_OPEN_DRAIN),

    // LV Status
    GPIO_INIT_ANALOG(LV_5V_V_SENSE_GPIO_Port, LV_5V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_3V3_V_SENSE_GPIO_Port, LV_3V3_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_12_V_SENSE_GPIO_Port, LV_12_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_24_V_SENSE_GPIO_Port, LV_24_V_SENSE_Pin),
    GPIO_INIT_INPUT(LV_24_V_FAULT_GPIO_Port, LV_24_V_FAULT_Pin, GPIO_INPUT_OPEN_DRAIN),
};

volatile raw_adc_values_t raw_adc_values;

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_2,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .dma_mode        = ADC_DMA_CIRCULAR,
    .adc_number      = 1,
};

ADCChannelConfig_t adc_channel_config[] = {
    {.channel = THTL_1_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = THTL_2_ADC_CHNL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_1_ADC_CHNL, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_2_ADC_CHNL, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LV_5V_V_SENSE_ADC_CHNL, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LV_3V3_V_SENSE_ADC_CHNL, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LV_12_V_SENSE_ADC_CHNL, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = LV_24_V_SENSE_ADC_CHNL, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
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

#define TargetCoreClockrateHz 16000000
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

brake_status_t brake_status = {0};

/* Function Prototypes */
void preflightChecks(void);
void preflightAnimation(void);
void heartBeatLED();
void lcdTxUpdate();
void enableInterrupts();
void encoderISR();
void handleDashboardInputs();
void sendBrakeStatus();
void sendVoltageData();
void pollBrakeStatus();
void sendVersion();
extern void HardFault_Handler();

// Communication queues
q_handle_t q_tx_usart;

int main(void) {
    /* Data Struct init */
    qConstruct(&q_tx_usart, NXT_STR_SIZE);

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 60, 2500);

    taskCreate(updateFaultDisplay, 500);
    taskCreate(heartBeatLED, 500);
    taskCreate(pedalsPeriodic, 15);
    taskCreate(handleDashboardInputs, 50);
    taskCreate(sendVersion, 5000);
    taskCreate(updateTelemetryPages, 200);
    taskCreate(pollBrakeStatus, 1000);
    taskCreate(sendTVParameters, 500);
    taskCreate(sendVoltageData, 5000);
    taskCreateBackground(lcdTxUpdate);
    taskCreateBackground(CAN_tx_update);
    taskCreateBackground(CAN_rx_update);

    schedStart();

    return 0;
}

/**
 * @brief Performs sequential initialization and setup of system peripherals and modules.
 *
 * @note Called repeatedly until preflight is registered as complete
 */
void preflightChecks(void) {
    static uint8_t state;

    switch (state++) {
        case 0:
            if (false == PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 1:
            if (false == PHAL_initUSART(&lcd, APB2ClockRateHz)) {
                HardFault_Handler();
            }
            break;
        case 2:
            if (false == PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
                HardFault_Handler();
            }
            if (false == PHAL_initDMA(&adc_dma_config)) {
                HardFault_Handler();
            }
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
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
        default:
            registerPreflightComplete(1);
            state = 255; // prevent wrap around
    }
}

void sendVersion() {
    CAN_SEND_dash_version(GIT_HASH);
}

// jose was here

void preflightAnimation(void) {
    // Controls external LEDs since they are more visible when dash is in car
    static uint32_t time_ext;
    static uint32_t time;

    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);

    PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 0);
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);

    switch (time++ % 6) // Creates a sweeping pattern
    {
        case 0:
        case 5:
            PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 1);
            break;
        case 1:
        case 4:
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
            break;
        case 2:
        case 3:
            PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
            break;
    }

    switch (time_ext++ % 4) // Creates a 25/75 blinking pattern
    {
        case 0:
            PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
            PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
            PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
            break;
    }
}

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

    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH) {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    } else {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
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

    static uint8_t trig;
    if (trig) {
        CAN_SEND_dash_can_stats(can_stats.can_peripheral_stats[CAN1_IDX].tx_of,
                            can_stats.can_peripheral_stats[CAN1_IDX].tx_fail,
                            can_stats.rx_of,
                            can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun);
    }

    trig = !trig;
}

void EXTI9_5_IRQHandler() {
    // EXTI9 (LEFT Button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR9) {
        input_state.left_button = 1;
        EXTI->PR |= EXTI_PR_PR9;
    }

    // EXTI8 (RIGHT Button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR8) {
        input_state.right_button = 1;
        EXTI->PR |= EXTI_PR_PR8;
    }

    // EXTI7 (DOWN Button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR7) {
        input_state.down_button = 1;
        EXTI->PR |= EXTI_PR_PR7;
    }

    // EXTI6 (UP Button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR6) {
        input_state.up_button = 1;
        EXTI->PR |= EXTI_PR_PR6;
    }
}

void EXTI15_10_IRQHandler() {
    // EXTI15 (SELECT button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR15) {
        input_state.select_button = 1;
        EXTI->PR |= EXTI_PR_PR15;
    }

    // EXTI14 (START button) triggered the interrupt
    if (EXTI->PR & EXTI_PR_PR14) {
        input_state.start_button = 1;
        EXTI->PR |= EXTI_PR_PR14;
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
        CAN_SEND_start_button(1);
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

    // Unmask interrupts
    EXTI->IMR |= (EXTI_IMR_MR6 | EXTI_IMR_MR7 | EXTI_IMR_MR8 | EXTI_IMR_MR9 | EXTI_IMR_MR14 | EXTI_IMR_MR15);
    
    // Set falling edge trigger for all buttons (pull-up)
    EXTI->RTSR &= ~(EXTI_RTSR_TR6 | EXTI_RTSR_TR7 | EXTI_RTSR_TR8 | EXTI_RTSR_TR9 | EXTI_RTSR_TR14 | EXTI_RTSR_TR15);
    EXTI->FTSR |= (EXTI_FTSR_TR6 | EXTI_FTSR_TR7 | EXTI_FTSR_TR8 | EXTI_FTSR_TR9 | EXTI_FTSR_TR14 | EXTI_FTSR_TR15);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Called periodically to send commands to the Nextion LCD display via USART
 *
 * @note The queue holds a max of 10 commands. Design your LCD page updates with this in mind.
 */
uint8_t cmd[NXT_STR_SIZE] = {'\0'}; // Buffer for Nextion LCD commands

void lcdTxUpdate() {
    if ((false == PHAL_usartTxBusy(&lcd)) && (SUCCESS_G == qReceive(&q_tx_usart, cmd))) {
        PHAL_usartTxDma(&lcd, (uint16_t*)cmd, strlen(cmd));
    }
}

void CAN1_RX0_IRQHandler() {
    CAN_rx_update();
}

void dashboard_bl_cmd_CALLBACK(can_data_t* can_data) {
    if (can_data->dashboard_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

/**
 * @brief Reads ADC values and sends scaled voltage data for different voltage rails
 *
 * Converts raw ADC values to actual voltages using voltage divider calculations
 * for 3.3V, 5V, 12V and 24V rails. Scales values by 100 before sending.
 * Resistor values must be manually updated if hardware changes.
 */
void sendVoltageData() {
    float adc_to_voltage = ADC_REF_VOLTAGE / 4095.0;

    float adc_voltage = raw_adc_values.lv_3v3_sense * adc_to_voltage;
    float vin_3v3     = adc_voltage * (LV_3V3_PULLUP + LV_3V3_PULLDOWN) / LV_3V3_PULLDOWN;

    adc_voltage  = raw_adc_values.lv_5v_sense * adc_to_voltage;
    float vin_5v = adc_voltage * (LV_5V_PULLUP + LV_5V_PULLDOWN) / LV_5V_PULLDOWN;

    adc_voltage   = raw_adc_values.lv_12v_sense * adc_to_voltage;
    float vin_12v = adc_voltage * (LV_12V_PULLUP + LV_12V_PULLDOWN) / LV_12V_PULLDOWN;

    adc_voltage   = raw_adc_values.lv_24_v_sense * adc_to_voltage;
    float vin_24v = adc_voltage * (LV_24V_PULLUP + LV_24V_PULLDOWN) / LV_24V_PULLDOWN;

    // Scale to 100x before sending
    CAN_SEND_dashboard_voltage(vin_3v3 * 100, vin_5v * 100, vin_12v * 100, vin_24v * 100);
}

void pollBrakeStatus() {
    brake_status.brake_status = PHAL_readGPIO(BRK_STAT_TAP_GPIO_Port, BRK_STAT_TAP_Pin);
    brake_status.brake_fail   = PHAL_readGPIO(BRK_FAIL_TAP_GPIO_Port, BRK_FAIL_TAP_Pin);
}

void HardFault_Handler() {
    schedPause();
    while (1)
        IWDG->KR = 0xAAAA; // Reset watchdog
}
