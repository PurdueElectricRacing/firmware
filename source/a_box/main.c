/* System Includes */
#include "common/can_library/generated/A_BOX.h"
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/can_library/faults_common.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/psched/psched.h"
#include "stm32f407xx.h"

/* Module Includes */
#include "main.h"
#include "orion.h"
#include "tmu.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // I-Sense
    GPIO_INIT_ANALOG(I_SENSE_CH1_GPIO_Port, I_SENSE_CH1_Pin),
    GPIO_INIT_ANALOG(I_SENSE_CH2_GPIO_Port, I_SENSE_CH2_Pin),

    // VCAN
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,

    // CCAN
    GPIO_INIT_CAN2RX_PB12,
    GPIO_INIT_CAN2TX_PB13,

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
    GPIO_INIT_ANALOG(TMU_1_1_Port, TMU_1_1_Pin),
    GPIO_INIT_ANALOG(TMU_1_2_Port, TMU_1_2_Pin),
    GPIO_INIT_ANALOG(TMU_2_1_Port, TMU_2_1_Pin),
    GPIO_INIT_ANALOG(TMU_2_2_Port, TMU_2_2_Pin),
    GPIO_INIT_ANALOG(TMU_3_1_Port, TMU_3_1_Pin),
    GPIO_INIT_ANALOG(TMU_3_2_Port, TMU_3_2_Pin),
    GPIO_INIT_ANALOG(TMU_4_1_Port, TMU_4_1_Pin),
    GPIO_INIT_ANALOG(TMU_4_2_Port, TMU_4_2_Pin),
    GPIO_INIT_ANALOG(TMU_5_1_Port, TMU_5_1_Pin),
    GPIO_INIT_ANALOG(TMU_5_2_Port, TMU_5_2_Pin),

    // Board Temp Measurement
    GPIO_INIT_ANALOG(BOARD_TEMP_Port, BOARD_TEMP_Pin),

    // 5V Monitoring
    GPIO_INIT_ANALOG(VSENSE_5V_Port, VSENSE_5V_Pin),

    // Orion BMS Comms (external pull up on PCB)
    GPIO_INIT_INPUT(BMS_DISCHARGE_ENABLE_Port, BMS_DISCHARGE_ENABLE_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(BMS_CHARGE_ENABLE_Port, BMS_CHARGE_ENABLE_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(BMS_CHARGER_SAFETY_Port, BMS_CHARGER_SAFETY_Pin, GPIO_INPUT_OPEN_DRAIN)};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSE,
    .use_pll                = false,
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

extern uint8_t orion_error;

extern uint32_t can_mbx_last_send_time[NUM_CAN_PERIPHERALS][CAN_TX_MAILBOX_CNT];

void PHAL_FaultHandler();
extern void HardFault_Handler();

void heartBeatLED();
void monitorStatus();
void preflightChecks();
void preflightAnimation();
void updateTherm();
void sendhbmsg();
void readCurrents();

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6, // Desire ADC clock to be 30MHz (upper bound), clocked from APB2 (160/6=27MHz)
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .adc_number      = 1,
    .dma_mode        = ADC_DMA_CIRCULAR};

volatile ADCReadings_t adc_readings;
ADCChannelConfig_t adc_channel_config[] = {
    {.channel = TMU_1_1_ADC_CHANNEL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_1_2_ADC_CHANNEL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_2_1_ADC_CHANNEL, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_2_2_ADC_CHANNEL, .rank = 4, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_3_1_ADC_CHANNEL, .rank = 5, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_3_2_ADC_CHANNEL, .rank = 6, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_4_1_ADC_CHANNEL, .rank = 7, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_4_2_ADC_CHANNEL, .rank = 8, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_5_1_ADC_CHANNEL, .rank = 9, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = TMU_5_2_ADC_CHANNEL, .rank = 10, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = I_SENSE_CH1_ADC_CHANNEL, .rank = 11, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = I_SENSE_CH2_ADC_CHANNEL, .rank = 12, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&adc_readings,
                                                 sizeof(adc_readings) / sizeof(adc_readings.tmu_1_1),
                                                 0b01);

int main(void) {
    /* Data Struct init */

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        PHAL_FaultHandler();
    }

    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        PHAL_FaultHandler();
    }

    /* ADC and DMA Initialization */
    if (false == PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc_dma_config)) {
        HardFault_Handler();
    }

    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    CAN_library_init();
    orionInit();

    /* Module init */
    schedInit(APB1ClockRateHz * 2); // See Datasheet DS11451 Figure. 4 for clock tree

    /* Task Creation */
    schedInit(SystemCoreClock);
    configureAnim(preflightAnimation, preflightChecks, 75, 750);
    taskCreate(heartBeatLED, 500);
    taskCreate(monitorStatus, 50);
    taskCreate(orionChargePeriodic, 50);
    taskCreate(fault_library_periodic, 100);
    taskCreate(sendhbmsg, 500);
    taskCreate(readCurrents, 50);
    taskCreateBackground(CAN_tx_update);
    taskCreateBackground(CAN_rx_update);

    /* No Way Home */
    schedStart();

    return 0;
}

// *** Startup configuration ***
void preflightChecks(void) {
    static uint16_t state;
    uint8_t charger_speed_def = 0;

    switch (state++) {
        case 0:
            initTMU();
            break;
        case 1:
            break;
        case 2:
            /* Initialize VCAN */
            if (false == PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)) {
                PHAL_FaultHandler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
        case 3:
            /* Initialize CCAN */
            if (false == PHAL_initCAN(CAN2, false, CCAN_BAUD_RATE)) {
                PHAL_FaultHandler();
            }
            NVIC_EnableIRQ(CAN2_RX0_IRQn);
            break;
        default:
            if (state > 750) {
                registerPreflightComplete(1);
                state = 751;
            }
            break;
    }
}

void sendhbmsg() {
    bool imd_status = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);

    CAN_SEND_precharge_hb(imd_status, orion_error);
}

void preflightAnimation(void) {
    static uint32_t time = 0;

    PHAL_writeGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);

    switch (time++ % 3) {
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
void heartBeatLED() {
    if ((sched.os_ticks - last_can_rx_time_ms) >= 500)
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    PHAL_toggleGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin);

    static uint8_t trig;
    if (trig)
        CAN_SEND_a_box_can_stats(can_stats.can_peripheral_stats[CAN1_IDX].tx_of, can_stats.can_peripheral_stats[CAN2_IDX].tx_of, can_stats.can_peripheral_stats[CAN1_IDX].tx_fail, can_stats.can_peripheral_stats[CAN2_IDX].tx_fail, can_stats.rx_of, can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun, can_stats.can_peripheral_stats[CAN2_IDX].rx_overrun);
    trig = !trig;
}

void monitorStatus() {
    uint8_t bms_err, imd_err, tmu_err;
    bms_err = orionErrors();
    tmu_err = readTemps();
    imd_err = !PHAL_readGPIO(IMD_STATUS_GPIO_Port, IMD_STATUS_Pin);

    //    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, !bms_err);

    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, bms_err);

    set_fault(FAULT_INDEX_A_BOX_IMD, imd_err);

    uint8_t stat = bms_err | tmu_err;
    PHAL_writeGPIO(BMS_STATUS_GPIO_Port, BMS_STATUS_Pin, stat);
}

// *** Compulsory CAN Tx/Rx callbacks ***
void a_box_bl_cmd_CALLBACK(can_data_t* p_can_data) {
    if (p_can_data->a_box_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void PHAL_FaultHandler() {
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
    __asm__("bkpt");
    HardFault_Handler();
}

void readCurrents() {
    // storing ADC values to preserve them
    uint16_t adc_isense_1 = adc_readings.isense_ch1;
    uint16_t adc_isense_2 = adc_readings.isense_ch2;
    // Calculating currents from ADC using equation from: https://www.lem.com/sites/default/files/products_datasheets/dhab_s_124.pdf
    float V_offset = 2.5; // offset voltage (V)
    float G1       = 26.7 / 1000; // channel 1 sensitivity (V/A)
    float G2       = 4.0 / 1000; // channel 2 sensitivity (V/A)
    // calculating Vout and converting from 3.3 to 5 based on voltage divider
    float Vout_ch1 = (ADC_VREF / ADC_ADDR_SIZE) * adc_isense_1 * (R1_ISENSE + R2_ISENSE) / R2_ISENSE;
    float Vout_ch2 = (ADC_VREF / ADC_ADDR_SIZE) * adc_isense_2 * (R1_ISENSE + R2_ISENSE) / R2_ISENSE;
    // calculating current, scaling by 17 due to coil turns, multiplying by 100 to send as int over CAN
    int16_t i_ch1 = (Vout_ch1 - V_offset) / G1 * 100;
    int16_t i_ch2 = (Vout_ch2 - V_offset) / G2 * 100;
    // sending currents over CAN
    CAN_SEND_i_sense(i_ch1, i_ch2);
}
