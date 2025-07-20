/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Chris McGalliard (cpmcgalliard@gmail.com)
 * @brief  VCU
 * @version 0.1
 * @date 2025-3-5
 *
 * @copyright Copyright (c) 2025
 *
 */

/* -------------------------------------------------------
    System Includes
-------------------------------------------------------- */
#include "common/amk/amk.h"
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/faults/faults.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"

/* -------------------------------------------------------
    Module Includes
-------------------------------------------------------- */
#include "can_parse.h"
#include "car.h"
#include "cooling.h"
#include "daq.h"
#include "main.h"

/* -------------------------------------------------------
    Pin Initialization
-------------------------------------------------------- */
GPIOInitConfig_t gpio_config[] =
    {
        /* Status Indicators */
        GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_INPUT(BRK_BUZZER_STAT_GPIO_Port, BRK_BUZZER_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
        GPIO_INIT_INPUT(READY_TO_DRIVE_STAT_GPIO_Port, READY_TO_DRIVE_STAT_GPIO_Pin, GPIO_INPUT_OPEN_DRAIN),

        /* 2025
    EV.5.11.5 The Tractive System Status Indicator must show when the GLV System is energized
    Condition                               Green Light         Red Light
    No Faults                               Always ON           OFF
    BMS EV.7.3.5 or IMD EV.7.6.5            OFF                 2 Hz to 5 Hz, 50% duty cycle
    */
        GPIO_INIT_OUTPUT(SAFE_STAT_R_GPIO_Port, SAFE_STAT_R_GPIO_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(SAFE_STAT_G_GPIO_Port, SAFE_STAT_G_GPIO_Pin, GPIO_OUTPUT_LOW_SPEED),

        /* CAN */
        GPIO_INIT_CANRX_PA11,
        GPIO_INIT_CANTX_PA12,
        GPIO_INIT_CAN2RX_PB12,
        GPIO_INIT_CAN2TX_PB13,

        /* SDC Control */
        GPIO_INIT_SPI1_SCK_PA5,
        GPIO_INIT_SPI1_MISO_PA6,
        GPIO_INIT_SPI1_MOSI_PA7,
        GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),

        GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(SDC_MUX_S0_GPIO_Port, SDC_MUX_S0_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(SDC_MUX_S1_GPIO_Port, SDC_MUX_S1_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(SDC_MUX_S2_GPIO_Port, SDC_MUX_S2_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(SDC_MUX_S3_GPIO_Port, SDC_MUX_S3_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_INPUT(SDC_MUX_DATA_GPIO_Port, SDC_MUX_DATA_Pin, GPIO_INPUT_OPEN_DRAIN),

        /* HV Bus Information */
        // GPIO_INIT_ANALOG(V_MC_SENSE_GPIO_Port, V_MC_SENSE_Pin),
        // GPIO_INIT_ANALOG(V_BAT_SENSE_GPIO_Port, V_BAT_SENSE_Pin),
        GPIO_INIT_INPUT(BMS_STAT_GPIO_Port, BMS_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),
        GPIO_INIT_INPUT(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin, GPIO_INPUT_OPEN_DRAIN),

        /* Wheel Speeds */
        GPIO_INIT_AF(MOTOR_R_WS_GPIO_Port, MOTOR_R_WS_Pin, MOTOR_R_WS_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
        GPIO_INIT_AF(MOTOR_L_WS_GPIO_Port, MOTOR_L_WS_Pin, MOTOR_L_WS_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),

        /* Shock Pots */
        GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
        GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),

        /* Load Sensors */
        // GPIO_INIT_ANALOG(LOAD_L_GPIO_Port, LOAD_L_Pin),
        // GPIO_INIT_ANALOG(LOAD_R_GPIO_Port, LOAD_R_Pin),

        /* Thermistor Analog Multiplexer */
        GPIO_INIT_OUTPUT(THERM_MUX_S0_GPIO_Port, THERM_MUX_S0_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(THERM_MUX_S1_GPIO_Port, THERM_MUX_S1_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(THERM_MUX_S2_GPIO_Port, THERM_MUX_S2_Pin, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_ANALOG(THERM_MUX_OUT_GPIO_Port, THERM_MUX_OUT_Pin)};

/* -------------------------------------------------------
    Clock Configuration
-------------------------------------------------------- */
#define TargetCoreClockrateHz 144000000
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

ClockRateConfig_t clock_config =
    {
        .clock_source = CLOCK_SOURCE_HSI,
        .use_pll = true,
        .pll_src = PLL_SRC_HSI16,
        .vco_output_rate_target_hz = 288000000,
        .system_clock_target_hz = TargetCoreClockrateHz,
        .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
        .apb1_clock_target_hz = (TargetCoreClockrateHz / 4),
        .apb2_clock_target_hz = (TargetCoreClockrateHz / 4),
};

/* -------------------------------------------------------
    ADC
-------------------------------------------------------- */
ADCInitConfig_t adc_config =
    {
        .clock_prescaler = ADC_CLK_PRESC_6,
        .resolution = ADC_RES_12_BIT,
        .data_align = ADC_DATA_ALIGN_RIGHT,
        .cont_conv_mode = true,
        .dma_mode = ADC_DMA_CIRCULAR,
        .adc_number = 1};

/* -------------------------------------------------------
   With 8 ADC channels, a clock prescaler of 16, and a
   sampling time of 480 cycles per channel, each channel
   gets read every 3.2ms
-------------------------------------------------------- */
volatile ADCReadings_t adc_readings;
ADCChannelConfig_t adc_channel_config[] =
    {
        {.channel = SHOCK_POT_L_ADC_CHNL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
        {.channel = SHOCK_POT_R_ADC_CHNL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480},
        {.channel = THERM_MUX_OUT_ADC_CHNL, .rank = 3, .sampling_time = ADC_CHN_SMP_CYCLES_480},
        // {.channel=LOAD_L_ADC_CHNL,         .rank=4,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
        // {.channel=LOAD_R_ADC_CHNL,         .rank=5,  .sampling_time=ADC_CHN_SMP_CYCLES_480},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t)&adc_readings,
                                                 sizeof(adc_readings) / sizeof(adc_readings.shock_l),
                                                 0b01);

/* -------------------------------------------------------
    Procedures
-------------------------------------------------------- */
void preflightAnimation(void);
void preflightChecks(void);
void heartBeatLED();
void send_fault(uint16_t, bool);
extern void HardFault_Handler();
void interpretLoadSensor(void);
float voltToForce(uint16_t load_read);
void can2Relaycan1(CAN_TypeDef* can_h);
void calibrate_lws(void);

int main(void) {
    /* HAL Initialization */
    PHAL_trimHSI(HSI_TRIM_MAIN_MODULE);
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, 1);

    /* Task Creation */
    schedInit(APB1ClockRateHz);

    /* Preflight */
    configureAnim(preflightAnimation, preflightChecks, 60, 750);

    /* Periodic Tasks */
    taskCreate(coolingPeriodic, 50);
    taskCreate(heartBeatLED, 500);
    taskCreate(monitorSDCPeriodic, 20);
    taskCreate(carHeartbeat, 500);
    taskCreate(carPeriodic, 15);
    taskCreate(interpretLoadSensor, 15);
    taskCreate(updateSDCFaults, 300);
    taskCreate(heartBeatTask, 100);
    taskCreate(send_shockpots, 15);
    taskCreate(update_lights, 500);
    taskCreate(parseMCDataPeriodic, 15);
    taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);

    /* Background Tasks */
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    /* Start all tasks */
    schedStart();

    return 0;
}

/**
 * @brief Resets the steering angle sensor calibration
 *        Call after assembly with wheel centered
 *        Device: Bosch F02U.V02.894-01
 */
void calibrate_lws(void) {
    // To zero the sensor after assembly:
    // Reset calibration with CCW = 5h
    // Start a new calibration with CCW = 3h
    // The sensor can then be used immediately

    // Call function at delay ~3 seconds
    static uint8_t status = 0;
    if (status == 0) {
        SEND_LWS_CONFIG(0x05, 0, 0); // reset cal
        status = 1;
    } else if (status == 1) {
        SEND_LWS_CONFIG(0x03, 0, 0); // start new
        status = 2;
    } else {
        // done
    }
}

void preflightChecks(void) {
    static uint8_t state;

    switch (state++) {
        case 0:
            /* VCAN */
            if (false == PHAL_initCAN(CAN1, false, VCAN_BPS)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 1:
            /* MCAN */
            if (false == PHAL_initCAN(CAN2, false, MCAN_BPS)) {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN2_RX0_IRQn);
            break;
        case 2:
            /* ADC */
            if (false == PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
                HardFault_Handler();
            }
            if (false == PHAL_initDMA(&adc_dma_config)) {
                HardFault_Handler();
            }
            PHAL_startTxfer(&adc_dma_config);
            PHAL_startADC(ADC1);
            break;
        case 3:
            break;
        case 4:
            /* Module Initialization */
            carInit();
            coolingInit();
            break;
        case 5:
            /* Lights off first */
            PHAL_writeGPIO(SAFE_STAT_G_GPIO_Port, SAFE_STAT_G_GPIO_Pin, 0);
            PHAL_writeGPIO(SAFE_STAT_R_GPIO_Port, SAFE_STAT_R_GPIO_Pin, 0);
            initCANParse();
            if (daqInit(&q_tx_can[CAN1_IDX][CAN_MAILBOX_LOW_PRIO])) {
                HardFault_Handler();
            }
            initFaultLibrary(FAULT_NODE_NAME, &q_tx_can[CAN1_IDX][CAN_MAILBOX_HIGH_PRIO], ID_FAULT_SYNC_MAIN_MODULE);
            break;
        case 6:
            for (uint8_t i = 0; i < SDC_MUX_HIGH_IDX; i++) {
                monitorSDCPeriodic();
            }
        default:
            registerPreflightComplete(1);
            state = 255; /* prevent wrap around */
    }
}

void preflightAnimation(void) {
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
        case 4:
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
            break;
        case 2:
        case 3:
            PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
            break;
    }
}

void heartBeatLED(void) {
    static uint8_t trig;
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH) {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    } else {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    }
    // Send every other time (1000 ms)
    if (trig) {
        SEND_MCU_STATUS(sched.skips, (uint8_t)sched.fg_time.cpu_use, (uint8_t)sched.bg_time.cpu_use, sched.error);
    } else {
        SEND_MAIN_MODULE_CAN_STATS(can_stats.can_peripheral_stats[CAN1_IDX].tx_of, can_stats.can_peripheral_stats[CAN2_IDX].tx_of, can_stats.can_peripheral_stats[CAN1_IDX].tx_fail, can_stats.can_peripheral_stats[CAN2_IDX].tx_fail, can_stats.rx_of, can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun, can_stats.can_peripheral_stats[CAN2_IDX].rx_overrun);
    }
    trig = !trig;
}

#define SCALE_F = (1 + (3.4 / 6.6))

float voltToForce(uint16_t load_read) {
    /*
    //Return in newtons
    float v_out_load_l = adc_readings.load_l / 4095 * 3.3;
    float v_out_load_r = adc_readings.load_r / 4095 * 3.3;
    //voltage -> weight
    //V_out = (V_in * R_2) / (R_1 + R_2)
    //Solve for V_in
    //R_1 = 3.4K
    //R_2 = 6.6K
    float v_in_load_l = (v_out_load_l * 10) / 6.6;
    float v_in_load_r = (v_out_load_r * 10) / 6.6;
    //voltage * 100 = mass
    //weight (in newtons) = mass * g
    float force_load_l = v_in_load_l * 100 * g;
    float force_load_r = v_in_load_r * 100 * g;
    */
    float g = 9.8;
    // float val = ((load_read / 4095.0 * 3.3) * 10.0)
    float val = ((load_read / 4095.0 * 3.3) * (1.0 + (3.4 / 6.6)));
    // return ( val / 6.6) * 100.0 * g;
    return val * 100.0 * g;
}

void interpretLoadSensor(void) {
    // float force_load_l = voltToForce(adc_readings.load_l);
    // float force_load_r = voltToForce(adc_readings.load_r);
    // SEND_LOAD_SENSOR_READINGS(force_load_l, force_load_r);
}

/* CAN Message Handling */

extern q_handle_t q_tx_can2_s[CAN_TX_MAILBOX_CNT];
extern uint32_t can2_mbx_last_send_time[CAN_TX_MAILBOX_CNT];

void can2TxUpdate(void) {
    CanMsgTypeDef_t tx_msg;
    for (uint8_t i = 0; i < CAN_TX_MAILBOX_CNT; ++i) {
        if (PHAL_txMailboxFree(CAN2, i)) {
            if (qReceive(&q_tx_can2_s[i], &tx_msg) == SUCCESS_G) // Check queue for items and take if there is one
            {
                PHAL_txCANMessage(&tx_msg, i);
                can2_mbx_last_send_time[i] = sched.os_ticks;
            }
        } else if (sched.os_ticks - can2_mbx_last_send_time[i] > CAN_TX_TIMEOUT_MS) {
            PHAL_txCANAbort(CAN2, i); // aborts tx and empties the mailbox
            can_stats.can_peripheral_stats[CAN2_IDX].tx_fail++;
        }
    }
}

void CAN1_RX0_IRQHandler() {
    canParseIRQHandler(CAN1);
}

void CAN2_RX0_IRQHandler() {
    can2Relaycan1(CAN2);
}

/* Override canParseIRQHandler base method to add CAN2 passthrough */
void can2Relaycan1(CAN_TypeDef* can_h) {
    can_peripheral_stats_t* rx_stats = (can_h == CAN1) ? (&can_stats.can_peripheral_stats[CAN1_IDX]) : (&can_stats.can_peripheral_stats[CAN2_IDX]);
    if (can_h->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
    {
        can_h->RF0R |= CAN_RF0R_FOVR0;
        rx_stats->rx_overrun++;
    }

    if (can_h->RF0R & CAN_RF0R_FULL0) // FIFO Full
    {
        can_h->RF0R |= CAN_RF0R_FULL0;
    }

    if (can_h->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        CanMsgTypeDef_t rx;
        rx.Bus = can_h;

        // Get either StdId or ExtId
        rx.IDE = CAN_RI0R_IDE & can_h->sFIFOMailBox[0].RIR;
        if (rx.IDE) {
            rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & can_h->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
        } else {
            rx.StdId = (CAN_RI0R_STID & can_h->sFIFOMailBox[0].RIR) >> CAN_RI0R_STID_Pos;
            rx.ExtId = rx.StdId; // for can_parse (assumes all are ExtId)
        }

        rx.DLC = (CAN_RDT0R_DLC & can_h->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;

        rx.Data[0] = (uint8_t)(can_h->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
        rx.Data[1] = (uint8_t)(can_h->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        rx.Data[2] = (uint8_t)(can_h->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t)(can_h->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t)(can_h->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
        rx.Data[5] = (uint8_t)(can_h->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
        rx.Data[6] = (uint8_t)(can_h->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t)(can_h->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        can_h->RF0R |= (CAN_RF0R_RFOM0);
        can_h->RF0R |= (CAN_RF0R_RFOM0);

        if (qSendToBack(&q_rx_can, &rx) != SUCCESS_G) {
            can_stats.rx_of++;
        }

        /* Pass through CAN2 messages onto CAN1 */
        rx.Bus = CAN1; // Override
        canTxSendToBack(&rx);
    }
}

void main_module_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a) {
    if (can_data.main_module_bl_cmd.cmd == BLCMD_RST) {
        Bootloader_ResetForFirmwareDownload();
    }
}

void HardFault_Handler() {
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while (1) {
        __asm__("nop");
    }
}
