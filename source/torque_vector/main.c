#include "main.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "bmi088.h"
#include "common/can_library/generated/TORQUE_VECTOR.h"
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/can_library/faults_common.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/spi.h"
#include "common/phal/usart.h"
#include "common/psched/psched.h"
#include "gps.h"
#include "vcu.h"

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
        GPIO_INIT_CANTX_PA12};

/* GPS USART */
dma_init_t usart_gps_tx_dma_config = USART4_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_gps_rx_dma_config = USART4_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_gps =
    {
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
        .rx_dma_cfg       = &usart_gps_rx_dma_config};

/* USB USART */
dma_init_t usart_usb_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_usb_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t usb                   = {
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
                      .rx_dma_cfg       = &usart_usb_rx_dma_config};

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
SPI_InitConfig_t spi_config =
    {
        .data_rate     = TargetCoreClockrateHz / 64,
        .data_len      = 8,
        .nss_sw        = true,
        .nss_gpio_port = SPI_CS_ACEL_GPIO_Port,
        .nss_gpio_pin  = SPI_CS_ACEL_Pin,
        .rx_dma_cfg    = &spi_rx_dma_config,
        .tx_dma_cfg    = &spi_tx_dma_config,
        .periph        = SPI1};

/* IMU Configuration */
BMI088_Handle_t bmi_handle = {
    .spi           = &spi_config,
    .accel_range   = ACCEL_RANGE_3G,
    .accel_bwp     = ACCEL_OS_NORMAL,
    .accel_odr     = ACCEL_ODR_50Hz,
    .gyro_range    = GYRO_RANGE_250,
    .gyro_datarate = GYRO_DR_100Hz_32Hz};

/* GPS Data */
GPS_Handle_t gps_handle = {0};

/* IMU Data */
static int16_t gyro_counter = 0;

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
void heartbeat_led(void);
void preflight_animation(void);
void preflightChecks(void);
void reportIMU(void);
void reportGPS(void);
void reportUTC(void);
extern void HardFault_Handler(void);

void parseIMU(void);
void pollIMU(void);
void VCU_MAIN(void);
void txUsart(void);

int main(void) {
    /* HAL Initialization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    /* GPIO initialization */
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // TV initialization (will break watchdog)
    pVCU = init_pVCU();
    fVCU = init_fVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflight_animation, preflightChecks, 74, 1000);

    taskCreateBackground(CAN_tx_update);
    taskCreateBackground(CAN_rx_update);

    taskCreate(heartbeat_led, 500);
    taskCreate(parseIMU, 20);
    taskCreate(VCU_MAIN, 20);
    taskCreate(reportIMU, IMU_ACCEL_PERIOD_MS);
    taskCreate(reportGPS, GPS_SPEED_PERIOD_MS);
    taskCreate(reportUTC, GPS_TIME_PERIOD_MS);

    /* No Way Home */
    schedStart();

    return 0;
}

// these int16s must be global for the can send macros to work
int16_t scaled_accel_x;
int16_t scaled_accel_y;
int16_t scaled_accel_z;

int16_t scaled_gyro_x;
int16_t scaled_gyro_y;
int16_t scaled_gyro_z;

int16_t scaled_speed;
int16_t scaled_heading;

void reportIMU() {
    scaled_accel_x = (int16_t)(bmi_handle.data.accel_x * PACK_COEFF_IMU_ACCEL_IMU_ACCEL_X);
    scaled_accel_y = (int16_t)(bmi_handle.data.accel_y * PACK_COEFF_IMU_ACCEL_IMU_ACCEL_Y);
    scaled_accel_z = (int16_t)(bmi_handle.data.accel_z * PACK_COEFF_IMU_ACCEL_IMU_ACCEL_Z);
    CAN_SEND_imu_accel(scaled_accel_x, scaled_accel_y, scaled_accel_z);

    scaled_gyro_x = (int16_t)(bmi_handle.data.gyro_x * PACK_COEFF_IMU_GYRO_IMU_GYRO_X);
    scaled_gyro_y = (int16_t)(bmi_handle.data.gyro_y * PACK_COEFF_IMU_GYRO_IMU_GYRO_Y);
    scaled_gyro_z = (int16_t)(bmi_handle.data.gyro_z * PACK_COEFF_IMU_GYRO_IMU_GYRO_Z);
    CAN_SEND_imu_gyro(scaled_gyro_x, scaled_gyro_y, scaled_gyro_z);
}

void reportGPS() {
    CAN_SEND_gps_coordinates(gps_handle.data.latitude, gps_handle.data.longitude);

    scaled_speed   = (int16_t)(gps_handle.data.groundSpeed * PACK_COEFF_GPS_SPEED_GPS_SPEED); 
    scaled_heading = (int16_t)(gps_handle.data.headingMotion * PACK_COEFF_GPS_SPEED_GPS_HEADING);
    CAN_SEND_gps_speed(scaled_speed, scaled_heading);
}

void reportUTC() {
    uint8_t abbreviated_year = (uint8_t)(gps_handle.data.year % 100);
    uint8_t millis           = (uint8_t)(gps_handle.data.nano / 1000);
    CAN_SEND_gps_time(
        abbreviated_year,
        gps_handle.data.month,
        gps_handle.data.day,
        gps_handle.data.hour,
        gps_handle.data.minute,
        gps_handle.data.second,
        millis
    );
}

void preflightChecks(void) {
    static uint16_t state;

    switch (state++) {
        case 0:
            /* VCAN Initialization */
            if (false == PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 1:
            /* SPI initialization */
            if (false == PHAL_SPI_init(&spi_config)) {
                HardFault_Handler();
            }
            spi_config.data_rate = APB2ClockRateHz / 16;
            PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);
            PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
            break;
        case 2:
            /* USART Initialization */
            if (false == PHAL_initUSART(&huart_gps, APB1ClockRateHz)) {
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
            break;
        case 6:
            /* BMI Initialization */
            if (!BMI088_init(&bmi_handle)) {
                HardFault_Handler();
            }
            break;
        case 9:
            BMI088_wakeAccel(&bmi_handle);
            break;
        // Delay for around 50ms to allow the accelerometer to wake up
        case 63:
            /* Accelerometer Init */
            if (false == BMI088_initAccel(&bmi_handle)) {
                HardFault_Handler();
            }
            break;
        case 65: {
            BMI088_readAccel(&bmi_handle);
            if (bmi_handle.data.accel_x == 0 && bmi_handle.data.accel_y == 0 && bmi_handle.data.accel_z == 0) {
                state = 8;
            }
            break;
        }
        default:
            if (state > 66) {
                CAN_library_init();
                registerPreflightComplete(1);
                state = 66; /* prevent wrap around */
            }
            break;
    }
}

void preflight_animation(void) {
    static uint32_t time;

    PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 0);
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);

    switch (time++ % 6) {
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

void heartbeat_led(void) {
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);

    if ((sched.os_ticks - can_data.main_hb.last_rx) >= CONN_LED_MS_THRESH)
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    static uint8_t trig;
    if (trig)
        CAN_SEND_tv_can_stats(can_stats.can_peripheral_stats[CAN1_IDX].tx_of,
                              can_stats.can_peripheral_stats[CAN1_IDX].tx_fail,
                              can_stats.rx_of,
                              can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun);
    trig = !trig;
}

void parseIMU(void) {
    static int16_t gyro_counter = 0;

    BMI088_readGyro(&bmi_handle);
    BMI088_readAccel(&bmi_handle);

    // Update Gyro OK flag every once in a while
    if (gyro_counter == 150) {
        bmi_handle.isGyroOK = BMI088_gyroOK(&bmi_handle);
        gyro_counter        = 0;
    } else {
        ++gyro_counter;
    }
}

void usart_recieve_complete_callback(usart_init_t *handle) {
    if (handle == &usb) {
        __asm__("nop");
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
    } else {
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

    txmsg.VCU_mode             = yVCU.VCU_mode;
    txmsg.TH_CF                = yVCU.TH_CF;
    txmsg.ST_CF                = yVCU.ST_CF;
    txmsg.VB_CF                = yVCU.VB_CF;
    txmsg.GS_CF                = yVCU.GS_CF;
    txmsg.IB_CF                = yVCU.IB_CF;
    txmsg.MT_CF                = yVCU.MT_CF;
    txmsg.CT_CF                = yVCU.CT_CF;
    txmsg.IT_CF                = yVCU.IT_CF;
    txmsg.MC_CF                = yVCU.MC_CF;
    txmsg.IC_CF                = yVCU.IC_CF;
    txmsg.BT_CF                = yVCU.BT_CF;
    txmsg.VT_DB_CF             = yVCU.VT_DB_CF;
    txmsg.TV_PP_CF             = yVCU.TV_PP_CF;
    txmsg.TC_TR_CF             = yVCU.TC_TR_CF;
    txmsg.VS_MAX_SR_CF         = yVCU.VS_MAX_SR_CF;
    txmsg.zero_current_counter = yVCU.zero_current_counter;
    txmsg.Batt_SOC             = yVCU.Batt_SOC;
    txmsg.Batt_Voc             = yVCU.Batt_Voc;
    txmsg.TO_AB_MX             = yVCU.TO_AB_MX;
    txmsg.TO_DR_MX             = yVCU.TO_DR_MX;
    txmsg.VT_mode              = yVCU.VT_mode;
    txmsg.TV_AV_ref            = yVCU.TV_AV_ref;
    txmsg.TV_delta_torque      = yVCU.TV_delta_torque;
    txmsg.TC_highs             = yVCU.TC_highs;
    txmsg.TC_lows              = yVCU.TC_lows;
    txmsg.SR                   = yVCU.SR;

    /* You shouldn't need to mess with any of this */
    memcpy(txbuffer + 2, &txmsg, sizeof(txmsg));
    // Send in 20-byte chunks, starting from the 3rd byte in the buffer (since the first two are reserved for the sync code)
    // for (uint16_t i = 0; i < sizeof(txmsg); i += 20) {
    //     uint16_t chunk_size = (i + 20 <= sizeof(txmsg)) ? 20 : sizeof(txmsg) - i;

    //     // Refresh the watchdog to avoid system reset
    //     IWDG->KR = 0xAAAA;

    //     PHAL_usartTxBl(&usb, txbuffer + i, chunk_size);
    // }
    PHAL_usartTxDma(&usb, (uint16_t *)txbuffer, 290);
}

void torquevector_bl_cmd_CALLBACK(can_data_t* p_can_data)
{
    if (p_can_data->torquevector_bl_cmd.cmd == BLCMD_RST)
    {
        Bootloader_ResetForFirmwareDownload();
    }
}

void VCU_MAIN(void) {
    /* Fill in X & F */
    vcu_pp(&fVCU, &xVCU, &gps_handle, &bmi_handle);

    /* Step VCU */
    vcu_step(&pVCU, &fVCU, &xVCU, &yVCU);

    /* Set VCU faults */
    update_fault(FAULT_INDEX_TORQUE_VECTOR_ES_ENABLED, (yVCU.VCU_mode == 0));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_ET_ENABLED, (yVCU.VCU_mode == 1));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_PT_ENABLED, (yVCU.VCU_mode == 2));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_VT_ENABLED, (yVCU.VCU_mode == 3));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_VS_ENABLED, (yVCU.VCU_mode == 4));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_NO_GPS_FIX, (fVCU.GS_FFLAG < 3));
    update_fault(FAULT_INDEX_TORQUE_VECTOR_YES_GPS_FIX, (fVCU.GS_FFLAG == 3));

    /* Send VCU messages */
    CAN_SEND_VCU_torques_speeds((int16_t)(100 * yVCU.TO_VT[0]), (int16_t)(100 * yVCU.TO_VT[1]), (int16_t)(100 * yVCU.TO_PT[0]), (int8_t)(yVCU.VCU_mode));
    CAN_SEND_vcu_soc_estimate((int16_t)(100 * yVCU.Batt_SOC), (int16_t)(10 * yVCU.Batt_Voc));
    CAN_SEND_drive_modes((int8_t)(yVCU.VT_mode), (int16_t)(yVCU.WM_VS[0]));
}

void HardFault_Handler() {
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while (1) {
        __asm__("nop");
    }
}
