/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software to measure pedal state and control the LCD display
 * @version 24.1
 * @date 2023-12-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_DASHBOARD

//STM32F407VGT6

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
    uint16_t shock_left;
    uint16_t shock_right;
    uint16_t load_left;
    uint16_t load_right;
    uint16_t lv_24v_sense;
    uint16_t lv_12v_sense;
    uint16_t lv_5v_sense;
    uint16_t lv_3v3_sense;
    uint16_t mcu_therm;
} raw_adc_values_t;

volatile extern raw_adc_values_t raw_adc_values;

// Status Indicators
#define CONN_LED_GPIO_Port          (GPIOE)
#define CONN_LED_Pin                (8)
#define CONN_LED_MS_THRESH          (500)
#define HEART_LED_GPIO_Port         (GPIOE)
#define HEART_LED_Pin               (9)
#define ERROR_LED_GPIO_Port         (GPIOE)
#define ERROR_LED_Pin               (7)
#define PRCHG_LED_GPIO_Port         (GPIOE)
#define PRCHG_LED_Pin               (1)
#define IMD_LED_GPIO_Port           (GPIOE)
#define IMD_LED_Pin                 (2)
#define BMS_LED_GPIO_Port           (GPIOE)
#define BMS_LED_Pin                 (3)

#define START_BTN_GPIO_Port         (GPIOD)
#define START_BTN_Pin               (11)

#define BRK_STAT_TAP_GPIO_Port      (GPIOA)
#define BRK_STAT_TAP_Pin            (7)
#define BRK_FAIL_TAP_GPIO_Port      (GPIOA)
#define BRK_FAIL_TAP_Pin            (6)

// CAN
#define VCAN_RX_GPIO_Port           (GPIOD)
#define VCAN_RX_Pin                 (0)
#define VCAN_TX_GPIO_Port           (GPIOD)
#define VCAN_TX_Pin                 (1)

// SPI Peripherals
#define SPI2_SCK_GPIO_Port          (GPIOB)
#define SPI2_SCK_Pin                (13)
#define SPI2_MISO_GPIO_Port         (GPIOB)
#define SPI2_MISO_Pin               (14)
#define SPI2_MOSI_GPIO_Port         (GPIOB)
#define SPI2_MOSI_Pin               (15)

#define EEPROM_nWP_GPIO_Port        (GPIOB)
#define EEPROM_nWP_Pin              (11)
#define EEPROM_NSS_GPIO_Port        (GPIOB)
#define EEPROM_NSS_Pin              (12)

// Throttle
#define THTL_1_GPIO_Port            (GPIOA)
#define THTL_1_Pin                  (2)
#define THTL_1_ADC_CHNL             (2)
#define THTL_2_GPIO_Port            (GPIOA)
#define THTL_2_Pin                  (3)
#define THTL_2_ADC_CHNL             (3)


// Brake
#define BRK_1_GPIO_Port             (GPIOA)
#define BRK_1_Pin                   (0)
#define BRK_1_ADC_CHNL              (0)
#define BRK_2_GPIO_Port             (GPIOA)
#define BRK_2_Pin                   (1)
#define BRK_2_ADC_CHNL              (1)

// Shock Pots
#define SHOCK_POT_L_GPIO_Port       (GPIOC)
#define SHOCK_POT_L_Pin             (0)
#define SHOCK_POT_L_ADC_CH          (10)
#define SHOCK_POT_R_GPIO_Port       (GPIOC)
#define SHOCK_POT_R_Pin             (1)
#define SHOCK_POT_R_ADC_CH          (11)

// Normal Force
#define LOAD_FL_GPIO_Port           (GPIOB)
#define LOAD_FL_Pin                 (0)
#define LOAD_FL_ADC_CH              (8)
#define LOAD_FR_GPIO_Port           (GPIOB)
#define LOAD_FR_Pin                 (1)
#define LOAD_FR_ADC_CH              (9)

// LCD
#define LCD_UART                    (USART1)
#define LCD_UART_TX_GPIO_Port       (GPIOA)
#define LCD_UART_TX_Pin             (9)
#define LCD_UART_RX_GPIO_Port       (GPIOA)
#define LCD_UART_RX_Pin             (10)

// Dashboard Input
#define DAQ_SW_GPIO_Port            (GPIOD)
#define DAQ_SW_Pin                  (8)

#define ENC_A_GPIO_Port             (GPIOD)
#define ENC_A_Pin                   (10)
#define ENC_B_GPIO_Port             (GPIOD)
#define ENC_B_Pin                   (9)

#define B_OK_GPIO_Port              (GPIOD)
#define B_OK_Pin                    (14)
#define B_LEFT_GPIO_Port            (GPIOD)
#define B_LEFT_Pin                  (13)
#define B_RIGHT_GPIO_Port           (GPIOD)
#define B_RIGHT_Pin                 (12)

// LV Status
#define LV_24V_SENSE_GPIO_Port      (GPIOC)
#define LV_24V_SENSE_Pin            (5)
#define LV_24V_SENSE_ADC_CHNL       (15)
#define LV_12V_SENSE_GPIO_Port      (GPIOC)
#define LV_12V_SENSE_Pin            (4)
#define LV_12V_SENSE_ADC_CHNL       (14)
#define LV_5V_SENSE_GPIO_Port       (GPIOC)
#define LV_5V_SENSE_Pin             (2)
#define LV_5V_SENSE_ADC_CHNL        (12)
#define LV_3V3_SENSE_GPIO_Port      (GPIOC)
#define LV_3V3_SENSE_Pin            (3)
#define LV_3V3_SENSE_ADC_CHNL       (13)

//Internal MCU Thermistor
#define MCU_THERM_ADC_CHNL          (17)
#define TS_CAL1_ADDR                ((uint16_t*) 0x1FFF75A8UL)
#define TS_CAL2_ADDR                ((uint16_t*) 0x1FFF75CAUL)
#define TS_CAL1_VAL                 ((int32_t) 30)
#define TS_CAL2_VAL                 ((int32_t) 130)
#define TS_VREF                     (3000UL)
#define ADC_VREF_INT                (3300UL)

#endif