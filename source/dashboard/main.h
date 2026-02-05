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

#include "common/phal/can.h"

//STM32F407

// LCD Constants
#define LCD_NUM_PAGES (9) // Number encoder selectable pages
#define LCD_BAUD_RATE (115200)

typedef struct __attribute__((packed)) {
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
    uint16_t brake1_pressure;
    uint16_t brake2_pressure;
} raw_adc_values_t;
volatile extern raw_adc_values_t raw_adc_values;

typedef struct {
    uint8_t update_page;
    uint8_t up_button;
    uint8_t down_button;
    uint8_t left_button;
    uint8_t right_button;
    uint8_t select_button;
    uint8_t start_button;
} dashboard_input_state_t;

// Status LED Indicators
#define CONN_LED_GPIO_Port  (GPIOB)
#define CONN_LED_Pin        (7)
#define CONN_LED_MS_THRESH  (500)
#define HEART_LED_GPIO_Port (GPIOB)
#define HEART_LED_Pin       (9)
#define ERROR_LED_GPIO_Port (GPIOD)
#define ERROR_LED_Pin       (2)
#define PRCHG_LED_GPIO_Port (GPIOC)
#define PRCHG_LED_Pin       (4)
#define IMD_LED_GPIO_Port   (GPIOA)
#define IMD_LED_Pin         (7)
#define BMS_LED_GPIO_Port   (GPIOA)
#define BMS_LED_Pin         (6)

// Status Inputs
#define START_BTN_GPIO_Port (GPIOB)
#define START_BTN_Pin       (14)

#define BRK1_THR_GPIO_Port (GPIOC)
#define BRK1_THR_Pin       (3)
#define BRK1_THR_ADC_CHNL  (9)
#define BRK2_THR_GPIO_Port (GPIOA)
#define BRK2_THR_Pin       (0)
#define BRK2_THR_ADC_CHNL  (1)

// CAN
#define VCAN_RX_GPIO_Port (GPIOB)
#define VCAN_RX_Pin       (5)
#define VCAN_TX_GPIO_Port (GPIOB)
#define VCAN_TX_Pin       (6)

// Throttle
#define THTL_1_GPIO_Port (GPIOC)
#define THTL_1_Pin       (2)
#define THTL_1_ADC_CHNL  (8)
#define THTL_2_GPIO_Port (GPIOC)
#define THTL_2_Pin       (1)
#define THTL_2_ADC_CHNL  (7)

// Aux Button inputs
#define B_UP_GPIO_Port     (GPIOC)
#define B_UP_Pin           (6)
#define B_DOWN_GPIO_Port   (GPIOC)
#define B_DOWN_Pin         (7)
#define B_RIGHT_GPIO_Port  (GPIOC)
#define B_RIGHT_Pin        (8)
#define B_LEFT_GPIO_Port   (GPIOC)
#define B_LEFT_Pin         (9)
#define B_SELECT_GPIO_Port (GPIOB)
#define B_SELECT_Pin       (15)

// Brake
#define BRK_1_GPIO_Port     (GPIOC)
#define BRK_1_Pin           (3)
#define BRK_1_ADC_CHNL      (9)
#define BRK_2_GPIO_Port     (GPIOA)
#define BRK_2_Pin           (0)
#define BRK_2_ADC_CHNL      (1)

#define BRAKE1_PRESSURE_PORT        (GPIOA)
#define BRAKE1_PRESSURE_PIN         (1)
#define BRAKE1_PRESSURE_ADC_CHANNEL (2)
#define BRAKE2_PRESSURE_PORT        (GPIOA)
#define BRAKE2_PRESSURE_PIN         (2)
#define BRAKE2_PRESSURE_ADC_CHANNEL (3)

// LCD
#define LCD_UART              (USART1)
#define LCD_UART_TX_GPIO_Port (GPIOA)
#define LCD_UART_TX_Pin       (9)
#define LCD_UART_RX_GPIO_Port (GPIOA)
#define LCD_UART_RX_Pin       (10)

void lcdTxUpdate();

#endif
