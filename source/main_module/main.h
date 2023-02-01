/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software for controlling SDC based on
 *         faults and cooling based on temperatures
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_

//STM32L496VGT6

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_MAIN_MODULE

// Status Indicators
#define ERR_LED_GPIO_Port           (GPIOE)
#define ERR_LED_Pin                 (7)
#define CONN_LED_GPIO_Port          (GPIOE)
#define CONN_LED_Pin                (8)
#define CONN_LED_MS_THRESH          (500)
#define HEARTBEAT_GPIO_Port         (GPIOE)
#define HEARTBEAT_Pin               (5)
#define BRK_LIGHT_GPIO_Port         (GPIOA)
#define BRK_LIGHT_Pin               (12)
#define BUZZER_GPIO_Port            (GPIOB)
#define BUZZER_Pin                  (2)
#define UNDERGLOW_GPIO_Port         (GPIOB)
#define UNDERGLOW_Pin               (5)

// CAN
#define VCAN_RX_GPIO_Port           (GPIOD)
#define VCAN_RX_Pin                 (0)
#define VCAN_TX_GPIO_Port           (GPIOD)
#define VCAN_TX_Pin                 (1)

#define IMUCAN_RX_GPIO_Port         (GPIOB)
#define IMUCAN_RX_Pin               (12)
#define IMUCAN_TX_GPIO_Port         (GPIOB)
#define IMUCAN_TX_Pin               (13)

// SPI Peripherals
#define SPI1_SCK_GPIO_Port          (GPIOE)
#define SPI1_SCK_Pin                (13)
#define SPI1_MISO_GPIO_Port         (GPIOE)
#define SPI1_MISO_Pin               (14)
#define SPI1_MOSI_GPIO_Port         (GPIOE)
#define SPI1_MOSI_Pin               (15)

#define EEPROM_nWP_GPIO_Port        (GPIOE)
#define EEPROM_nWP_Pin              (6)
#define EEPROM_NSS_GPIO_Port        (GPIOE)
#define EEPROM_NSS_Pin              (10)

#define SD_CARD_NSS_GPIO_Port       (GPIOE)
#define SD_CARD_NSS_Pin             (12)

// Shutdown Circuit
#define SDC_CTRL_GPIO_Port          (GPIOA)
#define SDC_CTRL_Pin                (8)

#define BSPD_TEST_CTRL_GPIO_Port    (GPIOB)
#define BSPD_TEST_CTRL_Pin          (9)

#define V_MC_SENSE_GPIO_Port        (GPIOC)
#define V_MC_SENSE_Pin              (4)
#define V_MC_SENSE_ADC_CHNL         (13)
#define V_BAT_SENSE_GPIO_Port       (GPIOC)
#define V_BAT_SENSE_Pin             (5)
#define V_BAT_SENSE_ADC_CHNL        (14)

#define BMS_STAT_GPIO_Port          (GPIOC)
#define BMS_STAT_Pin                (11)
#define PRCHG_STAT_GPIO_Port        (GPIOC)
#define PRCHG_STAT_Pin              (13)

// Motor Controllers
#define MC_L_UART                   (USART2)
#define MC_L_UART_TX_GPIO_Port      (GPIOD)
#define MC_L_UART_TX_GPIO_Pin       (5)
#define MC_L_UART_RX_GPIO_Port      (GPIOD)
#define MC_L_UART_RX_GPIO_Pin       (6)

#define MC_L_TIM                    (TIM3)
#define MC_L_TIM_CH                 (4)
#define MC_L_PWM_GPIO_Port          (GPIOC)
#define MC_L_PWM_Pin                (9)

#define MC_R_UART                   (USART1)
#define MC_R_UART_TX_GPIO_Port      (GPIOA)
#define MC_R_UART_TX_GPIO_Pin       (9)
#define MC_R_UART_RX_GPIO_Port      (GPIOA)
#define MC_R_UART_RX_GPIO_Pin       (10)

#define MC_R_TIM                    (TIM3)
#define MC_R_TIM_CH                 (3)
#define MC_R_PWM_GPIO_Port          (GPIOC)
#define MC_R_PWM_Pin                (8)

// Wheel Speed
#define MOTOR_L_WS_QUAD_TIM         (TIM5)
#define MOTOR_L_WS_A_GPIO_Port      (GPIOA)
#define MOTOR_L_WS_A_Pin            (0)
#define MOTOR_L_WS_B_GPIO_Port      (GPIOA)
#define MOTOR_L_WS_B_Pin            (1)

#define MOTOR_L_WS_PWM_TIM          (TIM1) // NOTE: SHARED WITH DT_FLOW
#define MOTOR_L_WS_PWM_TIM_CH       (1)
#define MOTOR_L_WS_Z_GPIO_Port      (GPIOE)
#define MOTOR_L_WS_Z_Pin            (9)

#define MOTOR_L_WS_ERROR_GPIO_Port  (GPIOB)
#define MOTOR_L_WS_ERROR_Pin        (6)

#define MOTOR_R_WS_QUAD_TIM         (TIM2)
#define MOTOR_R_WS_A_GPIO_Port      (GPIOA)
#define MOTOR_R_WS_A_Pin            (15)
#define MOTOR_R_WS_B_GPIO_Port      (GPIOB)
#define MOTOR_R_WS_B_Pin            (3)

#define MOTOR_R_WS_PWM_TIM          (TIM8) // NOTE: SHARED WITH BAT_FLOW
#define MOTOR_R_WS_PWM_CH           (1)
#define MOTOR_R_WS_Z_GPIO_Port      (GPIOC)
#define MOTOR_R_WS_Z_Pin            (6)

#define MOTOR_R_WS_ERROR_GPIO_Port  (GPIOB)
#define MOTOR_R_WS_ERROR_Pin        (7)

// Shock Pots
#define SHOCK_POT_L_GPIO_Port       (GPIOA)
#define SHOCK_POT_L_Pin             (5)
#define SHOCK_POT_L_ADC_CHNL        (10)
#define SHOCK_POT_R_GPIO_Port       (GPIOA)
#define SHOCK_POT_R_Pin             (2)
#define SHOCK_POT_R_ADC_CHNL        (7)

// Drivetrain
#define DT_GB_THERM_L_GPIO_Port     (GPIOA)
#define DT_GB_THERM_L_Pin           (4)
#define DT_GB_THERM_L_ADC_CHNL      (9)
#define DT_GB_THERM_R_GPIO_Port     (GPIOA)
#define DT_GB_THERM_R_Pin           (11)

#define DT_THERM_1_GPIO_Port        (GPIOC)
#define DT_THERM_1_Pin              (0)
#define DT_THERM_1_ADC_CHNL         (1)
#define DT_PUMP_CTRL_GPIO_Port      (GPIOB)
#define DT_PUMP_CTRL_Pin            (10)

#define DT_FLOW_RATE_TIM            (TIM1) // NOTE: SHARED WITH L_WS_Z
#define DT_FLOW_RATE_TIM_CH         (2)
#define DT_FLOW_RATE_GPIO_Port      (GPIOE)
#define DT_FLOW_RATE_Pin            (11)

#define DT_FAN_CTRL_TIM             (TIM4)
#define DT_FAN_CTRL_TIM_CH          (2)
#define DT_FAN_CTRL_GPIO_Port       (GPIOD)
#define DT_FAN_CTRL_Pin             (13)

#define DT_FAN_TACK_TIM             (TIM17)
#define DT_FAN_TACK_TIM_CH          (1)
#define DT_FAN_TACK_GPIO_Port       (GPIOE)
#define DT_FAN_TACK_Pin             (1)

// HV Battery
#define BAT_THERM_OUT_GPIO_Port     (GPIOD)
#define BAT_THERM_OUT_Pin           (11)
#define BAT_THERM_IN_GPIO_Port      (GPIOA)
#define BAT_THERM_IN_Pin            (3)
#define BAT_THERM_IN_ADC_CHNL       (8)
#define BAT_PUMP_CTRL_1_GPIO_Port   (GPIOB)
#define BAT_PUMP_CTRL_1_Pin         (14)
#define BAT_PUMP_CTRL_2_GPIO_Port   (GPIOB)
#define BAT_PUMP_CTRL_2_Pin         (15)

#define BAT_FLOW_RATE_TIM           (TIM8) // NOTE: SHARED WITH R_WS_Z
#define BAT_FLOW_RATE_TIM_CH        (CH2)
#define BAT_FLOW_RATE_GPIO_Port     (GPIOC)
#define BAT_FLOW_RATE_Pin           (7)

#define BAT_FAN_CTRL_TIM            (TIM4)
#define BAT_FAN_CTRL_TIM_CH         (1)
#define BAT_FAN_CTRL_GPIO_Port      (GPIOD)
#define BAT_FAN_CTRL_Pin            (12)

#define BAT_FAN_TACK_TIM            (TIM16)
#define BAT_FAN_TACK_TIM_CH         (1)
#define BAT_FAN_TACK_GPIO_Port      (GPIOE)
#define BAT_FAN_TACK_Pin            (0)

// LV Status
#define LV_24V_V_SENSE_GPIO_Port    (GPIOA)
#define LV_24V_V_SENSE_Pin          (6)
#define LV_24V_V_SENSE_ADC_CHNL     (11)
#define LV_24V_I_SENSE_GPIO_Port    (GPIOC)
#define LV_24V_I_SENSE_Pin          (1)
#define LV_24V_I_SENSE_ADC_CHNL     (2)

#define LV_12V_V_SENSE_GPIO_Port    (GPIOA)
#define LV_12V_V_SENSE_Pin          (7)
#define LV_12V_V_SENSE_ADC_CHNL     (12)

#define LV_5V_V_SENSE_GPIO_Port     (GPIOB)
#define LV_5V_V_SENSE_Pin           (0)
#define LV_5V_V_SENSE_ADC_CHNL      (15)
#define LV_5V_I_SENSE_GPIO_Port     (GPIOC)
#define LV_5V_I_SENSE_Pin           (2)
#define LV_5V_I_SENSE_ADC_CHNL      (3)

#define LV_3V3_V_SENSE_GPIO_Port    (GPIOB)
#define LV_3V3_V_SENSE_Pin          (1)
#define LV_3V3_V_SENSE_ADC_CHNL     (16)
#define LV_3V3_PG_GPIO_Port         (GPIOB)
#define LV_3V3_PG_Pin               (8)

#define LV_BAT_STAT_GPIO_Port       (GPIOB)
#define LV_BAT_STAT_Pin             (4)

#endif