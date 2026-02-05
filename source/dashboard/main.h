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

typedef struct {
    uint8_t brake_status;
    uint8_t brake_fail;
} brake_status_t;

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

#define BRK_STAT_TAP_GPIO_Port (GPIOA)
#define BRK_STAT_TAP_Pin       (7)
#define BRK_FAIL_TAP_GPIO_Port (GPIOA)
#define BRK_FAIL_TAP_Pin       (6)

#define BRK1_THR_GPIO_Port (GPIOA)
#define BRK1_THR_Pin       (4)
#define BRK1_THR_ADC_CHNL  (4)
#define BRK2_THR_GPIO_Port (GPIOA)
#define BRK2_THR_Pin       (5)
#define BRK2_THR_ADC_CHNL  (5)

#define DAQ_SWITCH_GPIO_Port (GPIOD)
#define DAQ_SWITCH_Pin       (8)

// CAN
#define VCAN_RX_GPIO_Port (GPIOB)
#define VCAN_RX_Pin       (5)
#define VCAN_TX_GPIO_Port (GPIOB)
#define VCAN_TX_Pin       (6)

// Throttle
#define THTL_1_GPIO_Port (GPIOA)
#define THTL_1_Pin       (2)
#define THTL_1_ADC_CHNL  (2)
#define THTL_2_GPIO_Port (GPIOA)
#define THTL_2_Pin       (3)
#define THTL_2_ADC_CHNL  (3)

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
#define BRK_1_GPIO_Port     (GPIOA)
#define BRK_1_Pin           (0)
#define BRK_1_ADC_CHNL      (0)
#define BRK_2_GPIO_Port     (GPIOA)
#define BRK_2_Pin           (1)
#define BRK_2_ADC_CHNL      (1)
#define BRK_1_DIG_GPIO_Port (GPIOC)
#define BRK_1_DIG_GPIO_Pin  (12)
#define BRK_2_DIG_GPIO_Port (GPIOC)
#define BRK_2_DIG_GPIO_Pin  (13)

// LCD
#define LCD_UART              (USART1)
#define LCD_UART_TX_GPIO_Port (GPIOA)
#define LCD_UART_TX_Pin       (9)
#define LCD_UART_RX_GPIO_Port (GPIOA)
#define LCD_UART_RX_Pin       (10)

// LV Status
#define LV_5V_V_SENSE_GPIO_Port  (GPIOC)
#define LV_5V_V_SENSE_Pin        (2)
#define LV_5V_V_SENSE_ADC_CHNL   (12)
#define LV_3V3_V_SENSE_GPIO_Port (GPIOC)
#define LV_3V3_V_SENSE_Pin       (3)
#define LV_3V3_V_SENSE_ADC_CHNL  (13)
#define LV_12_V_SENSE_GPIO_Port  (GPIOC)
#define LV_12_V_SENSE_Pin        (4)
#define LV_12_V_SENSE_ADC_CHNL   (14)
#define LV_24_V_SENSE_GPIO_Port  (GPIOC)
#define LV_24_V_SENSE_Pin        (5)
#define LV_24_V_SENSE_ADC_CHNL   (15)
#define LV_24_V_FAULT_GPIO_Port  (GPIOC)
#define LV_24_V_FAULT_Pin        (8)
#define LV_5V_SCALE              (0.413F)

// Voltage Sensing Resistors (in kOhms)
static constexpr float    LV_3V3_PULLUP   = 4.3F;
static constexpr float    LV_3V3_PULLDOWN = 10.0F;
static constexpr float    LV_5V_PULLUP    = 4.3F;
static constexpr float    LV_5V_PULLDOWN  = 3.3F;
static constexpr float    LV_12V_PULLUP   = 15.8F;
static constexpr float    LV_12V_PULLDOWN = 3.3F;
static constexpr float    LV_24V_PULLUP   = 47.0F;
static constexpr float    LV_24V_PULLDOWN = 3.3F;


void canTxSendToBack(CanMsgTypeDef_t* msg);
void lcdTxUpdate();

#endif
