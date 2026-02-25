/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software for controlling vehicle power
 *         distribution and monitoring
 * @version 0.1
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_

//STM32F407VGT6

#include "common/phal/can.h"

// Status Indicators
#define ERR_LED_GPIO_Port   (GPIOC)
#define ERR_LED_Pin         (13)
#define CONN_LED_GPIO_Port  (GPIOC)
#define CONN_LED_Pin        (14)
#define CONN_LED_MS_THRESH  (500)
#define HEARTBEAT_GPIO_Port (GPIOC)
#define HEARTBEAT_Pin       (15)

// CAN
#define VCAN_RX_GPIO_Port (GPIOD)
#define VCAN_RX_Pin       (0)
#define VCAN_TX_GPIO_Port (GPIOD)
#define VCAN_TX_Pin       (1)

// MUX Control
#define MUX_CTRL_A_GPIO_Port (GPIOB)
#define MUX_CTRL_A_Pin       (12)
#define MUX_CTRL_B_GPIO_Port (GPIOE)
#define MUX_CTRL_B_Pin       (10)
#define MUX_CTRL_C_GPIO_Port (GPIOB)
#define MUX_CTRL_C_Pin       (2)
#define MUX_OUT_GPIO_Port    (GPIOB)
#define MUX_OUT_Pin          (1)

// LED CTRL
#define SPI1_SCK_GPIO_Port  (GPIOB)
#define SPI1_SCK_Pin        (3)
#define SPI1_MOSI_GPIO_Port (GPIOB)
#define SPI1_MOSI_Pin       (5)

#define LED_CTRL_LAT_GPIO_Port   (GPIOD)
#define LED_CTRL_LAT_Pin         (7)
#define LED_CTRL_BLANK_GPIO_Port (GPIOB)
#define LED_CTRL_BLANK_Pin       (6)

// NOTE: Schematic refers to coolant hardware as 1 and 2
// MAPPING
// 1 = Battery Cooling
// 2 = Drivetrain Cooling

// Flow Rate
#define FLOW_RATE_1_TIM       (TIM3)
#define FLOW_RATE_1_TIM_CH    (1)
#define FLOW_RATE_1_GPIO_Port (GPIOC)
#define FLOW_RATE_1_Pin       (6)
#define FLOW_RATE_1_AF        (2)

#define FLOW_RATE_2_TIM       (TIM8)
#define FLOW_RATE_2_TIM_CH    (2)
#define FLOW_RATE_2_GPIO_Port (GPIOC)
#define FLOW_RATE_2_Pin       (7)
#define FLOW_RATE_2_AF        (3)

// Fan Control
#define FAN_1_PWM_TIM       (TIM1)
#define FAN_1_PWM_TIM_CH    (1)
#define FAN_1_PWM_GPIO_Port (GPIOE)
#define FAN_1_PWM_Pin       (9)
#define FAN_1_PWM_AF        (1)

#define FAN_2_PWM_TIM       (TIM1)
#define FAN_2_PWM_TIM_CH    (2)
#define FAN_2_PWM_GPIO_Port (GPIOE)
#define FAN_2_PWM_Pin       (11)
#define FAN_2_PWM_AF        (1)

#define FAN_3_PWM_TIM       (TIM1)
#define FAN_3_PWM_TIM_CH    (3)
#define FAN_3_PWM_GPIO_Port (GPIOE)
#define FAN_3_PWM_Pin       (13)
#define FAN_3_PWM_AF        (1)

#define FAN_4_PWM_TIM       (TIM1)
#define FAN_4_PWM_TIM_CH    (4)
#define FAN_4_PWM_GPIO_Port (GPIOA)
#define FAN_4_PWM_Pin       (11)
#define FAN_4_PWM_AF        (1)

#define FAN_1_TACH_TIM       (TIM4)
#define FAN_1_TACH_TIM_CH    (2)
#define FAN_1_TACH_GPIO_Port (GPIOB)
#define FAN_1_TACH_Pin       (7)
#define FAN_1_TACH_AF        (2)

#define FAN_2_TACH_TIM       (TIM10)
#define FAN_2_TACH_TIM_CH    (1)
#define FAN_2_TACH_GPIO_Port (GPIOB)
#define FAN_2_TACH_Pin       (8)
#define FAN_2_TACH_AF        (3)

#define FAN_3_TACH_TIM       (TIM4)
#define FAN_3_TACH_TIM_CH    (4)
#define FAN_3_TACH_GPIO_Port (GPIOB)
#define FAN_3_TACH_Pin       (9)
#define FAN_3_TACH_AF        (2)

#define FAN_4_TACH_TIM       (TIM12)
#define FAN_4_TACH_TIM_CH    (1)
#define FAN_4_TACH_GPIO_Port (GPIOB)
#define FAN_4_TACH_Pin       (14)
#define FAN_4_TACH_AF        (9)

// Pump Switches (High Power)
#define PUMP_1_CTRL_GPIO_Port (GPIOB)
#define PUMP_1_CTRL_Pin       (10)
#define PUMP_1_IMON_GPIO_Port (GPIOA)
#define PUMP_1_IMON_Pin       (6)
#define PUMP_1_IMON_ADC_CHNL  (6)

#define PUMP_2_CTRL_GPIO_Port (GPIOE)
#define PUMP_2_CTRL_Pin       (15)
#define PUMP_2_IMON_GPIO_Port (GPIOA)
#define PUMP_2_IMON_Pin       (5)
#define PUMP_2_IMON_ADC_CHNL  (5)

// Heat Exchanger Fan (High Power)
#define HXFAN_CTRL_GPIO_Port (GPIOD)
#define HXFAN_CTRL_Pin       (12)
#define HXFAN_IMON_GPIO_Port (GPIOC)
#define HXFAN_IMON_Pin       (3)
#define HXFAN_IMON_ADC_CHNL  (13)

// Shutdown Circuit (SDC) Switch (High Power)
#define SDC_IMON_GPIO_Port (GPIOA)
#define SDC_IMON_Pin       (0)
#define SDC_IMON_ADC_CHNL  (0)

// Fan Switches (Low Power)
#define FAN_3_CTRL_GPIO_Port (GPIOA)
#define FAN_3_CTRL_Pin       (9)

#define FAN_4_CTRL_GPIO_Port (GPIOA)
#define FAN_4_CTRL_Pin       (12)

#define FAN_1_CTRL_GPIO_Port (GPIOC)
#define FAN_1_CTRL_Pin       (8)

#define FAN_2_CTRL_GPIO_Port (GPIOC)
#define FAN_2_CTRL_Pin       (9)

// Driveline Current Sense
#define DLFR_CS_GPIO_Port (GPIOA)
#define DLFR_CS_Pin       (2)
#define DLFR_CS_ADC_CHNL  (2)
#define DLBK_CS_GPIO_Port (GPIOA)
#define DLBK_CS_Pin       (3)
#define DLBK_CS_ADC_CHNL  (3)

// Driveline Controls
#define DLFR_CTRL_GPIO_Port (GPIOD)
#define DLFR_CTRL_Pin       (9)
#define DLFR_NFLT_GPIO_Port (GPIOD)
#define DLFR_NFLT_Pin       (8)
#define DLBK_CTRL_GPIO_Port (GPIOD)
#define DLBK_CTRL_Pin       (11)
#define DLBK_NFLT_GPIO_Port (GPIOD)
#define DLBK_NFLT_Pin       (10)

// Main Module Switch (Low Power)
#define MAIN_CTRL_GPIO_Port (GPIOD)
#define MAIN_CTRL_Pin       (15)
#define MAIN_NFLT_GPIO_Port (GPIOD)
#define MAIN_NFLT_Pin       (13)
#define MAIN_CS_GPIO_Port   (GPIOC)
#define MAIN_CS_Pin         (2)
#define MAIN_CS_ADC_CHNL    (12)

// Dashboard Switch (Low Power)
#define DASH_NFLT_GPIO_Port (GPIOA)
#define DASH_NFLT_Pin       (8)
#define DASH_CS_GPIO_Port   (GPIOC)
#define DASH_CS_Pin         (1)
#define DASH_CS_ADC_CHNL    (11)

// Accumulator (ABox) Switch (Low Power)
#define ABOX_NFLT_GPIO_Port (GPIOA)
#define ABOX_NFLT_Pin       (10)
#define ABOX_CS_GPIO_Port   (GPIOC)
#define ABOX_CS_Pin         (0)
#define ABOX_CS_ADC_CHNL    (10)

// Bullet (Antennae) Switch (Low Power)
#define BLT_CTRL_GPIO_Port (GPIOE)
#define BLT_CTRL_Pin       (12)
#define BLT_NFLT_GPIO_Port (GPIOE)
#define BLT_NFLT_Pin       (14)

// 5V Critical Switch (5V)
#define CRIT_5V_CTRL_GPIO_Port (GPIOE)
#define CRIT_5V_CTRL_Pin       (1)
#define CRIT_5V_NFLT_GPIO_Port (GPIOE)
#define CRIT_5V_NFLT_Pin       (0)

// TV Switch (5V)
#define TV_CTRL_GPIO_Port (GPIOE)
#define TV_CTRL_Pin       (5)
#define TV_NFLT_GPIO_Port (GPIOE)
#define TV_NFLT_Pin       (4)

// DAQ Switch (5V)
#define DAQ_NFLT_GPIO_Port (GPIOE)
#define DAQ_NFLT_Pin       (2)

// 5V Fan Switch (5V)
#define FAN_5V_CTRL_GPIO_Port (GPIOE)
#define FAN_5V_CTRL_Pin       (8)
#define FAN_5V_NFLT_GPIO_Port (GPIOE)
#define FAN_5V_NFLT_Pin       (7)

// LV Battery BMS
#define LV_BMS_STAT_GPIO_Port (GPIOA)
#define LV_BMS_STAT_Pin       (15)
#define LV_BMS_TX_GPIO_Port   (GPIOC)
#define LV_BMS_TX_Pin         (10)
#define LV_BMS_RX_GPIO_Port   (GPIOC)
#define LV_BMS_RX_Pin         (11)

// Voltage/Current Sense
#define V24_VS_GPIO_Port (GPIOB)
#define V24_VS_Pin       (0)
#define V24_VS_ADC_CHNL  (8)

#define MUX_OUT_ADC_CHNL (9)

#define V24_CS_GPIO_Port (GPIOA)
#define V24_CS_Pin       (4)
#define V24_CS_ADC_CHNL  (4)
#define V5_CS_GPIO_Port  (GPIOA)
#define V5_CS_Pin        (1)
#define V5_CS_ADC_CHNL   (1)

#define V5_VS_GPIO_Port   (GPIOC)
#define V5_VS_Pin         (5)
#define V5_VS_ADC_CHNL    (15)
#define V3V3_VS_GPIO_Port (GPIOC)
#define V3V3_VS_Pin       (4)
#define V3V3_VS_ADC_CHNL  (14)

#define DAQ_IMON_GPIO_Port      (GPIOA)
#define DAQ_IMON_Pin            (7)
#define DAQ_IMON_ADC_CHNL       (7)
#define INTERNAL_THERM_ADC_CHNL (16)

// ADC Configuration
#define ADC_REF_mV (3300UL) // mV

typedef struct {
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.c to match
    uint16_t pump_1_imon;
    uint16_t pump_2_imon;
    uint16_t hxfan_imon;
    uint16_t sdc_imon;
    uint16_t dlfr_cs;
    uint16_t dlbk_cs;
    uint16_t main_cs;
    uint16_t dash_cs;
    uint16_t abox_cs;
    uint16_t daq_imon;
    uint16_t v24_vs;
    uint16_t v24_cs;
    uint16_t v5_vs;
    uint16_t v5_cs;
    uint16_t v3v3_vs;
    uint16_t internal_therm;
    uint16_t mux_out;
} __attribute__((packed)) ADCReadings_t;

volatile extern ADCReadings_t adc_readings;

void canTxSendToBack(CanMsgTypeDef_t *msg);
#endif
