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

/* Status LEDs */
#define CONN_LED_GPIO_Port   (GPIOE)
#define CONN_LED_Pin         (6)
#define CONN_LED_MS_THRESH  (500)
#define HEART_LED_GPIO_Port (GPIOE)
#define HEART_LED_Pin       (7)
#define ERROR_LED_GPIO_Port (GPIOE)
#define ERROR_LED_Pin       (5)
#define PRCHG_LED_GPIO_Port (GPIOE)
#define PRCHG_LED_Pin       (1)
#define IMD_LED_GPIO_Port   (GPIOE)
#define IMD_LED_Pin         (3)
#define BMS_LED_GPIO_Port   (GPIOE)
#define BMS_LED_Pin         (2)

#define START_BTN_GPIO_Port (GPIOE)
#define START_BTN_Pin       (0)

#define BRK_STAT_TAP_GPIO_Port (GPIOB)
#define BRK_STAT_TAP_Pin       (9)
#define BRK_FAIL_TAP_GPIO_Port (GPIOB)
#define BRK_FAIL_TAP_Pin       (10)

/* Throttle */
#define THTL_1_GPIO_Port (GPIOA)
#define THTL_1_Pin       (6)
#define THTL_1_ADC_CHNL  (11)
#define THTL_2_GPIO_Port (GPIOA)
#define THTL_2_Pin       (5)
#define THTL_2_ADC_CHNL  (10)

/* Brake */
#define BRK_1_GPIO_Port (GPIOA)
#define BRK_1_Pin       (0)
#define BRK_1_ADC_CHNL  (5)
#define BRK_2_GPIO_Port (GPIOA)
#define BRK_2_Pin       (1)
#define BRK_2_ADC_CHNL  (6)
#define BRK_3_GPIO_Port (GPIOA)
#define BRK_3_Pin       (4)
#define BRK_3_ADC_CHNL  (9)

/* Wheel */
#define WHEEL_SPI_READ    (0x01)
#define WHEEL_SPI_WRITE   (0x00)
#define WHEEL_SPI_ADDR    (0x40)
#define WHEEL_GPIO_REG    (0x09)
#define BTN_UP_EXP_Pin    (0)
#define BTN_DOWN_EXP_Pin  (1)
#define BTN_RIGHT_EXP_Pin (2)
#define BTN_LEFT_EXP_Pin  (3)
#define BTN_AUX1_EXP_Pin  (4)
#define BTN_AUX2_EXP_Pin  (5)
#define BTN_AUX3_EXP_Pin  (6)

/* SPI */
#define SCK_GPIO_Port     (GPIOB)
#define SCK_Pin           (3)
#define MISO_GPIO_Port    (GPIOB)
#define MISO_Pin          (4)
#define MOSI_GPIO_Port    (GPIOB)
#define MOSI_Pin          (5)
#define CSB_WHL_GPIO_Port (GPIOA)
#define CSB_WHL_Pin       (15)

/* EEPROM */
#define WC_GPIO_Port (GPIOA)
#define WC_Pin       (8)

int do_stuff(void);


#endif