/* System Includes */
#include "stm32l496xx.h"
#include "common/bootloader/bootloader_common.h"
#include "common/psched/psched.h"
#include "common/phal_L4/usart/usart.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/spi/spi.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/faults/faults.h"

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "daq.h"
#include "pedals.h"
#include "lcd.h"
#include "nextion.h"
#include "hdd.h"


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
 // SPI Peripherals
 GPIO_INIT_SPI1_SCK_PE13,
 GPIO_INIT_SPI1_MISO_PE14,
 GPIO_INIT_SPI1_MOSI_PE15,
 GPIO_INIT_OUTPUT(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(EEPROM_NSS_GPIO_Port, EEPROM_NSS_Pin, GPIO_OUTPUT_LOW_SPEED),
 // Throttle
 GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
 GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),
 // Brake
 GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
 GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
 GPIO_INIT_ANALOG(BRK_3_GPIO_Port, BRK_3_Pin),
 // Motor Controllers
 GPIO_INIT_USART3TX_PC10,
 GPIO_INIT_USART3RX_PC11,
 GPIO_INIT_USART2TX_PD5,
 GPIO_INIT_USART2RX_PD6,
 // Wheel Speed
 GPIO_INIT_AF(MOTOR_L_WS_A_GPIO_Port, MOTOR_L_WS_A_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_AF(MOTOR_L_WS_B_GPIO_Port, MOTOR_L_WS_B_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_AF(MOTOR_L_WS_Z_GPIO_Port, MOTOR_L_WS_Z_Pin, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_INPUT(MOTOR_L_WS_ERROR_GPIO_Port, MOTOR_L_WS_ERROR_Pin, GPIO_INPUT_PULL_UP),
 GPIO_INIT_AF(MOTOR_R_WS_A_GPIO_Port, MOTOR_R_WS_A_Pin, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_AF(MOTOR_R_WS_B_GPIO_Port, MOTOR_R_WS_B_Pin, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_AF(MOTOR_R_WS_Z_GPIO_Port, MOTOR_R_WS_Z_Pin, 3, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
 GPIO_INIT_INPUT(MOTOR_R_WS_ERROR_GPIO_Port, MOTOR_R_WS_ERROR_Pin, GPIO_INPUT_PULL_UP),
 // Shock Pots
 GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
 GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),
 // Drivetrain
 GPIO_INIT_ANALOG(DT_GB_THERM_L_GPIO_Port, DT_GB_THERM_L_Pin),
 GPIO_INIT_ANALOG(DT_GB_THERM_R_GPIO_Port, DT_GB_THERM_R_Pin),
 // LCD
 GPIO_INIT_USART1TX_PA9,
 GPIO_INIT_USART1RX_PA10,
 // HDD
 GPIO_INIT_INPUT(B_OK_GPIO_Port, B_OK_Pin, GPIO_INPUT_OPEN_DRAIN),
 GPIO_INIT_INPUT(B_DOWN_GPIO_Port, B_DOWN_Pin, GPIO_INPUT_OPEN_DRAIN),
 GPIO_INIT_INPUT(B_UP_GPIO_Port, B_UP_Pin, GPIO_INPUT_OPEN_DRAIN),
 GPIO_INIT_INPUT(B_RIGHT_GPIO_Port, B_RIGHT_Pin, GPIO_INPUT_OPEN_DRAIN),
 GPIO_INIT_INPUT(B_LEFT_GPIO_Port, B_LEFT_Pin, GPIO_INPUT_OPEN_DRAIN),
 GPIO_INIT_OUTPUT(B_MUX_0_GPIO_Port, B_MUX_0_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(B_MUX_1_GPIO_Port, B_MUX_1_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(B_MUX_2_GPIO_Port, B_MUX_2_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(B_MUX_3_GPIO_Port, B_MUX_3_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(B_MUX_4_GPIO_Port, B_MUX_4_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_INPUT(B_MUX_DATA_GPIO_Port, B_MUX_DATA_Pin, GPIO_INPUT_OPEN_DRAIN),
 // LV Status
 GPIO_INIT_ANALOG(LV_5V_V_SENSE_GPIO_Port, LV_5V_V_SENSE_Pin),
 GPIO_INIT_ANALOG(LV_3V3_V_SENSE_GPIO_Port, LV_3V3_V_SENSE_Pin),
};

volatile raw_adc_values_t raw_adc_values;

/* ADC Configuration */
ADCInitConfig_t adc_config = {
   .clock_prescaler = ADC_CLK_PRESC_6,
   .resolution      = ADC_RES_12_BIT,
   .data_align      = ADC_DATA_ALIGN_RIGHT,
   .cont_conv_mode  = true,
   .overrun         = true,
   .dma_mode        = ADC_DMA_CIRCULAR
};
// TODO: check prescaler for udpate rate
ADCChannelConfig_t adc_channel_config[] = {
   {.channel=THTL_1_ADC_CHNL, .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=THTL_2_ADC_CHNL, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=BRK_1_ADC_CHNL,  .rank=3, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=BRK_2_ADC_CHNL,  .rank=4, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=BRK_3_ADC_CHNL,  .rank=5, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=SHOCK_POT_L_ADC_CH, .rank=6, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=SHOCK_POT_R_ADC_CH, .rank=7, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=LV_5V_V_SENSE_ADC_CHNL, .rank=8, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=LV_3V3_V_SENSE_ADC_CHNL, .rank=9, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
   {.channel=MCU_THERM_ADC_CHNL, .rank=10, .sampling_time=ADC_CHN_SMP_CYCLES_640_5}
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.t1), 0b01);

/* USART Confiugration */
dma_init_t usart_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t lcd = {
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .mode        = MODE_TX_RX,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .adv_feature = {
                   .auto_baud = false,
                   .ab_mode = AB_START,
                   .tx_inv = false,
                   .rx_inv = false,
                   .data_inv = false,
                   .tx_rx_swp = false,
                   .overrun = false,
                   .dma_on_rx_err = false,
                   .msb_first = false,
                  },
   .tx_dma_cfg = &usart_tx_dma_config,
   .rx_dma_cfg = &usart_rx_dma_config
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
   .system_source          = SYSTEM_CLOCK_SRC_HSI,
   .system_clock_target_hz = TargetCoreClockrateHz,
   .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
   .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
   .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

hdd_value_t hdd = {
    .deadband_pos = 0,
    .intensity_pos = 0,
    .deadband_prev = 0,
    .intensity_prev = 0
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern page_t curr_page;
extern uint8_t tvNotifiValue;
extern bool knob;

static int32_t ts_ratio;
static uint16_t ts_cal_1, ts_cal_2;


/* Function Prototypes */
void preflightChecks(void);
void preflightAnimation(void);
void heartBeatLED();
void canTxUpdate();
void usartTxUpdate();
extern void HardFault_Handler();
void pollHDD();
void enableInterrupts();
void sendMCUTempsVolts();
void sendVoltageSense();


q_handle_t q_tx_can;
q_handle_t q_rx_can;
q_handle_t q_tx_usart;

int main (void){

    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_usart, NXT_STR_SIZE);

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    initFaultLibrary(FAULT_NODE_NAME, &q_tx_can, ID_FAULT_SYNC_DASHBOARD);
    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);


    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 120, 2500);

    taskCreate(updatePage, 500);
    taskCreate(updateFaultDisplay, 500);
    taskCreate(heartBeatLED, 500);
    taskCreate(heartBeatTask, 100);
    taskCreate(pollHDD, 250);
    taskCreate(update_data_pages, 200);
    taskCreate(pedalsPeriodic, 15);
    taskCreate(sendMCUTempsVolts, 500);
    taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);

    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);


    taskCreateBackground(usartTxUpdate);

    schedStart();

    return 0;
}

void preflightChecks(void) {
    static uint8_t state;

    switch (state++)
    {
        case 0:
            if(!PHAL_initCAN(CAN1, false))
            {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
           break;
        case 1:
            if(!PHAL_initUSART(USART1, &lcd, APB2ClockRateHz))
            {
                HardFault_Handler();
            }
            break;
        case 2:
            //Enable MCU Internal Thermistor
            // ADC123_COMMON->CCR |= ADC_CCR_TSEN;
            // ts_cal_1 = *(TS_CAL1_ADDR);
            // ts_cal_2 = *(TS_CAL2_ADDR);
            // ts_ratio = (int32_t)(TS_CAL2_VAL - TS_CAL1_VAL) / (ts_cal_2 - ts_cal_1);
            break;
        case 3:
            // if(!PHAL_initI2C(I2C1))
            // {
            //     HardFault_Handler();
            // }
            break;
       case 4:
            if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
            {
                HardFault_Handler();
            }
            if(!PHAL_initDMA(&adc_dma_config))
            {
                HardFault_Handler();
            }
            ADC123_COMMON->CCR |= ADC_CCR_TSEN;
            ts_cal_1 = *(TS_CAL1_ADDR);
            ts_cal_2 = *(TS_CAL2_ADDR);
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
            break;
        case 5:
            /* Module Initialization */
            initCANParse(&q_rx_can);
            if (daqInit(&q_tx_can))
                HardFault_Handler();
            break;
        case 6:
            //Initialize HDD
            pollHDD();
            enableInterrupts();
            break;
        case 7:
            //Initialize LCD
            initLCD();
            break;
        default:
            registerPreflightComplete(1);
            state = 255; // prevent wrap around
    }
}

void preflightAnimation(void) {
    static uint32_t time;

    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);

    switch (time++ % 2)
    {
        case 0:
            PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
            PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
            PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
            break;
    }
}

void sendMCUTempsVolts() {
    int16_t calc_temp = (int16_t) ((((int32_t) raw_adc_values.mcu_therm)*ADC_VREF_INT/ TS_VREF - ts_cal_1) *
                             (TS_CAL2_VAL - TS_CAL1_VAL) / (ts_cal_2 - ts_cal_1) + TS_CAL1_VAL);
    float lv_5v_sense = ((VREF / 0xFFFU) * raw_adc_values.lv_5v_sense) / LV_5V_SCALE;
    float lv_3v3_sense = (VREF / 0xFFFU) * raw_adc_values.lv_3v3_sense;
    // SEND_DASHBOARD_VOLTS_TEMP(q_tx_can, calc_temp, (uint16_t)(lv_5v_sense * 100), (uint16_t)(lv_3v3_sense * 100));
    SEND_DASHBOARD_VOLTS_TEMP(q_tx_can, raw_adc_values.mcu_therm, raw_adc_values.lv_5v_sense, raw_adc_values.lv_3v3_sense);
}

void pollHDD() {
    hdd.deadband_prev = hdd.deadband_pos;
    hdd.intensity_prev = hdd.intensity_pos;
    for (uint8_t i = 0; i < 24; i++) {
        //BMUX0 == LSB, BMUX4 == LSB
        PHAL_writeGPIO(B_MUX_0_GPIO_Port, B_MUX_0_Pin, (bool)(i & 0x01));
        PHAL_writeGPIO(B_MUX_1_GPIO_Port, B_MUX_1_Pin, (bool)(i & 0x02));
        PHAL_writeGPIO(B_MUX_2_GPIO_Port, B_MUX_2_Pin, (bool)(i & 0x04));
        PHAL_writeGPIO(B_MUX_3_GPIO_Port, B_MUX_3_Pin, (bool)(i & 0x08));
        PHAL_writeGPIO(B_MUX_4_GPIO_Port, B_MUX_4_Pin, (bool)(i & 0x10));
        for (uint8_t j = 0; j < 10; j++) {
            __asm__("nop");
        }
        if (i <= 11) {
            if (PHAL_readGPIO(B_MUX_DATA_GPIO_Port, B_MUX_DATA_Pin)) {
                hdd.deadband_pos = i;
                if (hdd.deadband_pos != hdd.deadband_prev) {
                    knob = 1;
                    knobDisplay();
                }
            }
        }
        else {
            if (PHAL_readGPIO(B_MUX_DATA_GPIO_Port, B_MUX_DATA_Pin)) {
                hdd.intensity_pos = i - 12;
                if (hdd.intensity_pos != hdd.intensity_prev) {
                    knob = 0;
                    knobDisplay();
                }
            }
        }
    }
}

static volatile uint32_t last_click_time;
void EXTI0_IRQHandler() {
    if (EXTI->PR1 & EXTI_PR1_PIF0) {
        PHAL_toggleGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin);
        SEND_START_BUTTON(q_tx_can, 1);
        EXTI->PR1 |= EXTI_PR1_PIF0;
    }
}

void EXTI9_5_IRQHandler() {
    if (EXTI->PR1 & EXTI_PR1_PIF8) {
        if (sched.os_ticks - last_click_time < 200) {
            last_click_time = sched.os_ticks;
            EXTI->PR1 |= EXTI_PR1_PIF8;
        }
        else {
            last_click_time = sched.os_ticks;
            moveLeft();
            EXTI->PR1 |= EXTI_PR1_PIF8;
        }
    }
}

void EXTI15_10_IRQHandler() {
    if (EXTI->PR1 & EXTI_PR1_PIF12) {
        if (sched.os_ticks - last_click_time < 300) {
            last_click_time = sched.os_ticks;
            EXTI->PR1 |= EXTI_PR1_PIF12;
        }
        else {
            last_click_time = sched.os_ticks;
            selectItem();
            EXTI->PR1 |= EXTI_PR1_PIF12;
        }
    }
    else if (EXTI->PR1 & EXTI_PR1_PIF13) {
        if (sched.os_ticks - last_click_time < 250) {
            last_click_time = sched.os_ticks;
            EXTI->PR1 |= EXTI_PR1_PIF13;
        }
        else {
            last_click_time = sched.os_ticks;
            moveDown();
            EXTI->PR1 |= EXTI_PR1_PIF13;
        }
    }
    else if (EXTI->PR1 & EXTI_PR1_PIF14) {
        if (sched.os_ticks - last_click_time < 250) {
            last_click_time = sched.os_ticks;
            EXTI->PR1 |= EXTI_PR1_PIF14;
        }
        else {
            last_click_time = sched.os_ticks;
            moveUp();
            EXTI->PR1 |= EXTI_PR1_PIF14;
        }
    }
    else if (EXTI->PR1 & EXTI_PR1_PIF15) {
        if (sched.os_ticks - last_click_time < 250) {
            last_click_time = sched.os_ticks;
            EXTI->PR1 |= EXTI_PR1_PIF15;
        }
        else {
            last_click_time = sched.os_ticks;
            moveRight();
            EXTI->PR1 |= EXTI_PR1_PIF15;
        }
    }
}
void heartBeatLED()
{
    PHAL_toggleGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    if (!can_data.main_hb.stale && can_data.main_hb.precharge_state) {
        PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
    }
    else {
        PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);
    }
    if (!can_data.precharge_hb.stale) {
        PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, !can_data.precharge_hb.IMD);
        PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, !can_data.precharge_hb.BMS);
    }
    else {
        PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 0);
        PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 0);
    }
}

void enableInterrupts() {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    //Unmask + Enable interrupt for start button
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PE;
    EXTI->IMR1 |= EXTI_IMR1_IM0;
    EXTI->RTSR1 &= ~EXTI_RTSR1_RT0;
    EXTI->FTSR1 |= EXTI_FTSR1_FT0;
    NVIC_EnableIRQ(EXTI0_IRQn);

    //Left button
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PD;
    EXTI->IMR1 |= EXTI_IMR1_IM8;
    EXTI->FTSR1 |= EXTI_FTSR1_FT8;
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    //Ok button
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PB;
    EXTI->IMR1 |= EXTI_IMR1_IM12;
    EXTI->FTSR1 |= EXTI_FTSR1_FT12;
    EXTI->RTSR1 &= ~EXTI_RTSR1_RT12;
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    //Down Button
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PB;
    EXTI->IMR1 |= EXTI_IMR1_IM13;
    EXTI->FTSR1 |= EXTI_FTSR1_FT13;
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    //Up Button
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI14_PB;
    EXTI->IMR1 |= EXTI_IMR1_IM14;
    EXTI->FTSR1 |= EXTI_FTSR1_FT14;
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    //Right Button
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PB;
    EXTI->IMR1 |= EXTI_IMR1_IM15;
    EXTI->FTSR1 |= EXTI_FTSR1_FT15;
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

uint8_t cmd[NXT_STR_SIZE] = {'\0'};
void usartTxUpdate()
{
    if (PHAL_usartTxDmaComplete(&lcd) &&
        qReceive(&q_tx_usart, cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART1, &lcd, (uint16_t *) cmd, strlen(cmd));
    }
}


void canTxUpdate()
{
   CanMsgTypeDef_t tx_msg;
   if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
   {
       PHAL_txCANMessage(&tx_msg);
   }
}

void CAN1_RX0_IRQHandler()
{
   if (CAN1->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
       CAN1->RF0R &= !(CAN_RF0R_FOVR0);

   if (CAN1->RF0R & CAN_RF0R_FULL0) // FIFO Full
       CAN1->RF0R &= !(CAN_RF0R_FULL0);

   if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
   {
       CanMsgTypeDef_t rx;
       rx.Bus = CAN1;

       // Get either StdId or ExtId
       if (CAN_RI0R_IDE & CAN1->sFIFOMailBox[0].RIR)
       {
         rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
       }
       else
       {
         rx.StdId = (CAN_RI0R_STID & CAN1->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
       }

       rx.DLC = (CAN_RDT0R_DLC & CAN1->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;

       rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0)  & 0xFF;
       rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8)  & 0xFF;
       rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
       rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
       rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0)  & 0xFF;
       rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8)  & 0xFF;
       rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
       rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

       CAN1->RF0R |= (CAN_RF0R_RFOM0);
       qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
   }
}

void dashboard_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.dashboard_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void HardFault_Handler()
{
   schedPause();
   while(1) IWDG->KR = 0xAAAA;
}