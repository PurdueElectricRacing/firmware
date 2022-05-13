#include "stm32l496xx.h"

#include "common/psched/psched.h"
#include "common/queue/queue.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/dma/dma.h"

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "daq.h"
#include "cooling.h"
#include "car.h"

GPIOInitConfig_t gpio_config[] = {
    // CAN
    GPIO_INIT_CANRX_PD1,
    GPIO_INIT_CANTX_PD0,
    GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // Status Indicators
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(UNDERGLOW_GPIO_Port, UNDERGLOW_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // Drivetrain
    GPIO_INIT_ANALOG(DT_THERM_1_GPIO_Port, DT_THERM_1_Pin),
    GPIO_INIT_ANALOG(DT_THERM_2_GPIO_Port, DT_THERM_2_Pin),
    GPIO_INIT_OUTPUT(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // GPIO_INIT_OUTPUT(DT_PUMP_FLOW_ADJ_GPIO_Port, DT_PUMP_FLOW_ADJ_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(DT_FLOW_RATE_PWM_GPIO_Port, DT_FLOW_RATE_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // Battery (HV)
    GPIO_INIT_ANALOG(BAT_THERM_OUT_GPIO_Port, BAT_THERM_OUT_Pin),
    GPIO_INIT_ANALOG(BAT_THERM_IN_GPIO_Port, BAT_THERM_IN_Pin),
    GPIO_INIT_OUTPUT(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // GPIO_INIT_OUTPUT(BAT_PUMP_FLOW_ADJ_GPIO_Port, BAT_PUMP_FLOW_ADJ_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(BAT_FLOW_RATE_PWM_GPIO_Port, BAT_FLOW_RATE_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // TODO: conversion for I_SENSE_C1
    GPIO_INIT_ANALOG(I_SENSE_C1_GPIO_Port, I_SENSE_C1_Pin),
    // Battery (LV)
    // TODO: use lipo bat stat
    GPIO_INIT_INPUT(LIPO_BAT_STAT_GPIO_Port, LIPO_BAT_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // TODO: use lv current
    GPIO_INIT_ANALOG(LV_I_SENSE_GPIO_Port, LV_I_SENSE_Pin),
    // I2C
    GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_I2C1_SCL_PB6,
    GPIO_INIT_I2C1_SDA_PB7,
    GPIO_INIT_I2C4_SCL_PB10,
    GPIO_INIT_I2C4_SDA_PB11,
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
    {.channel=DT_THERM_1_ADC_CHNL,    .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=DT_THERM_2_ADC_CHNL,    .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=BAT_THERM_OUT_ADC_CHNL, .rank=3, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=BAT_THERM_IN_ADC_CHNL,  .rank=4, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=I_SENSE_C1_ADC_CHNL,    .rank=5, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=LV_I_SENSE_ADC_CHNL,    .rank=6, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &adc_readings, 
            sizeof(adc_readings) / sizeof(adc_readings.dt_therm_1), 0b01);

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / (1)),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
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
void linkDAQVars();
void canTxUpdate(void);
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main (void)
{
    /* Data Struct Initialization */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

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
    schedInit(SystemCoreClock);
    configureAnim(preflightAnimation, preflightChecks, 60, 750);

    taskCreate(coolingPeriodic, 100);
    taskCreate(heartBeatLED, 500);
    taskCreate(carHeartbeat, 100);
    taskCreate(carPeriodic, 15);
    taskCreate(setFanPWM, 1);
    taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

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
            if(!PHAL_initI2C(I2C))
            {
                HardFault_Handler();
            }
            if(!PHAL_initI2C(DBG_I2C))
            {
                HardFault_Handler();
            }
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
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
           break;
        case 3:
           /* Module Initialization */
           carInit();
           coolingInit();
           break;
       case 4:
           initCANParse(&q_rx_can);
           linkDAQVars();
           daqInit(&q_tx_can, I2C);
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
void heartBeatLED()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
}

void linkDAQVars()
{
    linkReada(DAQ_ID_DT_LITERS_P_MIN_X10, &cooling.dt_liters_p_min_x10);
    linkReada(DAQ_ID_DT_FLOW_ERROR, &cooling.dt_flow_error);
    linkReada(DAQ_ID_DT_TEMP_ERROR, &cooling.dt_temp_error);
    linkReada(DAQ_ID_BAT_LITERS_P_MIN_X10, &cooling.bat_liters_p_min_x10);
    linkReada(DAQ_ID_BAT_FLOW_ERROR, &cooling.bat_flow_error);
    linkReada(DAQ_ID_BAT_TEMP_ERROR, &cooling.bat_temp_error);
}

void canTxUpdate(void)
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
    }
}

void bootloader_request_reset_CALLBACK(CanParsedData_t* data)
{
    Bootloader_ResetForFirmwareDownload();
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

void HardFault_Handler()
{
    // TODO: make error led stay on (watch dog is gonna just reset the micro)
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}