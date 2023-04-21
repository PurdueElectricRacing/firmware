/* System Includes */
#include "stm32l496xx.h"
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/bootloader/bootloader_common.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/usart/usart.h"
#include "common/phal_L4/spi/spi.h"


/* Module Includes */
#include "main.h"
#include "daq.h"
#include "imu.h"
#include "orion.h"
#include "bsxlite_interface.h"
#include "tmu.h"


#include "common/faults/faults.h"




/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
   // CAN
   GPIO_INIT_CANRX_PA11,
   GPIO_INIT_CANTX_PA12,
   GPIO_INIT_CAN2RX_PB12,
   GPIO_INIT_CAN2TX_PB13,


   // SPI
   GPIO_INIT_AF(SPI_SCLK_GPIO_Port, SPI_SCLK_Pin,  5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
   GPIO_INIT_AF(SPI_MOSI_GPIO_Port, SPI_MOSI_Pin,  5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
   GPIO_INIT_AF(SPI_MISO_GPIO_Port, SPI_MISO_Pin,  5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_SPI2_SCK_PB10,
   GPIO_INIT_SPI2_MOSI_PC3,
   GPIO_INIT_SPI2_MISO_PC2,
   GPIO_INIT_OUTPUT(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, GPIO_OUTPUT_HIGH_SPEED),
   GPIO_INIT_OUTPUT(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, GPIO_OUTPUT_HIGH_SPEED),
   GPIO_INIT_OUTPUT(SPI_CS_TMU_GPIO_Port, SPI_CS_TMU_GPIO_Pin, GPIO_OUTPUT_HIGH_SPEED),


   // Status and HV Monitoring
   GPIO_INIT_OUTPUT(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_INPUT(IMD_HS_PWM_GPIO_Port, IMD_HS_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(IMD_LS_PWM_GPIO_Port, IMD_LS_PWM_Pin, GPIO_INPUT_OPEN_DRAIN),
   GPIO_INIT_INPUT(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin, GPIO_INPUT_OPEN_DRAIN),


   // LEDs
   GPIO_INIT_OUTPUT(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, GPIO_OUTPUT_LOW_SPEED),

   //Select Pins
   GPIO_INIT_OUTPUT(MUX_A_NON_ISO_Port, MUX_A_NON_ISO_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_B_NON_ISO_Port, MUX_B_NON_ISO_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_C_NON_ISO_Port, MUX_C_NON_ISO_Pin, GPIO_OUTPUT_LOW_SPEED),
   GPIO_INIT_OUTPUT(MUX_D_NON_ISO_Port, MUX_D_NON_ISO_Pin, GPIO_OUTPUT_LOW_SPEED)
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


q_handle_t q_tx_can;
q_handle_t q_rx_can;


void PHAL_FaultHandler();
extern void HardFault_Handler();


void canTxUpdate();
void heartBeatLED();
void monitorStatus();
void sendIMUData();
void imuConfigureAccel();
void preflightChecks();
void preflightAnimation();


dma_init_t spi_rx_dma_config = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI1_TXDMA_CONT_CONFIG(NULL, 1);


SPI_InitConfig_t spi_config = {
   .data_rate = TargetCoreClockrateHz / 64,
   .data_len  = 8,
   .nss_sw = true,
   .nss_gpio_port = SPI_CS_GYRO_GPIO_Port,
   .nss_gpio_pin = SPI_CS_GYRO_Pin,
   .rx_dma_cfg = &spi_rx_dma_config,
   .tx_dma_cfg = &spi_tx_dma_config,
   .periph = SPI1
};


dma_init_t spi2_rx_dma_config = SPI2_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi2_tx_dma_config = SPI2_TXDMA_CONT_CONFIG(NULL, 1);
SPI_InitConfig_t spi2_config = {
   .data_rate = TargetCoreClockrateHz / 64,
   .data_len  = 8,
   .nss_sw = true,
   .nss_gpio_port = SPI_CS_TMU_GPIO_Port,
   .nss_gpio_pin = SPI_CS_TMU_GPIO_Pin,
   .rx_dma_cfg = &spi2_rx_dma_config,
   .tx_dma_cfg = &spi2_tx_dma_config,
   .periph = SPI2
};


BMI088_Handle_t bmi_config = {
   .accel_csb_gpio_port = SPI_CS_ACEL_GPIO_Port,
   .accel_csb_pin = SPI_CS_ACEL_Pin,
   .accel_range = ACCEL_RANGE_3G,
   .accel_odr = ACCEL_ODR_50Hz,
   .accel_bwp = ACCEL_OS_NORMAL,
   .gyro_csb_gpio_port = SPI_CS_GYRO_GPIO_Port,
   .gyro_csb_pin = SPI_CS_GYRO_Pin,
   .gyro_datarate = GYRO_DR_100Hz_32Hz,
   .gyro_range = GYRO_RANGE_250,
   .spi = &spi_config
};


tmu_handle_t tmu = {
   .spi = &spi2_config,
};


IMU_Handle_t imu_h = {
   .bmi = &bmi_config,
};




int main (void)
{
   /* Data Struct init */
   qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
   qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));


   /* HAL Initilization */
   if (0 != PHAL_configureClockRates(&clock_config))
       PHAL_FaultHandler();


   if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
       PHAL_FaultHandler();


//    set high during init
//    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, 1);


   if (1 != PHAL_initCAN(CAN1, false))
       PHAL_FaultHandler();


   if (1 != PHAL_initCAN(CAN2, false))
       PHAL_FaultHandler();


   spi2_config.data_rate = APB2ClockRateHz / 16;
   if (!PHAL_SPI_init(&spi2_config))
       PHAL_FaultHandler();


   NVIC_EnableIRQ(CAN1_RX0_IRQn);
   NVIC_EnableIRQ(CAN2_RX0_IRQn);

    initCANParse(&q_rx_can);
    orionInit();

    if (daqInit(&q_tx_can))
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
   // taskCreate(updateFaults, 1);
   // taskCreate(sendIMUData, 10);
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
           if (!initTMU(&tmu)) {
                PHAL_FaultHandler();
           }

           break;
        case 1:
            initFaultLibrary(FAULT_NODE_NAME, &q_tx_can, ID_FAULT_SYNC_PRECHARGE);
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

    bool imd_status = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);


   SEND_PRECHARGE_HB(q_tx_can, imd_status, orion_error);
}




void monitorStatus()
{
   uint8_t bms_err, imd_err;
   bms_err = orionErrors();
   imd_err = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);

//    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, !bms_err);

   PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, bms_err);
   readTemps(&tmu);

   setFault(ID_IMD_FAULT, imd_err);

   PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, bms_err | imd_err);
}






void sendIMUData()
{
   imu_periodic(&imu_h);
}




// *** Compulsory CAN Tx/Rx callbacks ***
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


void precharge_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
   if (can_data.precharge_bl_cmd.cmd == BLCMD_RST)
       Bootloader_ResetForFirmwareDownload();
}


void PHAL_FaultHandler()
{
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
   asm("bkpt");
   HardFault_Handler();
}
