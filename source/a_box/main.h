/**
 * @file main.h
 * @brief "Abox" node source code
 * 
 * @author Sebastian Arthur (arthur31@purdue.edu), Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

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

#define SPI2_CS_PORT (GPIOB)
#define SPI2_CS_PIN  (1)

//  Status Inputs + Diagnostic Values
#define CHARGER_CONNECT_PORT (GPIOB)
#define CHARGER_CONNECT_PIN (2)

#define IMD_STATUS_PORT (GPIOA)
#define IMD_STATUS_PIN (9)

#define NOT_PRECHARGE_COMPLETE_PORT (GPIOB)
#define NOT_PRECHARGE_COMPLETE_PIN (11)

#define ISENSE_MCU_PORT (GPIOA)
#define ISENSE_MCU_PIN (0)

#define VBATT_MCU_PORT (GPIOA)
#define VBATT_MCU_PIN (1)

// OUTPUT PINS

#define BMS_STATUS_PORT (GPIOA)
#define BMS_STATUS_PIN (10)

#define FAN1_PWM_PORT (GPIOB)
#define FAN1_PWM_PIN (10)

#endif
