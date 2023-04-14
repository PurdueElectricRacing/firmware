/* System Includes */
#include "stm32l471xx.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/spi/spi.h"
#include "common/psched/psched.h"
#include "common/phal_L4/usart/usart.h"

/* Module Includes */
#include "bmi088.h"
#include "can_parse.h"
#include "bsxlite_interface.h"
#include "imu.h"
#include "main.h"
#include "gps.h"
#include "bmm150.h"
#include "SFS.h"
#include "SFS_pp.h"

extern q_handle_t q_tx_can;

uint8_t collect_test[100] = {0};

GPIOInitConfig_t gpio_config[] = {
    // Status Indicators
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),

    // SPI
    GPIO_INIT_AF(SPI_SCLK_GPIO_Port, SPI_SCLK_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI_MOSI_GPIO_Port, SPI_MOSI_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI_MISO_GPIO_Port, SPI_MISO_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(SPI_CS_MAG_GPIO_Port, SPI_CS_MAG_Pin, GPIO_OUTPUT_HIGH_SPEED),

    // GPS USART
    GPIO_INIT_USART3RX_PC5,
    GPIO_INIT_USART3TX_PC4,

    // EEPROM
    GPIO_INIT_OUTPUT(NAV_EEPROM_CS_GPIO_PORT, NAV_EEPROM_CS_PIN, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(NAV_WP_GPIO_PORT, NAV_WP_PIN, GPIO_OUTPUT_HIGH_SPEED),

    // CAN
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12};

/* USART Configuration */
// M9N GPS
dma_init_t usart_gps_tx_dma_config = USART3_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_gps_rx_dma_config = USART3_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_gps = {
    .baud_rate = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode = MODE_TX_RX,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .obsample = OB_DISABLE,
    .ovsample = OV_16,
    .adv_feature.rx_inv = false,
    .adv_feature.tx_inv = false,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_gps_tx_dma_config,
    .rx_dma_cfg = &usart_gps_rx_dma_config};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source = SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz = (TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

dma_init_t spi_rx_dma_config = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI1_TXDMA_CONT_CONFIG(NULL, 1);

SPI_InitConfig_t spi_config = {
    .data_rate = TargetCoreClockrateHz / 64,
    .data_len = 8,
    .nss_sw = true,
    .nss_gpio_port = SPI_CS_MAG_GPIO_Port,
    .nss_gpio_pin = SPI_CS_MAG_Pin,
    .rx_dma_cfg = NULL,
    .tx_dma_cfg = NULL,
    // .rx_dma_cfg = &spi_rx_dma_config,
    // .tx_dma_cfg = &spi_tx_dma_config,
    .periph = SPI1};

uint8_t num_iterations = 0;

// Test Nav Message
GPS_Handle_t testGPSHandle = {};
vector_3d_t accel_in, gyro_in, mag_in;

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
    .spi = &spi_config};
BMM150_Handle_t bmm_config = {
    .spi = &spi_config,
    .mag_csb_gpio_port = SPI_CS_MAG_GPIO_Port,
    .mag_csb_pin = SPI_CS_MAG_Pin};
IMU_Handle_t imu_h = {
    .bmi = &bmi_config,
};

/* Function Prototypes */
void canTxUpdate(void);
void heartBeatLED(void);
void preflightAnimation(void);
void preflightChecks(void);
void sendIMUData(void);
void collectGPSData(void);
void collectMagData(void);
extern void HardFault_Handler(void);
q_handle_t q_tx_can, q_rx_can;

/* SFS Definitions */
static ExtU rtU; /* External inputs */
static ExtY rtY; /* External outputs */
static RT_MODEL rtM_;
static RT_MODEL *const rtMPtr = &rtM_; /* Real-time model */
static DW rtDW;                        /* Observable states */
static RT_MODEL *const rtM = rtMPtr;

int main(void)
{
    // memset(spi2_tx_buffer + 8, 255, 100 - 8);

    /* Data Struct Initialization */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* HAL Initialization */
    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }

    /* GPIO initialization */
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    /* USART initialization */
    huart_gps.rx_dma_cfg->circular = true;
    if (!PHAL_initUSART(USART3, &huart_gps, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    PHAL_usartRxDma(USART3, &huart_gps, (uint16_t *)testGPSHandle.raw_message, 100);

    /* SPI initialization */
    if (!PHAL_SPI_init(&spi_config))
    {
        HardFault_Handler();
    }
    spi_config.data_rate = APB2ClockRateHz / 16;

    PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);
    PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    PHAL_writeGPIO(SPI_CS_MAG_GPIO_Port, SPI_CS_MAG_Pin, 1);

    // while (1)
    // {
    //     PHAL_usartRxBl(USART3, (uint16_t *)collect_test, 100);
    //     // PHAL_toggleGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin);
    // }

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 74, 1000);

    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    taskCreate(heartBeatLED, 500);

    taskCreate(sendIMUData, 10);
    taskCreate(collectGPSData, 40);
    taskCreate(collectMagData, 40);
    taskCreate(SFS_MAIN, 10);

    /* No Way Home */
    schedStart();

    return 0;
}

void preflightChecks(void)
{
    static uint16_t state;

    switch (state++)
    {
    case 0:
        if (!PHAL_initCAN(CAN1, false))
        {
            HardFault_Handler();
        }
        NVIC_EnableIRQ(CAN1_RX0_IRQn);
        break;
    case 2:
        if (!BMM150_readID(&bmm_config))
        {
            asm("nop");
        }
        break;
    case 1:
        if (!BMI088_init(&bmi_config))
        {
            HardFault_Handler();
        }
        break;
    case 250:
        BMI088_powerOnAccel(&bmi_config);
        break;
    case 500:
        if (!BMI088_initAccel(&bmi_config))
            HardFault_Handler();
        break;
    case 700:
        /* Pack model data into RTM */
        rtM->dwork = &rtDW;

        /* Initialize model */
        SFS_initialize(rtM);
    default:
        if (state > 750)
        {
            if (!imu_init(&imu_h))
                HardFault_Handler();
            initCANParse(&q_rx_can);
            registerPreflightComplete(1);
            state = 750; // prevent wrap around
        }
        break;
    }
}

void preflightAnimation(void)
{
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
    case 2:
    case 3:
        PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
        break;
    case 4:
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
        break;
    }
}
void heartBeatLED(void)
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);

    // if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
    //      PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    // else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
}

void sendIMUData(void)
{
    imu_periodic(&imu_h, &rtU);
}

uint8_t poll_pvt[] = {"0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19"};

// Test function for usartRxDma
void collectGPSData(void)
{
    testGPSHandle.messages_received++;
    BMI088_readGyro(&bmi_config, &gyro_in);
    BMI088_readAccel(&bmi_config, &accel_in);
    testGPSHandle.acceleration = accel_in;
    testGPSHandle.gyroscope = gyro_in;
    parseVelocity(&testGPSHandle, &rtU);
    // PHAL_usartRxBl(USART3, (uint16_t *)collect_test, 100);
    // PHAL_usartRxDma(USART3, &huart_gps, (uint16_t *)collect_test, 100);
    // PHAL_usartTxDma(USART3, &huart_gps, (uint16_t *)poll_pvt, strlen(poll_pvt));
    // while (PHAL_SPI_busy(&spi2_config))
    //     ;
    // if (num_iterations == 1)
    // {
    //     memset(spi2_tx_buffer, 255, sizeof(spi2_tx_buffer));
    // }
    // else
    // {
    //     PHAL_SPI_transfer(&spi2_config, spi2_tx_buffer, 100, spi2_rx_buffer);
    // }

    // if (spi2_rx_buffer[0] != 255)
    // {
    //     asm("nop");
    // }
    // while (PHAL_SPI_busy(&spi2_config))
    //     ;
    // num_iterations++;
}

void collectMagData(void)
{
    BMM150_readMag(&bmm_config, &rtU);
}

void canTxUpdate(void)
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G) // Check queue for items and take if there is one
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

        rx.Data[0] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
        rx.Data[1] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        rx.Data[2] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
        rx.Data[5] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
        rx.Data[6] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        CAN1->RF0R |= (CAN_RF0R_RFOM0);

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
    }
}

// void main_module_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
// {
//     if (can_data.main_module_bl_cmd.cmd == BLCMD_RST)
//         Bootloader_ResetForFirmwareDownload();
// }

void SFS_MAIN(void)
{
    SFS_pp(&rtU);

    static boolean_T OverrunFlag = false;

    /* Disable interrupts here */

    /* Check for overrun */
    if (OverrunFlag)
    {
        rtmSetErrorStatus(rtM, "Overrun");
        return;
    }

    OverrunFlag = true;

    /* Save FPU context here (if necessary) */
    /* Re-enable timer or interrupt here */
    /* Set model inputs here */

    /* Step the model */
    SFS_step(rtM, &rtU, &rtY);
    SEND_SFS_POS(q_tx_can, (int16_t)(rtY.pos_VNED[0] * 100),
                 (int16_t)(rtY.pos_VNED[1] * 100), (int16_t)(rtY.pos_VNED[2] * 100));
    SEND_SFS_VEL(q_tx_can, (int16_t)(rtY.vel_VNED[0] * 100),
                 (int16_t)(rtY.vel_VNED[1] * 100), (int16_t)(rtY.vel_VNED[2] * 100));
    SEND_SFS_ACC(q_tx_can, (int16_t)(rtY.acc_VNED[0] * 100),
                 (int16_t)(rtY.acc_VNED[1] * 100), (int16_t)(rtY.acc_VNED[2] * 100));
    SEND_SFS_ANG(q_tx_can, (int16_t)(rtY.ang_NED[0] * 10000),
                 (int16_t)(rtY.ang_NED[1] * 10000), (int16_t)(rtY.ang_NED[2] * 10000), (int16_t)(rtY.ang_NED[3] * 10000));
    SEND_SFS_ANG_VEL(q_tx_can, (int16_t)(rtY.angvel_VNED[0] * 10000),
                     (int16_t)(rtY.angvel_VNED[1] * 10000), (int16_t)(rtY.angvel_VNED[2] * 10000));
    /* Get model outputs here */

    /* Indicate task complete */
    OverrunFlag = false;

    /* Disable interrupts here */
    /* Restore FPU context here (if necessary) */
    /* Enable interrupts here */
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while (1)
    {
        __asm__("nop");
    }
}