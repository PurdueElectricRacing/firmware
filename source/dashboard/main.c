/* System Includes */
#include "stm32l432xx.h"
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

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "daq.h"
#include "pedals.h"
#include "lcd.h"
#include "nextion.h"

#include "common/faults/faults.h"

GPIOInitConfig_t gpio_config[] = {
 GPIO_INIT_CANRX_PA11,
 GPIO_INIT_CANTX_PA12,
 GPIO_INIT_USART2TX_PA2,
 GPIO_INIT_USART2RX_PA3,
 // EEPROM
 GPIO_INIT_I2C1_SCL_PA9,
 GPIO_INIT_I2C1_SDA_PA10,
 GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
 // SPI
  GPIO_INIT_AF(SCK_GPIO_Port, SCK_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_AF(MISO_GPIO_Port, MISO_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_AF(MOSI_GPIO_Port, MOSI_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_OUTPUT(CSB_WHL_GPIO_Port, CSB_WHL_Pin, GPIO_OUTPUT_LOW_SPEED),
 // Throttle
 GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
 GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),
 // Brake
 GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
 GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
 GPIO_INIT_ANALOG(BRK_3_GPIO_Port, BRK_3_Pin),
 // Status LEDs
 GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT(HEART_LED_GPIO_Port, HEART_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT_OPEN_DRAIN(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT_OPEN_DRAIN(IMD_LED_GPIO_Port, IMD_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_OUTPUT_OPEN_DRAIN(BMS_LED_GPIO_Port, BMS_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
 GPIO_INIT_INPUT(START_BTN_GPIO_Port, START_BTN_Pin, GPIO_INPUT_PULL_UP),
};

/* ADC Configuration */
ADCInitConfig_t adc_config = {
   .clock_prescaler = ADC_CLK_PRESC_6,
   .resolution      = ADC_RES_12_BIT,
   .data_align      = ADC_DATA_ALIGN_RIGHT,
   .cont_conv_mode  = true,
   .overrun         = true,
   .dma_mode        = ADC_DMA_CIRCULAR
};
ADCChannelConfig_t adc_channel_config[] = {
   {.channel=THTL_1_ADC_CHNL, .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
   {.channel=THTL_2_ADC_CHNL, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
   {.channel=BRK_1_ADC_CHNL,  .rank=3, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
   {.channel=BRK_2_ADC_CHNL,  .rank=4, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
   {.channel=BRK_3_ADC_CHNL,  .rank=5, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_pedals, sizeof(raw_pedals) / sizeof(raw_pedals.t1), 0b01);

/* USART Confiugration */
dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart2 = {
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

/* SPI Configuration */
dma_init_t spi_rx_dma_cfg = SPI1_RXDMA_CONT_CONFIG(NULL, 1);
dma_init_t spi_tx_dma_cfg = SPI1_TXDMA_CONT_CONFIG(NULL, 2);
SPI_InitConfig_t hspi1 = {
   .data_rate     = 160000,
   .data_len      = 8,
   .nss_sw        = true,
   .nss_gpio_port = CSB_WHL_GPIO_Port,
   .nss_gpio_pin  = CSB_WHL_Pin,
   .rx_dma_cfg    = &spi_rx_dma_cfg,
   .tx_dma_cfg    = &spi_tx_dma_cfg,
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
   .system_source          = SYSTEM_CLOCK_SRC_HSI,
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

/* Function Prototypes */
void preflightChecks(void);
void preflightAnimation(void);
void heartBeatLED();
void heartBeatMsg();
void canTxUpdate();
void usartTxUpdate();
void linkDAQVars();
void checkStartBtn();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;
q_handle_t q_tx_usart;


int main (void)
{

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

    /* Task Creation */
    schedInit(SystemCoreClock);
    configureAnim(preflightAnimation, preflightChecks, 120, 750);

    taskCreate(heartBeatLED, 500);
    taskCreate(heartBeatMsg, 100);
    taskCreate(checkStartBtn, 100);
    taskCreate(pedalsPeriodic, 15);
    //********* UNCOMMENT END


    // taskCreate(joystickUpdatePeriodic, 60);
    // taskCreate(valueUpdatePeriodic, 60);

    //*******UNCOMMENT

    // taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    //*********UNCOMMENT

    // taskCreate(update_page, 500);

    taskCreate(update_time, 50);
    taskCreate(update_err_pages, 500);
    taskCreate(update_info_pages, 200);
    taskCreate(update_race_colors, 1000);
    taskCreate(updateBarsFast, 75);

    //taskCreate(check_precharge, 100);

    taskCreate(check_buttons, 100);
    // taskCreate(check_error, 1000);

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
            if(!PHAL_initUSART(USART2, &huart2, APB1ClockRateHz))
            {
                HardFault_Handler();
            }
            break;
        case 2:
            if(!PHAL_SPI_init(&hspi1))
            {
                HardFault_Handler();
            }
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
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
            break;
        case 5:
            /* Module Initialization */
            initCANParse(&q_rx_can);
            linkDAQVars();
            // if (daqInit(&q_tx_can, I2C1))
            // {
            //     HardFault_Handler();
            // }
            break;
        case 6:
            // char *race_page = "extra_info\0";
            // // PHAL_usartTxDma(USART2, &huart2, (uint16_t *) race_page, strlen(race_page));
            set_page("race\0");
            // char *new_text = "CAR_FUCKED\n";
            // set_text("t11\0", NXT_TEXT, new_text);
            // set_value("car_state", "=", 64528);
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

void heartBeatLED()
{
    PHAL_toggleGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin);
    if (can_data.main_hb.precharge_state)
        PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 0);
    else PHAL_writeGPIO(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, 1);
    // TODO IMD LED
    // PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, 1);
    PHAL_writeGPIO(IMD_LED_GPIO_Port, IMD_LED_Pin, !can_data.precharge_hb.IMD);
    // TODO BMS LED
    // PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, 1);
    PHAL_writeGPIO(BMS_LED_GPIO_Port, BMS_LED_Pin, !can_data.precharge_hb.BMS);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
}

void heartBeatMsg()
{
    SEND_DASHBOARD_HB(q_tx_can, pedals.apps_faulted,
                                    pedals.bse_faulted,
                                    pedals.apps_brake_faulted);
}

bool start_prev = false;
uint8_t start_ct = 0;
void checkStartBtn()
{
   // Button is inverted
   if (!PHAL_readGPIO(START_BTN_GPIO_Port, START_BTN_Pin))
   {
       if (!start_prev) start_ct++;
       if (start_ct > 3)
       {
           if (can_data.main_hb.car_state == CAR_STATE_READY2DRIVE || raw_pedals.b2 > BRAKE_PRESSURE_THRESHOLD)
           {
               SEND_START_BUTTON(q_tx_can, 1);
               start_prev = true;
               start_ct = 0;
           }
       }
   }
   else
   {
       start_prev = false;
       start_ct = 0;
   }
}

void linkDAQVars()
{
//    linkReada(DAQ_ID_T1MAX,  &pedal_calibration.t1max);
//    linkWritea(DAQ_ID_T1MAX, &pedal_calibration.t1max);
//    linkReada(DAQ_ID_T1MIN,  &pedal_calibration.t1min);
//    linkWritea(DAQ_ID_T1MIN, &pedal_calibration.t1min);
//    linkReada(DAQ_ID_T2MAX,  &pedal_calibration.t2max);
//    linkWritea(DAQ_ID_T2MAX, &pedal_calibration.t2max);
//    linkReada(DAQ_ID_T2MIN,  &pedal_calibration.t2min);
//    linkWritea(DAQ_ID_T2MIN, &pedal_calibration.t2min);
//    linkReada(DAQ_ID_B3MAX,  &pedal_calibration.b3max);
//    linkWritea(DAQ_ID_B3MAX, &pedal_calibration.b3max);
//    linkReada(DAQ_ID_B3MIN,  &pedal_calibration.b3min);
//    linkWritea(DAQ_ID_B3MIN, &pedal_calibration.b3min);
//    linkReada(DAQ_ID_B1,     &raw_pedals.b1);
//    linkReada(DAQ_ID_B2,     &raw_pedals.b2);
//    linkReada(DAQ_ID_T1,     &raw_pedals.t1);
//    linkReada(DAQ_ID_T2,     &raw_pedals.t2);
//    linkReada(DAQ_ID_B3,     &raw_pedals.b3);
}

uint8_t cmd[NXT_STR_SIZE] = {'\0'};
void usartTxUpdate()
{
    if (PHAL_usartTxDmaComplete(&huart2) &&
        qReceive(&q_tx_usart, cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART2, &huart2, (uint16_t *) cmd, strlen(cmd));
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