/**
 * @file main.h
 * @brief "Abox" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOB)
#define HEARTBEAT_LED_PIN   (5)
#define ERROR_LED_PORT      (GPIOB)
#define ERROR_LED_PIN       (9)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (4)

// SPI1 for BMS
#define SPI1_SCK_PORT  (GPIOA)
#define SPI1_MISO_PORT (GPIOA)
#define SPI1_MOSI_PORT (GPIOA)
#define SPI1_SCK_PIN   (5)
#define SPI1_MISO_PIN  (6)
#define SPI1_MOSI_PIN  (7)
#define SPI1_CS_PORT (GPIOB)
#define SPI1_CS_PIN  (0)

// ISENSE and VSENSE
#define ISENSE_GPIO_PORT   (GPIOA)
#define ISENSE_GPIO_PIN    (0)
#define ISENSE_ADC_CHANNEL (1)
#define VBATT_GPIO_PORT    (GPIOA)
#define VBATT_GPIO_PIN     (1)
#define VBATT_ADC_CHANNEL  (2)

// Input status pins
#define CHARGER_CONNECTED_PORT (GPIOB)
#define CHARGER_CONNECTED_PIN  (2)
#define NOT_PRECHARGE_COMPLETE_PORT (GPIOB)
#define NOT_PRECHARGE_COMPLETE_PIN  (11)
#define IMD_STATUS_PORT (GPIOA)
#define IMD_STATUS_PIN  (9)


#endif
