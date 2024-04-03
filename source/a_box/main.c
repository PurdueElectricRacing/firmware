/* System Includes */
#include "stm32f407xx.h"
#include "can_parse.h"
#include "common/bootloader/bootloader_common.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/can/can.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"

/* Module Includes */
#include "main.h"
#include "daq.h"
#include "orion.h"
#include "tmu.h"


#include "common/faults/faults.h"




/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
   // I-Sense
   GPIO_INIT_ANALOG(I_SENSE_CH1_GPIO_Port, I_SENSE_CH1_Pin),
   GPIO_INIT_ANALOG(I_SENSE_CH2_GPIO_Port, I_SENSE_CH2_Pin),

   // CAN
   GPIO_INIT_CANRX_PA11,
   GPIO_INIT_CANTX_PA12,

   // Status and HV Monitoring
   GPIO_INIT_OUTPUT_OPEN_DRAIN(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_INPUT(IMD_HS_PWM_GPIO_Port, IMD_HS_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(IMD_LS_PWM_GPIO_Port, IMD_LS_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin, GPIO_INPUT_OPEN_DRAIN),


   // LEDs
   GPIO_INIT_OUTPUT(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, GPIO_OUTPUT_LOW_SPEED),

   // TMU Mux Selects and Measurements
   GPIO_INIT_OUTPUT(MUX_A_Port, MUX_A_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_B_Port, MUX_B_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_C_Port, MUX_C_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_D_Port, MUX_D_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_ANALOG(TMU_1_Port, TMU_1_Pin),
   GPIO_INIT_ANALOG(TMU_2_Port, TMU_2_Pin),
   GPIO_INIT_ANALOG(TMU_3_Port, TMU_3_Pin),
   GPIO_INIT_ANALOG(TMU_4_Port, TMU_4_Pin),

   // Board Temp Measurement
   GPIO_INIT_ANALOG(BOARD_TEMP_Port, BOARD_TEMP_Pin),

   // 5V Monitoring
   GPIO_INIT_ANALOG(VSENSE_5V_Port, VSENSE_5V_Pin),

   // Orion BMS Comms (external pull up on PCB)
   GPIO_INIT_INPUT(BMS_DISCHARGE_ENABLE_Port, BMS_DISCHARGE_ENABLE_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(BMS_CHARGE_ENABLE_Port, BMS_CHARGE_ENABLE_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(BMS_CHARGER_SAFETY_Port, BMS_CHARGER_SAFETY_Pin, GPIO_INPUT_OPEN_DRAIN)
};


#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
   .system_source              =SYSTEM_CLOCK_SRC_HSI,
   .system_clock_target_hz     =TargetCoreClockrateHz,
   .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
   .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
   .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};


/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern uint8_t orion_error;

/**
 * q_tx_can_0 -> hlp [0,1] -> mailbox 1
 * q_tx_can_1 -> hlp [2,3] -> mailbox 2 
 * q_tx_can_2 -> hlp [4,5] -> mailbox 3
*/
q_handle_t q_tx_can_0, q_tx_can_1, q_tx_can_2;
q_handle_t q_rx_can;

bool bms_daq_override = false;
bool bms_daq_stat = false;


void PHAL_FaultHandler();
extern void HardFault_Handler();


void canTxUpdate();
void heartBeatLED();
void monitorStatus();
void preflightChecks();
void preflightAnimation();
void updateTherm();
void sendhbmsg();

tmu_handle_t tmu;

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6, // Desire ADC clock to be 30MHz (upper bound), clocked from APB2 (160/6=27MHz)
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .adc_number      = 1,
    .dma_mode        = ADC_DMA_CIRCULAR
};

volatile ADCReadings_t adc_readings;
ADCChannelConfig_t adc_channel_config[] = {
    {.channel=TMU_1_ADC_CHANNEL,    .rank=1,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
    {.channel=TMU_2_ADC_CHANNEL,    .rank=2,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
    {.channel=TMU_3_ADC_CHANNEL,    .rank=3,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
    {.channel=TMU_4_ADC_CHANNEL,    .rank=4,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &adc_readings,
            sizeof(adc_readings) / sizeof(adc_readings.tmu_1), 0b01);

int main (void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can_0, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_can_1, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_can_2, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));


   /* HAL Initilization */
   if (0 != PHAL_configureClockRates(&clock_config))
       PHAL_FaultHandler();


   if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
       PHAL_FaultHandler();

   /* ADC and DMA Initialization */
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

//    set high during init
//    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, 1);


   if (1 != PHAL_initCAN(CAN1, false))
       PHAL_FaultHandler();

//     for (uint16_t dimitri_is_not_better_than_me = 0; dimitri_is_not_better_than_me < 1000; dimitri_is_not_better_than_me++)
//     {
//         asm("nop");
//     }

//    if (1 != PHAL_initCAN(CAN2, fchrom alse))
//        PHAL_FaultHandler();


   // spi2_config.data_rate = APB2ClockRateHz / 16;
   // if (!PHAL_SPI_init(&spi2_config))
   //     PHAL_FaultHandler();


   NVIC_EnableIRQ(CAN1_RX0_IRQn);
   NVIC_EnableIRQ(CAN2_RX0_IRQn);

    initCANParse(&q_rx_can);
    orionInit();

    bms_daq_override = false;
    bms_daq_stat = false;

    if (daqInit(&q_tx_can_2))
        HardFault_Handler();

   /* Module init */
   schedInit(APB1ClockRateHz * 2); // See Datasheet DS11451 Figure. 4 for clock tree


   /* Task Creation */
   schedInit(SystemCoreClock);
   configureAnim(preflightAnimation, preflightChecks, 75, 750);
   taskCreate(heartBeatLED, 500);
   taskCreate(monitorStatus, 50);
   taskCreate(orionChargePeriodic, 50);
   taskCreate(heartBeatTask, 100);
   taskCreate(sendhbmsg, 500);
   taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);


   taskCreateBackground(canTxUpdate);
   taskCreateBackground(canRxUpdate);


   /* No Way Home */
   schedStart();


   return 0;
}




// *** Startup configuration ***
void preflightChecks(void)
{
   static uint16_t state;


   switch (state++)
   {
       case 0 :
            initTMU(&tmu);
            break;
        case 1:
            initFaultLibrary(FAULT_NODE_NAME, &q_tx_can_0, ID_FAULT_SYNC_A_BOX);
            break;
       default:
           if (state > 750)
           {
               registerPreflightComplete(1);
               state = 751;
           }
           break;
   }
}

void sendhbmsg()
{
    bool imd_status = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);


   SEND_PRECHARGE_HB(imd_status, orion_error);
}




void preflightAnimation(void)
{
   static uint32_t time = 0;


   PHAL_writeGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, 0);
   PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
   PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);


   switch (time++ % 3)
   {
       case 0:
           PHAL_writeGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, 1);
           break;


       case 1:
           PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
           break;


       case 2:
           PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
           break;
   }
}


// *** Misc. tasks ***
void heartBeatLED()
{
   if ((sched.os_ticks - last_can_rx_time_ms) >= 500)
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
   else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
   PHAL_toggleGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin);
}




void monitorStatus()
{
   uint8_t bms_err, imd_err;
   bms_err = orionErrors();
   imd_err = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);

//    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, !bms_err);

   if (bms_daq_override | tmu_daq_override) PHAL_toggleGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin);
   else PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, bms_err);
   readTemps(&tmu);

   setFault(ID_IMD_FAULT, imd_err);

   uint8_t stat = bms_err;
   if (bms_daq_override) stat = bms_daq_stat;
   PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, stat);
}


// *** Compulsory CAN Tx/Rx callbacks ***
/* CAN Message Handling */
void canTxSendToBack(CanMsgTypeDef_t *msg)
{
    if (msg->IDE == 1)
    {
        // extended id, check hlp
        switch((msg->ExtId >> 26) & 0b111)
        {
            case 0:
            case 1:
                qSendToBack(&q_tx_can_0, msg);
                break;
            case 2:
            case 3:
                qSendToBack(&q_tx_can_1, msg);
                break;
            default:
                qSendToBack(&q_tx_can_2, msg);
                break;
        }
    }
    else
    {
        qSendToBack(&q_tx_can_0, &msg);
}
}

void canTxUpdate(void)
{
    CanMsgTypeDef_t tx_msg;
    if(PHAL_txMailboxFree(CAN1, 0))
    {
        if (qReceive(&q_tx_can_0, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
        {
            PHAL_txCANMessage(&tx_msg, 0);
        }
    }
    if(PHAL_txMailboxFree(CAN1, 1))
    {
        if (qReceive(&q_tx_can_1, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
        {
            PHAL_txCANMessage(&tx_msg, 1);
        }
    }
    if(PHAL_txMailboxFree(CAN1, 2))
    {
        if (qReceive(&q_tx_can_2, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
        {
            PHAL_txCANMessage(&tx_msg, 2);
        }
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


       // Get either StdId or ExtId
       if (CAN_RI0R_IDE & CAN1->sFIFOMailBox[0].RIR)
       {
         rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
       }
       else
       {
         rx.StdId = (CAN_RI0R_STID & CAN1->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
       }


       rx.Bus = CAN1;
       rx.DLC = (CAN_RDT0R_DLC & CAN1->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;


       rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
       rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
       rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
       rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
       rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
       rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
       rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
       rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;


       CAN1->RF0R     |= (CAN_RF0R_RFOM0);
       canProcessRxIRQs(&rx);
       qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
   }
}




void CAN2_RX0_IRQHandler()
{
   if (CAN2->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
       CAN2->RF0R &= !(CAN_RF0R_FOVR0);


   if (CAN2->RF0R & CAN_RF0R_FULL0) // FIFO Full
       CAN2->RF0R &= !(CAN_RF0R_FULL0);


   if (CAN2->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
   {
       CanMsgTypeDef_t rx;
       rx.Bus = CAN2;


       // Get either StdId or ExtId
       if (CAN_RI0R_IDE & CAN2->sFIFOMailBox[0].RIR)
       {
         rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN2->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
       }
       else
       {
         rx.StdId = (CAN_RI0R_STID & CAN2->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
       }


       rx.DLC = (CAN_RDT0R_DLC & CAN2->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;


       rx.Data[0] = (uint8_t) (CAN2->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
       rx.Data[1] = (uint8_t) (CAN2->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
       rx.Data[2] = (uint8_t) (CAN2->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
       rx.Data[3] = (uint8_t) (CAN2->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
       rx.Data[4] = (uint8_t) (CAN2->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
       rx.Data[5] = (uint8_t) (CAN2->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
       rx.Data[6] = (uint8_t) (CAN2->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
       rx.Data[7] = (uint8_t) (CAN2->sFIFOMailBox[0].RDHR >> 24) & 0xFF;


       CAN2->RF0R     |= (CAN_RF0R_RFOM0);
       canProcessRxIRQs(&rx);
       qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
   }
}


void a_box_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
   if (can_data.a_box_bl_cmd.cmd == BLCMD_RST)
       Bootloader_ResetForFirmwareDownload();
}


void PHAL_FaultHandler()
{
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
   asm("bkpt");
   HardFault_Handler();
}
