/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software to measure pedal state and control the LCD display
 * @version 0.1
 * @date 2022-03-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_DASHBOARD

//STM32L496VGT6

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
    uint16_t b3;
    uint16_t shock_left;
    uint16_t shock_right;
    uint16_t lv_5v_sense;
    uint16_t lv_3v3_sense;
    uint16_t mcu_therm;
} raw_adc_values_t;

volatile extern raw_adc_values_t raw_adc_values;

// Status Indicators
#define CONN_LED_GPIO_Port          (GPIOE)
#define CONN_LED_Pin                (6)
#define CONN_LED_MS_THRESH          (500)
#define HEART_LED_GPIO_Port         (GPIOE)
#define HEART_LED_Pin               (7)
#define ERROR_LED_GPIO_Port         (GPIOE)
#define ERROR_LED_Pin               (5)
#define PRCHG_LED_GPIO_Port         (GPIOE)
#define PRCHG_LED_Pin               (1)
#define IMD_LED_GPIO_Port           (GPIOE)
#define IMD_LED_Pin                 (3)
#define BMS_LED_GPIO_Port           (GPIOE)
#define BMS_LED_Pin                 (2)

#define START_BTN_GPIO_Port         (GPIOE)
#define START_BTN_Pin               (0)

#define BRK_STAT_TAP_GPIO_Port      (GPIOB)
#define BRK_STAT_TAP_Pin            (9)
#define BRK_FAIL_TAP_GPIO_Port      (GPIOB)
#define BRK_FAIL_TAP_Pin            (10)

// CAN
#define VCAN_RX_GPIO_Port           (GPIOD)
#define VCAN_RX_Pin                 (0)
#define VCAN_TX_GPIO_Port           (GPIOD)
#define VCAN_TX_Pin                 (1)

// SPI Peripherals
#define SPI1_SCK_GPIO_Port          (GPIOE)
#define SPI1_SCK_Pin                (13)
#define SPI1_MISO_GPIO_Port         (GPIOE)
#define SPI1_MISO_Pin               (14)
#define SPI1_MOSI_GPIO_Port         (GPIOE)
#define SPI1_MOSI_Pin               (15)

#define EEPROM_nWP_GPIO_Port        (GPIOE)
#define EEPROM_nWP_Pin              (11)
#define EEPROM_NSS_GPIO_Port        (GPIOE)
#define EEPROM_NSS_Pin              (12)

// Throttle
#define THTL_1_GPIO_Port            (GPIOA)
#define THTL_1_Pin                  (4)
#define THTL_1_ADC_CHNL             (9)
#define THTL_2_GPIO_Port            (GPIOA)
#define THTL_2_Pin                  (5)
#define THTL_2_ADC_CHNL             (10)


// Brake
#define BRK_1_GPIO_Port             (GPIOA)
#define BRK_1_Pin                   (2)
#define BRK_1_ADC_CHNL              (7)
#define BRK_2_GPIO_Port             (GPIOA)
#define BRK_2_Pin                   (3)
#define BRK_2_ADC_CHNL              (8)
#define BRK_3_GPIO_Port             (GPIOA)
#define BRK_3_Pin                   (6)
#define BRK_3_ADC_CHNL              (11)

// Motor Controllers
#define MC_L_UART                   (USART3)
#define MC_L_UART_TX_GPIO_Port      (GPIOC)
#define MC_L_UART_TX_GPIO_Pin       (10)
#define MC_L_UART_RX_GPIO_Port      (GPIOC)
#define MC_L_UART_RX_GPIO_Pin       (11)

#define MC_R_UART                   (USART2)
#define MC_R_UART_TX_GPIO_Port      (GPIOD)
#define MC_R_UART_TX_GPIO_Pin       (5)
#define MC_R_UART_RX_GPIO_Port      (GPIOD)
#define MC_R_UART_RX_GPIO_Pin       (6)

// Wheel Speed
#define MOTOR_L_WS_QUAD_TIM         (TIM5)
#define MOTOR_L_WS_A_GPIO_Port      (GPIOA)
#define MOTOR_L_WS_A_Pin            (0)
#define MOTOR_L_WS_B_GPIO_Port      (GPIOA)
#define MOTOR_L_WS_B_Pin            (1)

#define MOTOR_L_WS_PWM_TIM          (TIM1)
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

#define MOTOR_R_WS_PWM_TIM          (TIM8)
#define MOTOR_R_WS_PWM_CH           (1)
#define MOTOR_R_WS_Z_GPIO_Port      (GPIOC)
#define MOTOR_R_WS_Z_Pin            (6)

#define MOTOR_R_WS_ERROR_GPIO_Port  (GPIOB)
#define MOTOR_R_WS_ERROR_Pin        (7)

// Shock Pots
#define SHOCK_POT_L_GPIO_Port       (GPIOC)
#define SHOCK_POT_L_Pin             (2)
#define SHOCK_POT_L_ADC_CH          (3)
#define SHOCK_POT_R_GPIO_Port       (GPIOC)
#define SHOCK_POT_R_Pin             (3)
#define SHOCK_POT_R_ADC_CH          (4)

// Drivetrain
#define DT_GB_THERM_L_GPIO_Port     (GPIOC)
#define DT_GB_THERM_L_Pin           (0)
#define DT_GB_THERM_R_GPIO_Port     (GPIOC)
#define DT_GB_THERM_R_Pin           (1)

// LCD
#define LCD_UART                    (USART1)
#define LCD_UART_TX_GPIO_Port       (GPIOA)
#define LCD_UART_TX_Pin             (9)
#define LCD_UART_RX_GPIO_Port       (GPIOA)
#define LCD_UART_RX_Pin             (10)

// HDD
#define B_OK_GPIO_Port              (GPIOB)
#define B_OK_Pin                    (12)
#define B_DOWN_GPIO_Port            (GPIOB)
#define B_DOWN_Pin                  (13)
#define B_UP_GPIO_Port              (GPIOB)
#define B_UP_Pin                    (14)
#define B_RIGHT_GPIO_Port           (GPIOB)
#define B_RIGHT_Pin                 (15)
#define B_LEFT_GPIO_Port            (GPIOD)
#define B_LEFT_Pin                  (8)
#define B_MUX_0_GPIO_Port           (GPIOD)
#define B_MUX_0_Pin                 (9)
#define B_MUX_1_GPIO_Port           (GPIOD)
#define B_MUX_1_Pin                 (10)
#define B_MUX_2_GPIO_Port           (GPIOD)
#define B_MUX_2_Pin                 (11)
#define B_MUX_3_GPIO_Port           (GPIOD)
#define B_MUX_3_Pin                 (12)
#define B_MUX_4_GPIO_Port           (GPIOD)
#define B_MUX_4_Pin                 (13)
#define B_MUX_DATA_GPIO_Port        (GPIOD)
#define B_MUX_DATA_Pin              (14)

// LV Status
#define LV_5V_V_SENSE_GPIO_Port     (GPIOB)
#define LV_5V_V_SENSE_Pin           (0)
#define LV_5V_V_SENSE_ADC_CHNL      (15)
#define LV_3V3_V_SENSE_GPIO_Port    (GPIOB)
#define LV_3V3_V_SENSE_Pin          (1)
#define LV_3V3_V_SENSE_ADC_CHNL     (16)
#define LV_5V_SCALE                 (0.413F)

//Internal MCU Thermistor
#define MCU_THERM_ADC_CHNL          (17)
#define TS_CAL1_ADDR                ((uint16_t*) 0x1FFF75A8UL)
#define TS_CAL2_ADDR                ((uint16_t*) 0x1FFF75CAUL)
#define TS_CAL1_VAL                 ((int32_t) 30)
#define TS_CAL2_VAL                 ((int32_t) 130)
#define TS_VREF                     (3000UL)
#define ADC_VREF_INT                (3300UL)


#endif