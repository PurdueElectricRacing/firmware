/* System Includes */
#include "stm32l496xx.h"
#include "common/bootloader/bootloader_common.h"
#include "common/faults/faults.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/eeprom_spi/eeprom_spi.h"
#include "common/phal_L4/dma/dma.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/usart/usart.h"
#include "common/plettenberg/plettenberg.h"
#include "common/phal_L4/usart/usart.h"
#include "common/plettenberg/plettenberg.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"

/* Module Includes */
#include "car.h"
#include "can_parse.h"
#include "cooling.h"
#include "daq.h"
#include "main.h"
#include "power_monitor.h"


GPIOInitConfig_t gpio_config[] = {
    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,
    GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // Status Indicators
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(UNDERGLOW_GPIO_Port, UNDERGLOW_Pin, GPIO_OUTPUT_LOW_SPEED),
    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,
    GPIO_INIT_CAN2RX_PB12,
    GPIO_INIT_CAN2TX_PB13,
    // SPI
    GPIO_INIT_SPI1_SCK_PE13,
    GPIO_INIT_SPI1_MISO_PE14,
    GPIO_INIT_SPI1_MOSI_PE15,
    GPIO_INIT_OUTPUT(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(EEPROM_NSS_GPIO_Port, EEPROM_NSS_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_CARD_NSS_GPIO_Port, SD_CARD_NSS_Pin, GPIO_OUTPUT_LOW_SPEED),
    // SDC
    GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BSPD_TEST_CTRL_GPIO_Port, BSPD_TEST_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(V_MC_SENSE_GPIO_Port, V_MC_SENSE_Pin),
    GPIO_INIT_ANALOG(V_BAT_SENSE_GPIO_Port, V_BAT_SENSE_Pin),
    GPIO_INIT_INPUT(BMS_STAT_GPIO_Port, BMS_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // Motor Controllers
    GPIO_INIT_USART2TX_PD5,
    GPIO_INIT_USART2RX_PD6,
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,
    GPIO_INIT_AF(MC_L_PWM_GPIO_Port, MC_L_PWM_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(MC_R_PWM_GPIO_Port, MC_R_PWM_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
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
    GPIO_INIT_OUTPUT(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_AF(DT_FLOW_RATE_GPIO_Port, DT_FLOW_RATE_Pin, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(DT_FAN_CTRL_GPIO_Port, DT_FAN_CTRL_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_OUTPUT(DT_FAN_CTRL_GPIO_Port, DT_FAN_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_AF(DT_FAN_TACK_GPIO_Port, DT_FAN_TACK_Pin, 14, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    // HV Battery
    GPIO_INIT_OUTPUT(BAT_PUMP_CTRL_1_GPIO_Port, BAT_PUMP_CTRL_1_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BAT_PUMP_CTRL_2_GPIO_Port, BAT_PUMP_CTRL_2_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_AF(BAT_FLOW_RATE_GPIO_Port, BAT_FLOW_RATE_Pin, 3, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(BAT_FAN_CTRL_GPIO_Port, BAT_FAN_CTRL_Pin, 2, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_OUTPUT(BAT_FAN_CTRL_GPIO_Port, BAT_FAN_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_AF(BAT_FAN_TACK_GPIO_Port, BAT_FAN_TACK_Pin, 14, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    // LV Status
    GPIO_INIT_ANALOG(LV_24V_V_SENSE_GPIO_Port, LV_24V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_24V_I_SENSE_GPIO_Port, LV_24V_I_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_12V_V_SENSE_GPIO_Port, LV_12V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_5V_V_SENSE_GPIO_Port, LV_5V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_5V_I_SENSE_GPIO_Port, LV_5V_I_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_3V3_V_SENSE_GPIO_Port, LV_3V3_V_SENSE_Pin),
    GPIO_INIT_INPUT(LV_3V3_PG_GPIO_Port, LV_3V3_PG_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(LV_BAT_STAT_GPIO_Port, LV_BAT_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // Thermistor Analog Multiplexer
    GPIO_INIT_OUTPUT(THERM_MUX_S0_GPIO_Port, THERM_MUX_S0_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(THERM_MUX_S1_GPIO_Port, THERM_MUX_S1_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(THERM_MUX_D_GPIO_Port, THERM_MUX_D_Pin)
};

/* USART Configuration */
// Left Motor Controller UART
dma_init_t usart_l_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_l_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
char usart_l_rx_array[MC_MAX_RX_LENGTH] = {'\0'};
volatile usart_rx_buf_t huart_l_rx_buf = {
    .last_msg_time = 0, .msg_size = MC_MAX_TX_LENGTH,
    .last_msg_loc  = 0, .last_rx_time = 0,
    .rx_buf_size   = MC_MAX_RX_LENGTH, .rx_buf = usart_l_rx_array
};
usart_init_t huart_l = {
    .baud_rate   = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = false,
    .adv_feature.tx_inv    = false,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_l_tx_dma_config,
    .rx_dma_cfg = &usart_l_rx_dma_config
};
// Right Motor Controller UART
dma_init_t usart_r_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_r_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
char usart_r_rx_array[MC_MAX_RX_LENGTH] = {'\0'};
volatile usart_rx_buf_t huart_r_rx_buf = {
    .last_msg_time = 0, .msg_size = MC_MAX_TX_LENGTH,
    .last_msg_loc  = 0, .last_rx_time = 0,
    .rx_buf_size   = MC_MAX_RX_LENGTH, .rx_buf = usart_r_rx_array
};
usart_init_t huart_r = {
    .baud_rate   = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = false,
    .adv_feature.tx_inv    = false,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_r_tx_dma_config,
    .rx_dma_cfg = &usart_r_rx_dma_config
};
/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_16,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .overrun         = true,
    .dma_mode        = ADC_DMA_CIRCULAR
};

/* With 11 items, 16 prescaler, and 640 sample time, each channel gets read every 1.4ms */
volatile ADCReadings_t adc_readings;
ADCChannelConfig_t adc_channel_config[] = {
    {.channel=V_MC_SENSE_ADC_CHNL,     .rank=1,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=V_BAT_SENSE_ADC_CHNL,    .rank=2,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=SHOCK_POT_L_ADC_CHNL,    .rank=3,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=SHOCK_POT_R_ADC_CHNL,    .rank=4,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_24V_V_SENSE_ADC_CHNL, .rank=5,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_24V_I_SENSE_ADC_CHNL, .rank=6,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_12V_V_SENSE_ADC_CHNL, .rank=7,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_5V_V_SENSE_ADC_CHNL,  .rank=8,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_5V_I_SENSE_ADC_CHNL,  .rank=9,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_3V3_V_SENSE_ADC_CHNL, .rank=10, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=THERM_MUX_D_ADC_CHNL,    .rank=11, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=DT_GB_THERM_L_ADC_CHNL,  .rank=12, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=DT_GB_THERM_R_ADC_CHNL,  .rank=13, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=INTERNAL_THERM_ADC_CHNL, .rank=14, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &adc_readings,
            sizeof(adc_readings) / sizeof(adc_readings.lv_3v3_v_sense), 0b01);

/* SPI Configuration */
dma_init_t spi_rx_dma_config = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI1_TXDMA_CONT_CONFIG(NULL, 1);

SPI_InitConfig_t spi_config = {
    .data_len  = 8,
    .nss_sw = false,
    .nss_gpio_port = EEPROM_NSS_GPIO_Port,
    .nss_gpio_pin = EEPROM_NSS_Pin,
    .rx_dma_cfg = &spi_rx_dma_config,
    .tx_dma_cfg = &spi_tx_dma_config,
    .periph = SPI1
};


/* Clock Configuration */
#define TargetCoreClockrateHz 80000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_PLL,
    .pll_src                    =PLL_SRC_HSI16,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / 4),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / 4),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void preflightAnimation(void);
void preflightChecks(void);
void heartBeatLED();
void usartTxUpdate(void);
void usartIdleIRQ(volatile usart_init_t *huart, volatile usart_rx_buf_t *rx_buf);
void canTxUpdate(void);
void send_fault(uint16_t, bool);
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;
uint8_t can_tx_fails; // number of CAN messages that failed to transmit
q_handle_t q_tx_usart_l;
q_handle_t q_tx_usart_r;

int main(void){
    /* Data Struct Initialization */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    can_tx_fails = 0;
    qConstruct(&q_tx_usart_l, MC_MAX_TX_LENGTH);
    qConstruct(&q_tx_usart_r, MC_MAX_TX_LENGTH);

    /* HAL Initialization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }


    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 60, 750);

    taskCreate(coolingPeriodic, 200);
    taskCreate(heartBeatLED, 500);
    taskCreate(carHeartbeat, 100);
    taskCreate(carPeriodic, 15);
    taskCreate(wheelSpeedsPeriodic, 15);
    taskCreate(updatePowerMonitor, 100);
    taskCreate(heartBeatTask, 100);
    taskCreate(parseMCDataPeriodic, MC_LOOP_DT);
    taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);
    taskCreate(memFg, MEM_FG_TIME);
    // taskCreate(updateFaults, 1);
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);
    taskCreateBackground(usartTxUpdate);
    taskCreateBackground(memBg);

    // calibrateSteeringAngle(&i);
    // SEND_LWS_CONFIG(q_tx_can, 0x05, 0, 0); // reset cal
    // SEND_LWS_CONFIG(q_tx_can, 0x03, 0, 0); // start new

    schedStart();

    return 0;
}

void preflightChecks(void) {
    static uint8_t state;

    switch (state++)
    {
        case 0:
            huart_l.rx_dma_cfg->circular = true;
            if(!PHAL_initUSART(MC_L_UART, &huart_l, APB1ClockRateHz))
            {
                HardFault_Handler();
            }
            huart_r.rx_dma_cfg->circular = true;
            if(!PHAL_initUSART(MC_R_UART, &huart_r, APB2ClockRateHz))
            {
                HardFault_Handler();
            }
            break;
        case 1:
            if(!PHAL_initCAN(CAN1, false))
            {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            spi_config.data_rate = APB2ClockRateHz / 16; // 5 MHz
            if (!PHAL_SPI_init(&spi_config))
                HardFault_Handler();
            if (initMem(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, &spi_config, 1, 1) != E_SUCCESS)
                HardFault_Handler();
           break;
        case 2:
            if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config,
                            sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
            {
                HardFault_Handler();
            }
            if(!PHAL_initDMA(&adc_dma_config))
            {
                HardFault_Handler();
            }
            initPowerMonitor();
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
           break;
        case 3:
            /* UART Initialization */
            MC_L_UART->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TCIE | USART_CR1_TXEIE);
            NVIC_EnableIRQ(USART1_IRQn);
            // initial rx request
            PHAL_usartRxDma(MC_L_UART, &huart_l,
                            (uint16_t *) huart_l_rx_buf.rx_buf,
                            huart_l_rx_buf.rx_buf_size);
            MC_R_UART->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TCIE | USART_CR1_TXEIE);
            NVIC_EnableIRQ(USART2_IRQn);
            // initial rx request
            PHAL_usartRxDma(MC_R_UART, &huart_r,
                            (uint16_t *) huart_r_rx_buf.rx_buf,
                            huart_r_rx_buf.rx_buf_size);
            break;
        case 4:
           /* Module Initialization */
           carInit();
           coolingInit();
           break;
       case 5:
           initCANParse(&q_rx_can);
           if(daqInit(&q_tx_can))
               HardFault_Handler();
            initFaultLibrary(FAULT_NODE_NAME, &q_tx_can, ID_FAULT_SYNC_MAIN_MODULE);
           break;
        default:
            registerPreflightComplete(1);
            state = 255; // prevent wrap around
    }
}

void preflightAnimation(void) {
    static uint32_t time;

    PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 0);
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);

    switch (time++ % 6)
    {
        case 0:
        case 5:
            PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 1);
            break;
        case 1:
        case 4:
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
            break;
        case 2:
        case 3:
            PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
            break;
    }
}

void heartBeatLED(void)
{
    static uint8_t trig;
    // TODO: fix HB LED
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    // Send every other time (1000 ms)
    if (trig) {
        SEND_MCU_STATUS(q_tx_can, sched.skips, (uint8_t) sched.fg_time.cpu_use,
                                           (uint8_t) sched.bg_time.cpu_use,
                                           sched.error, can_tx_fails);
    }
    trig = !trig;
}

/* USART Message Handling */
uint8_t tmp_left[MC_MAX_TX_LENGTH] = {'\0'};
uint8_t tmp_right[MC_MAX_TX_LENGTH] = {'\0'};
void usartTxUpdate(void)
{
    if (PHAL_usartTxDmaComplete(&huart_l) &&
        qReceive(&q_tx_usart_l, tmp_left) == SUCCESS_G)
    {
        PHAL_usartTxDma(MC_L_UART, &huart_l, (uint16_t *) tmp_left, strlen(tmp_left));
    }
    if (PHAL_usartTxDmaComplete(&huart_r) &&
        qReceive(&q_tx_usart_r, tmp_right) == SUCCESS_G)
    {
        PHAL_usartTxDma(MC_R_UART, &huart_r, (uint16_t *) tmp_right, strlen(tmp_right));
    }
}

void USART1_IRQHandler(void) {
    if (MC_L_UART->ISR & USART_ISR_IDLE) {
        usartIdleIRQ(&huart_l, &huart_l_rx_buf);
        MC_L_UART->ICR = USART_ICR_IDLECF;
    }
}

void USART2_IRQHandler(void) {
    if (MC_R_UART->ISR & USART_ISR_IDLE) {
        usartIdleIRQ(&huart_r, &huart_r_rx_buf);
        MC_R_UART->ICR = USART_ICR_IDLECF;
    }
}

void usartIdleIRQ(volatile usart_init_t *huart, volatile usart_rx_buf_t *rx_buf)
{
    // TODO: check for overruns, framing errors, etc
    uint16_t new_loc = 0;
    rx_buf->last_rx_time = sched.os_ticks;
    new_loc = rx_buf->rx_buf_size - huart->rx_dma_cfg->channel->CNDTR;      // extract last location from DMA
    if (new_loc == rx_buf->rx_buf_size) new_loc = 0;                        // should never happen
    else if (new_loc < rx_buf->last_rx_loc) new_loc += rx_buf->rx_buf_size; // wrap around
    if (new_loc - rx_buf->last_rx_loc > rx_buf->msg_size)                   // status msg vs just an echo
    {
        rx_buf->last_msg_time = sched.os_ticks;
        rx_buf->last_msg_loc = (rx_buf->last_rx_loc + 1) % rx_buf->rx_buf_size;
    }
    rx_buf->last_rx_loc = new_loc % rx_buf->rx_buf_size;
}

/* CAN Message Handling */
void canTxUpdate(void)
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        if (!PHAL_txCANMessage(&tx_msg)) ++can_tx_fails;
    }
}

void CAN1_RX0_IRQHandler()
{
    if (CAN1->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
        CAN1->RF0R &= !(CAN_RF0R_FOVR0);
        CAN1->RF0R &= !(CAN_RF0R_FOVR0);

    if (CAN1->RF0R & CAN_RF0R_FULL0) // FIFO Full
        CAN1->RF0R &= !(CAN_RF0R_FULL0);
        CAN1->RF0R &= !(CAN_RF0R_FULL0);

    if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        CanMsgTypeDef_t rx;
        rx.Bus = CAN1;

        // Get either StdId or ExtId
        rx.IDE = CAN_RI0R_IDE & CAN1->sFIFOMailBox[0].RIR;
        if (rx.IDE)
        {
          rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
        }
        else
        {
          rx.StdId = (CAN_RI0R_STID & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_STID_Pos;
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
        CAN1->RF0R |= (CAN_RF0R_RFOM0);

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
    }
}

void main_module_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.main_module_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}