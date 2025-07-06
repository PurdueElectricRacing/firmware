#include <stdint.h>

#include "MadgwickAHRS.h"
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/faults/faults.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/spi.h"
#include "common/phal/usart.h"
#include "common/psched/psched.h"
#include "can/can_parse.h"

#include "main.h"

#include <string.h>
#include "gps.h"
#include "bmi088.h"
#include <math.h>

GPIOInitConfig_t gpio_config[] =
{
    /* Status Indicators */
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),

    /* IMU */
    GPIO_INIT_AF(SPI_SCLK_GPIO_Port, SPI_SCLK_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI_MOSI_GPIO_Port, SPI_MOSI_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(SPI_MISO_GPIO_Port, SPI_MISO_Pin, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, GPIO_OUTPUT_HIGH_SPEED),

    /* GPS */
    GPIO_INIT_UART4RX_PA1,
    GPIO_INIT_UART4TX_PA0,
    GPIO_INIT_OUTPUT(GPS_RESET_GPIO_Port, GPS_RESET_Pin, GPIO_OUTPUT_LOW_SPEED),

    /* USB Logging */
    // GPIO_INIT_USART1TX_PA9,
    // GPIO_INIT_USART1RX_PA10,

    /* VCAN */
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12
};

/* GPS USART */
dma_init_t usart_gps_tx_dma_config = USART4_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_gps_rx_dma_config = USART4_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_gps = {
    .baud_rate        = 115200,
    .word_length      = WORD_8,
    .hw_flow_ctl      = HW_DISABLE,
    .stop_bits        = SB_ONE,
    .parity           = PT_NONE,
    .obsample         = OB_DISABLE,
    .ovsample         = OV_16,
    .periph           = UART4,
    .wake_addr        = false,
    .usart_active_num = USART4_ACTIVE_IDX,
    .tx_errors        = 0,
    .rx_errors        = 0,
    .tx_dma_cfg       = &usart_gps_tx_dma_config,
    .rx_dma_cfg       = &usart_gps_rx_dma_config
};

/* USB USART */
dma_init_t usart_usb_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_usb_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_usb = {
    .baud_rate        = 115200,
    .word_length      = WORD_8,
    .stop_bits        = SB_ONE,
    .parity           = PT_NONE,
    .hw_flow_ctl      = HW_DISABLE,
    .ovsample         = OV_16,
    .obsample         = OB_DISABLE,
    .periph           = USART1,
    .wake_addr        = false,
    .usart_active_num = USART1_ACTIVE_IDX,
    .tx_dma_cfg       = &usart_usb_tx_dma_config,
    .rx_dma_cfg       = &usart_usb_rx_dma_config
};

/* Clock Configuration */
#define TargetCoreClockrateHz 96000000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSE,
    .use_pll                   = true,
    .pll_src                   = PLL_SRC_HSE,
    .vco_output_rate_target_hz = 192000000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / 4),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / 4),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* IMU SPI */
dma_init_t spi_rx_dma_config = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI1_TXDMA_CONT_CONFIG(NULL, 1);

SPI_InitConfig_t spi_config = {
    .data_rate     = TargetCoreClockrateHz / 64,
    .data_len      = 8,
    .nss_sw        = true,
    .nss_gpio_port = SPI_CS_ACEL_GPIO_Port,
    .nss_gpio_pin  = SPI_CS_ACEL_Pin,
    .rx_dma_cfg    = &spi_rx_dma_config,
    .tx_dma_cfg    = &spi_tx_dma_config,
    .periph        = SPI1
};

/* IMU Configuration */
BMI088_Handle_t bmi_handle = {
    .spi                 = &spi_config,
    .accel_range         = ACCEL_RANGE_3G,
    .accel_bwp           = ACCEL_OS_NORMAL,
    .accel_odr           = ACCEL_ODR_50Hz,
    .gyro_range          = GYRO_RANGE_250,
    .gyro_datarate       = GYRO_DR_100Hz_32Hz
};

// Post Filtered state estimate
IMU_data_t state_estimate = {0}; // TODO extend state estimate to more than just IMU data

/* GPS Data */
GPS_Handle_t gps_handle = {0};

/* VCU Data */
static pVCU_struct pVCU;
static fVCU_struct fVCU;
static xVCU_struct xVCU;
static yVCU_struct yVCU;

/* USB UART Data */
static struct serial_tx txmsg;
static struct serial_rx rxmsg;
static uint16_t rxbuffer[(sizeof(rxmsg) + 1) / 2];
static uint8_t txbuffer[2 + sizeof(txmsg)] = {0xAA, 0x55};

/* Function Prototypes */
void heartBeatLED(void);
void preflightAnimation(void);
void preflightChecks(void);
extern void HardFault_Handler(void);

void parseIMU(void);
void VCU_MAIN(void);
void txUsart(void);


int main(void)
{
    /* HAL Initialization */
    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }

    /* GPIO initialization */
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    // TV initialization (will break watchdog)
    pVCU = init_pVCU();
    fVCU = init_fVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 74, 1000);

    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    taskCreate(heartBeatLED, 500);
    taskCreate(heartBeatTask, 100);
    taskCreate(parseIMU, 20);
    taskCreate(VCU_MAIN, 20);

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
        /* VCAN Initialization */
        if (false == PHAL_initCAN(CAN1, false, VCAN_BPS))
        {
            HardFault_Handler();
        }
        NVIC_EnableIRQ(CAN1_RX0_IRQn);
        break;
    case 1:
        /* SPI initialization */
        if (false == PHAL_SPI_init(&spi_config))
        {
            HardFault_Handler();
        }
        spi_config.data_rate = APB2ClockRateHz / 16;
        PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);
        PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
        break;
    case 2:
        /* USART Initialization */
        if (false == PHAL_initUSART(&huart_gps, APB1ClockRateHz))
        {
            HardFault_Handler();
        }
        break;
    case 3:
        /* GPS Initialization */
        PHAL_writeGPIO(GPS_RESET_GPIO_Port, GPS_RESET_Pin, 1);
        PHAL_usartRxDma(&huart_gps, (uint16_t *)(gps_handle.gps_rx_buffer), GPS_RX_BUF_SIZE, 1);
        break;
    case 4:
        /* USB USART */
        // if (!PHAL_initUSART(&usb, APB1ClockRateHz))
        // {
        //     HardFault_Handler();
        // }
        break;
    case 5:
        //PHAL_usartRxDma(&usb, rxbuffer, sizeof(rxbuffer), 1);
        initFaultLibrary(FAULT_NODE_NAME, &q_tx_can[CAN1_IDX][CAN_MAILBOX_HIGH_PRIO], ID_FAULT_SYNC_TORQUE_VECTOR);
        break;
    case 6:
        /* BMI Initialization */
        if (!BMI088_init(&bmi_handle))
        {
            HardFault_Handler();
        }
        break;
    case 9:
        BMI088_wakeAccel(&bmi_handle);
        break;
    // Delay for around 50ms to allow the accelerometer to wake up
    case 63:
        /* Accelerometer Init */
        if (false == BMI088_initAccel(&bmi_handle))
        {
            HardFault_Handler();
        }
        break;
    case 65:
    {
        BMI088_readAccel(&bmi_handle);
        if (bmi_handle.data.accel_x == 0 && bmi_handle.data.accel_y == 0 && bmi_handle.data.accel_z == 0)
        {
            state = 8;
        }
        break;
    }
    default:
        if (state > 66)
        {
            initCANParse();
            registerPreflightComplete(1);
            state = 66; /* prevent wrap around */
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

    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);


    static uint8_t trig;
    if (trig) SEND_TV_CAN_STATS(can_stats.can_peripheral_stats[CAN1_IDX].tx_of,
                                can_stats.can_peripheral_stats[CAN1_IDX].tx_fail,
                                can_stats.rx_of, can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun);
    trig = !trig;
}

// ! Move to BMI088.c ?

static inline void quaternion_to_euler(float q0, float q1, float q2, float q3, float *roll, float *pitch, float *yaw) {
    // Convert quaternion to Euler angles
    *roll  = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2));
    *pitch = asinf(2.0f * (q0 * q2 - q3 * q1));
    *yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3));
}

void parseIMU(void) {
    static int16_t gyro_counter = 0;

    BMI088_readGyro(&bmi_handle);
    BMI088_readAccel(&bmi_handle);

    IMU_data_t data = bmi_handle.data;

    MadgwickAHRSupdateIMU(
        data.gyro_x, data.gyro_y, data.gyro_z,
        data.accel_x, data.accel_y, data.accel_z);

    // Update Gyro OK flag every once in a while
    if (gyro_counter == 150) {
        bmi_handle.isGyroOK = BMI088_gyroOK(&bmi_handle);
        gyro_counter = 0;
    } else {
        ++gyro_counter;
    }
}

void usart_recieve_complete_callback(usart_init_t *handle)
{
    if (handle == &huart_usb)
    {
        ASM_NOP();
        // memcpy(&rxmsg, rxbuffer, sizeof(rxmsg));
        // memcpy(xVCU.WT_RAW, rxmsg.WT_RAW, sizeof(xVCU.WT_RAW));
        // memcpy(xVCU.WM_RAW, rxmsg.WM_RAW, sizeof(xVCU.WM_RAW));
        // memcpy(xVCU.AV_RAW, rxmsg.AV_RAW, sizeof(xVCU.AV_RAW));
        // memcpy(xVCU.AG_RAW, rxmsg.AG_RAW, sizeof(xVCU.AG_RAW));
        // memcpy(xVCU.TO_RAW, rxmsg.TO_RAW, sizeof(xVCU.TO_RAW));

        // xVCU.TH_RAW = rxmsg.TH_RAW;
        // xVCU.ST_RAW = rxmsg.ST_RAW;
        // xVCU.VB_RAW = rxmsg.VB_RAW;
        // xVCU.GS_RAW = rxmsg.GS_RAW;
        // xVCU.IB_RAW = rxmsg.IB_RAW;
        // xVCU.MT_RAW = rxmsg.MT_RAW;
        // xVCU.CT_RAW = rxmsg.CT_RAW;
        // xVCU.IT_RAW = rxmsg.IT_RAW;
        // xVCU.MC_RAW = rxmsg.MC_RAW;
        // xVCU.IC_RAW = rxmsg.IC_RAW;
        // xVCU.BT_RAW = rxmsg.BT_RAW;
        // xVCU.VT_DB_RAW = rxmsg.VT_DB_RAW;
        // xVCU.TC_TR_RAW = rxmsg.TC_TR_RAW;
        // xVCU.TV_PP_RAW = rxmsg.TV_PP_RAW;

        // fVCU.CS_SFLAG = rxmsg.CS_SFLAG;
        // fVCU.TB_SFLAG = rxmsg.TB_SFLAG;
        // fVCU.SS_SFLAG = rxmsg.SS_SFLAG;
        // fVCU.WT_SFLAG = rxmsg.WT_SFLAG;
        // fVCU.IV_SFLAG = rxmsg.IV_SFLAG;
        // fVCU.BT_SFLAG = rxmsg.BT_SFLAG;
        // fVCU.IAC_SFLAG = rxmsg.IAC_SFLAG;
        // fVCU.IAT_SFLAG = rxmsg.IAT_SFLAG;
        // fVCU.IBC_SFLAG = rxmsg.IBC_SFLAG;
        // fVCU.IBT_SFLAG = rxmsg.IBT_SFLAG;
        // fVCU.SS_FFLAG = rxmsg.SS_FFLAG;
        // fVCU.AV_FFLAG = rxmsg.AV_FFLAG;
        // fVCU.GS_FFLAG = rxmsg.GS_FFLAG;
        // fVCU.VCU_PFLAG = rxmsg.VCU_PFLAG;
    }
    else
    {
        GPS_Decode(&gps_handle);
    }
}

void txUsart() {

    /* EDIT HERE TO SEND */
    /* I would call txusart on your own when VCU_MAIN is done */
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
    txmsg.VT_DB_CF = yVCU.VT_DB_CF;
    txmsg.TV_PP_CF = yVCU.TV_PP_CF;
    txmsg.TC_TR_CF = yVCU.TC_TR_CF;
    txmsg.VS_MAX_SR_CF = yVCU.VS_MAX_SR_CF;
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
    txmsg.SR = yVCU.SR;


    /* You shouldn't need to mess with any of this */
    memcpy(txbuffer + 2, &txmsg, sizeof(txmsg));
    PHAL_usartTxDma(&huart_usb, (uint16_t *) txbuffer, 290);
}

/* CAN Message Handling */
void CAN1_RX0_IRQHandler()
{
    canParseIRQHandler(CAN1);
}

void VCU_MAIN(void)
{
    /* Fill in X & F */
    vcu_pp(&fVCU, &xVCU, &gps_handle, &bmi_handle, &state_estimate);

    /* Step VCU */
    vcu_step(&pVCU, &fVCU, &xVCU, &yVCU);

    /* Set VCU faults */
    setFault(ID_ES_ENABLED_FAULT,(yVCU.VCU_mode==0));
    setFault(ID_ET_ENABLED_FAULT,(yVCU.VCU_mode==1));
    setFault(ID_PT_ENABLED_FAULT,(yVCU.VCU_mode==2));
    setFault(ID_VT_ENABLED_FAULT,(yVCU.VCU_mode==3));
    setFault(ID_VS_ENABLED_FAULT,(yVCU.VCU_mode==4));
    setFault(ID_NO_GPS_FIX_FAULT,(fVCU.GS_FFLAG < 3));
    setFault(ID_YES_GPS_FIX_FAULT,(fVCU.GS_FFLAG == 3));

    /* Send VCU messages */

    SEND_VCU_TORQUES_SPEEDS((int16_t)(100*yVCU.TO_VT[0]), (int16_t)(100*yVCU.TO_VT[1]), (int16_t)(100*yVCU.TO_PT[0]), (int8_t)(yVCU.VCU_mode));
    SEND_VCU_SOC_ESTIMATE((int16_t)(100*yVCU.Batt_SOC), (int16_t)(10*yVCU.Batt_Voc));
    SEND_DRIVE_MODES((int8_t)(yVCU.VT_mode), (int16_t)(yVCU.WM_VS[0]));
}

void torquevector_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.torquevector_bl_cmd.cmd == BLCMD_RST)
    {
        Bootloader_ResetForFirmwareDownload();
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while (1)
    {
        __asm__("nop");
    }
}
