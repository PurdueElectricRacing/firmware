/* System Includes */
#include "common/bootloader/bootloader_common.h"
#include "common/daq/can_parse_base.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/faults/faults.h"
#include "common/common_defs/common_defs.h"

/* Module Includes */
#include "main.h"
#include "source/torque_vector/can/can_parse.h"

#include "bsxlite_interface.h"

#include "bmi088.h"
#include "imu.h"
#include "gps.h"
#include "vcu.h"

#include <stdint.h>
#include <string.h>

uint8_t collect_test[100] = {0};

GPIOInitConfig_t gpio_config[] = {

    // LEDS
    GPIO_INIT_OUTPUT(ERR_LED_PORT, ERR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_PORT, CONN_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_ERR_LED_PORT, SD_ERR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_ACT_LED_PORT, SD_ACT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_DET_LED_PORT, SD_DET_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(PPS_GPS_PORT, PPS_GPS_PIN, GPIO_OUTPUT_LOW_SPEED),

    // SPI1 - ACCEL & GYRO
    GPIO_INIT_AF(SPI1_SCLK_PORT, SPI1_SCLK_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI1_MOSI_PORT, SPI1_MOSI_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI1_MISO_PORT, SPI1_MISO_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(SPI1_CSB_ACCEL_PORT, SPI1_CSB_ACCEL_PIN, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(SPI1_CSB_GYRO_PORT, SPI1_CSB_GYRO_PIN, GPIO_OUTPUT_HIGH_SPEED),

    // UART Logging
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,

    // GPS SPI
    // GPIO_INIT_AF(SPI2_CLK_GPS_PORT, SPI2_CLK_GPS_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_UART4RX_PA1,
    GPIO_INIT_UART4TX_PA0,

    // GPS Auxillary pins
    GPIO_INIT_OUTPUT(RESET_GPS_PORT, RESET_GPS_PIN, GPIO_OUTPUT_LOW_SPEED),

    // CAN
    // GPIO_INIT_CANRX_PA11,
    // GPIO_INIT_CANTX_PA12
    };

// GPS USART Configuration
dma_init_t usart_gps_tx_dma_config = USART4_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_gps_rx_dma_config = USART4_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_gps =
{
    .baud_rate = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .obsample = OB_DISABLE,
    .ovsample = OV_16,
    .periph   = UART4,
    .wake_addr = false,
    .usart_active_num = USART4_ACTIVE_IDX,
    .tx_errors = 0,
    .rx_errors = 0,
    .tx_dma_cfg = &usart_gps_tx_dma_config,
    .rx_dma_cfg = &usart_gps_rx_dma_config
};


dma_init_t usart_usb_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_usb_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t usb = {
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .periph      = USART1,
   .wake_addr   = false,
   .usart_active_num = USART1_ACTIVE_IDX,
   .tx_dma_cfg = &usart_usb_tx_dma_config,
   .rx_dma_cfg = &usart_usb_rx_dma_config
};

/*
Datasheet Page 12
The maximum
transfer rate using SPI is 125 kB/s and the maximum SPI clock frequency is 5.5 MHz.
*/


#define TargetCoreClockrateHz 96000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_PLL,
    .pll_src                    =PLL_SRC_HSI16,
    .vco_output_rate_target_hz  =192000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / 4),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / 4),
};

// #define TargetCoreClockrateHz 16000000
// ClockRateConfig_t clock_config = {
//     .system_source              =SYSTEM_CLOCK_SRC_HSE,
//     .pll_src                    =PLL_SRC_HSI16,
//     .vco_output_rate_target_hz  =16000000,
//     .system_clock_target_hz     =TargetCoreClockrateHz,
//     .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
//     .apb1_clock_target_hz       =(TargetCoreClockrateHz / 4),
//     .apb2_clock_target_hz       =(TargetCoreClockrateHz / 4),
// };

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
    .nss_gpio_port = SPI1_CSB_ACCEL_PORT,
    .nss_gpio_pin = SPI1_CSB_ACCEL_PIN,
    .rx_dma_cfg = &spi_rx_dma_config,
    .tx_dma_cfg = &spi_tx_dma_config,
    // .rx_dma_cfg = NULL,
    // .tx_dma_cfg = NULL,
    .periph = SPI1};

// Test Nav Message
GPS_Handle_t GPSHandle = {};
vector_3d_t accel_in, gyro_in, mag_in;

static struct serial_tx txmsg;
static struct serial_rx rxmsg;
static uint16_t rxbuffer[(sizeof(rxmsg) + 1) / 2];
static uint8_t txbuffer[2 + sizeof(txmsg)] = {0xAA, 0x55};

BMI088_Handle_t bmi_config = {
    .accel_csb_gpio_port = SPI1_CSB_ACCEL_PORT,
    .accel_csb_pin = SPI1_CSB_ACCEL_PIN,
    .accel_range = ACCEL_RANGE_3G,
    .accel_odr = ACCEL_ODR_50Hz,
    .accel_bwp = ACCEL_OS_NORMAL,
    .gyro_csb_gpio_port = SPI1_CSB_GYRO_PORT,
    .gyro_csb_pin = SPI1_CSB_GYRO_PIN,
    .gyro_datarate = GYRO_DR_100Hz_32Hz,
    .gyro_range = GYRO_RANGE_250,
    .spi = &spi_config};

IMU_Handle_t imu_h = {
    .bmi = &bmi_config,
};

/* Function Prototypes */
void heartBeatLED(void);
void preflightAnimation(void);
void preflightChecks(void);
extern void HardFault_Handler(void);

void parseIMU(void);
void pollIMU(void);
void VCU_MAIN(void);
void txUsart(void);

/* Moving Median Definition */
static int16_t gyro_counter = 0; /* Number of steps that gyro has not been checked */

// VCU structs
static pVCU_struct pVCU;
static fVCU_struct fVCU;
static xVCU_struct xVCU;
static yVCU_struct yVCU;

int main(void)
{
    /* Data Struct Initialization */

    /* HAL Initialization */
    PHAL_trimHSI(HSI_TRIM_TORQUE_VECTOR);
    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }

    /* GPIO initialization */
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 100, 750);
    PHAL_writeGPIO(RESET_GPS_PORT, RESET_GPS_PIN, 1);
    // taskCreateBackground(canTxUpdate);
    // taskCreateBackground(canRxUpdate);

    taskCreate(heartBeatLED, 500);
    taskCreate(txUsart, 100);
    taskCreate(heartBeatTask, 100);

    taskCreate(parseIMU, 20);
    // taskCreate(pollIMU, 20);
    taskCreate(VCU_MAIN, 15);

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
        // if (!PHAL_initCAN(CAN1, false, VCAN_BPS))
        // {
        //     HardFault_Handler();
        // }
        // NVIC_EnableIRQ(CAN1_RX0_IRQn);
        // PHAL_writeGPIO(SPI1_CSB_ACCEL_PORT, SPI1_CSB_ACCEL_PIN, 0);
        // PHAL_writeGPIO(SPI1_CSB_GYRO_PORT, SPI1_CSB_GYRO_PIN, 0);
        break;
    case 2:
        // /* USART initialization */
        // if (!PHAL_initUSART(&huart_gps, APB1ClockRateHz))
        // {
        //     HardFault_Handler();
        // }
        if (!PHAL_initUSART(&usb, APB1ClockRateHz))
        {
            HardFault_Handler();
        }
        break;
    case 3:
        // GPS Initialization
        PHAL_writeGPIO(RESET_GPS_PORT, RESET_GPS_PIN, 1);
        // PHAL_usartRxDma(&huart_gps, (uint16_t *)GPSHandle.raw_message, 100, 1);
        PHAL_usartRxDma(&usb, rxbuffer, sizeof(rxbuffer), 1);
    break;
    case 5:
        initFaultLibrary(FAULT_NODE_NAME, &q_tx_can[CAN1_IDX][CAN_MAILBOX_HIGH_PRIO], ID_FAULT_SYNC_TORQUE_VECTOR);
        PHAL_writeGPIO(SPI1_CSB_ACCEL_PORT, SPI1_CSB_ACCEL_PIN, 1);
        PHAL_writeGPIO(SPI1_CSB_GYRO_PORT, SPI1_CSB_GYRO_PIN, 1);
        break;
    case 10:
        /* SPI initialization */
        if (!PHAL_SPI_init(&spi_config))
        {
            HardFault_Handler();
        }
        spi_config.data_rate = APB2ClockRateHz / 16;
        break;
    case 200:
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
        /* Initialize VCU structs */
        pVCU = init_pVCU();
        fVCU = init_fVCU();
        xVCU = init_xVCU();
        yVCU = init_yVCU();
        break;
    default:
        if (state > 750)
        {
            if (!imu_init(&imu_h))
                HardFault_Handler();
            //initCANParse();
            registerPreflightComplete(1);
            state = 750; // prevent wrap around
        }
        break;
    }
}

void preflightAnimation(void)
{
    static uint32_t time;

    PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
    PHAL_writeGPIO(ERR_LED_PORT, ERR_LED_PIN, 0);
    PHAL_writeGPIO(CONN_LED_PORT, CONN_LED_PIN, 0);

    switch (time++ % 6)
    {
    case 0:
    case 5:
        PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 1);
        break;
    case 1:
    case 2:
    case 3:
        PHAL_writeGPIO(ERR_LED_PORT, ERR_LED_PIN, 1);
        break;
    case 4:
        PHAL_writeGPIO(CONN_LED_PORT, CONN_LED_PIN, 1);
        break;
    }
}

void heartBeatLED(void)
{
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);

    // if ((sched.os_ticks - last_can_rx_time_ms) >= 1000)
    //      PHAL_writeGPIO(CONN_LED_PORT, CONN_LED_PIN, 0);
    // else PHAL_writeGPIO(CONN_LED_PORT, CONN_LED_PIN, 1);


    // static uint8_t trig;
    // //if (trig) SEND_TV_CAN_STATS(can_stats.tx_of, can_stats.tx_fail, can_stats.rx_of, can_stats.rx_overrun);
    // trig = !trig;
}

void pollIMU(void)
{
    imu_periodic(&imu_h);
}

void parseIMU(void)
{
    GPSHandle.messages_received++;
    BMI088_readGyro(&bmi_config, &gyro_in);
    BMI088_readAccel(&bmi_config, &accel_in);
    GPSHandle.acceleration = accel_in;
    GPSHandle.gyroscope = gyro_in;

    /* Update Gyro OK flag */
    if (gyro_counter == 150){
        GPSHandle.gyro_OK = BMI088_gyroOK(&bmi_config);
        gyro_counter = 0;
    } else {
        ++gyro_counter;
    }
}

void usart_recieve_complete_callback(usart_init_t *handle)
{
    if (handle == &usb)
    {
        memcpy(&rxmsg, rxbuffer, sizeof(rxmsg));

        // Data
        memcpy(xVCU.WT_RAW, rxmsg.WT_RAW, sizeof(xVCU.WT_RAW));
        memcpy(xVCU.WM_RAW, rxmsg.WM_RAW, sizeof(xVCU.WM_RAW));
        memcpy(xVCU.AV_RAW, rxmsg.AV_RAW, sizeof(xVCU.AV_RAW));
        memcpy(xVCU.AG_RAW, rxmsg.AG_RAW, sizeof(xVCU.AG_RAW));
        memcpy(xVCU.TO_RAW, rxmsg.TO_RAW, sizeof(xVCU.TO_RAW));

        xVCU.TH_RAW = rxmsg.TH_RAW;
        xVCU.ST_RAW = rxmsg.ST_RAW;
        xVCU.VB_RAW = rxmsg.VB_RAW;
        xVCU.GS_RAW = rxmsg.GS_RAW;
        xVCU.IB_RAW = rxmsg.IB_RAW;
        xVCU.MT_RAW = rxmsg.MT_RAW;
        xVCU.CT_RAW = rxmsg.CT_RAW;
        xVCU.IT_RAW = rxmsg.IT_RAW;
        xVCU.MC_RAW = rxmsg.MC_RAW;
        xVCU.IC_RAW = rxmsg.IC_RAW;
        xVCU.BT_RAW = rxmsg.BT_RAW;
        xVCU.DB_RAW = rxmsg.DB_RAW;
        xVCU.PI_RAW = rxmsg.PI_RAW;
        xVCU.PP_RAW = rxmsg.PP_RAW;

        // Flags
        fVCU.CS_SFLAG = rxmsg.CS_SFLAG;
        fVCU.TB_SFLAG = rxmsg.TB_SFLAG;
        fVCU.SS_SFLAG = rxmsg.SS_SFLAG;
        fVCU.WT_SFLAG = rxmsg.WT_SFLAG;
        fVCU.IV_SFLAG = rxmsg.IV_SFLAG;
        fVCU.BT_SFLAG = rxmsg.BT_SFLAG;
        fVCU.IAC_SFLAG = rxmsg.IAC_SFLAG;
        fVCU.IAT_SFLAG = rxmsg.IAT_SFLAG;
        fVCU.IBC_SFLAG = rxmsg.IBC_SFLAG;
        fVCU.IBT_SFLAG = rxmsg.IBT_SFLAG;
        fVCU.SS_FFLAG = rxmsg.SS_FFLAG;
        fVCU.AV_FFLAG = rxmsg.AV_FFLAG;
        fVCU.GS_FFLAG = rxmsg.GS_FFLAG;
        fVCU.VCU_PFLAG = rxmsg.VCU_PFLAG;
    }
    else 
    {
        parseVelocity(&GPSHandle);
    }

}

/* CAN Message Handling */
void CAN1_RX0_IRQHandler()
{
    canParseIRQHandler(CAN1);
}

void VCU_MAIN(void)
{
    /* Fill in X & F */
    // vcu_pp(&xVCU, &fVCU, &GPSHandle);

    /* Step VCU */
    vcu_step(&pVCU, &fVCU, &xVCU, &yVCU);

    /* Set TV faults */
    setFault(ID_ET_ENABLED_FAULT,(yVCU.VCU_mode==0) || (yVCU.VCU_mode==1));
    setFault(ID_PT_ENABLED_FAULT,(yVCU.VCU_mode==2));
    setFault(ID_VT_ENABLED_FAULT,(yVCU.VCU_mode==3));
    setFault(ID_VS_ENABLED_FAULT,(yVCU.VCU_mode==4));
    setFault(ID_NO_GPS_FIX_FAULT,(fVCU.GS_FFLAG < 3));
    setFault(ID_YES_GPS_FIX_FAULT,(fVCU.GS_FFLAG == 3));

    /* Send messages */
    SEND_VCU_TORQUES_SPEEDS(yVCU.TO_VT[0], yVCU.TO_VT[1], yVCU.TO_PT[0], yVCU.WM_VS[0]);
    SEND_VCU_SOC_ESTIMATE(yVCU.Batt_SOC, yVCU.Batt_Voc);
    SEND_DRIVE_MODES(yVCU.VCU_mode, yVCU.VT_mode);
}

void torquevector_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.torquevector_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_PORT, ERR_LED_PIN, 1);
    while (1)
    {
        __asm__("nop");
    }
}

void txUsart()
{
    memcpy(txmsg.ET_permit_buffer, yVCU.ET_permit_buffer, sizeof(txmsg.ET_permit_buffer));
    memcpy(txmsg.PT_permit_buffer, yVCU.PT_permit_buffer, sizeof(txmsg.PT_permit_buffer));
    memcpy(txmsg.VS_permit_buffer, yVCU.VS_permit_buffer, sizeof(txmsg.VS_permit_buffer));
    memcpy(txmsg.VT_permit_buffer, yVCU.VT_permit_buffer, sizeof(txmsg.VT_permit_buffer));
    memcpy(txmsg.IB_CF_buffer, yVCU.IB_CF_buffer, sizeof(txmsg.IB_CF_buffer));
    memcpy(txmsg.WT_CF, yVCU.WT_CF, sizeof(txmsg.WT_CF));
    memcpy(txmsg.WM_CF, yVCU.WM_CF, sizeof(txmsg.WM_CF));
    memcpy(txmsg.AV_CF, yVCU.AV_CF, sizeof(txmsg.AV_CF));
    memcpy(txmsg.AG_CF, yVCU.AG_CF, sizeof(txmsg.AG_CF));
    memcpy(txmsg.TO_CF, yVCU.TO_CF, sizeof(txmsg.TO_CF));
    memcpy(txmsg.TO_ET, yVCU.TO_ET, sizeof(txmsg.TO_ET));
    memcpy(txmsg.TO_PT, yVCU.TO_PT, sizeof(txmsg.TO_PT));
    memcpy(txmsg.WM_VS, yVCU.WM_VS, sizeof(txmsg.WM_VS));
    memcpy(txmsg.TO_VT, yVCU.TO_VT, sizeof(txmsg.TO_VT));

    txmsg.VCU_mode = yVCU.VCU_mode;
    txmsg.TH_CF = yVCU.TH_CF;
    txmsg.ST_CF = yVCU.ST_CF;
    txmsg.VB_CF = yVCU.VB_CF;
    txmsg.GS_CF = yVCU.GS_CF;
    txmsg.IB_CF = yVCU.IB_CF;
    txmsg.MT_CF = yVCU.MT_CF;
    txmsg.CT_CF = yVCU.CT_CF;
    txmsg.IT_CF = yVCU.IT_CF;
    txmsg.MC_CF = yVCU.MC_CF;
    txmsg.IC_CF = yVCU.IC_CF;
    txmsg.BT_CF = yVCU.BT_CF;
    txmsg.DB_CF = yVCU.DB_CF;
    txmsg.PI_CF = yVCU.PI_CF;
    txmsg.PP_CF = yVCU.PP_CF;
    txmsg.zero_current_counter = yVCU.zero_current_counter;
    txmsg.Batt_SOC = yVCU.Batt_SOC;
    txmsg.Batt_Voc = yVCU.Batt_Voc;
    txmsg.TO_AB_MX = yVCU.TO_AB_MX;
    txmsg.TO_DR_MX = yVCU.TO_DR_MX;
    txmsg.VT_mode = yVCU.VT_mode;
    txmsg.TV_AV_ref = yVCU.TV_AV_ref;
    txmsg.TV_delta_torque = yVCU.TV_delta_torque;
    txmsg.TC_highs = yVCU.TC_highs;
    txmsg.TC_lows = yVCU.TC_lows;
    txmsg.sl = yVCU.sl;
    
    memcpy(txbuffer + 2, &txmsg, sizeof(txmsg));
    PHAL_usartTxBl(&usb, txbuffer, sizeof(txbuffer));
}
