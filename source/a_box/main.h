/**
 * @file main.h
 * @brief "Abox" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
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

#define SPI1_CS_PORT   (GPIOB)
#define SPI1_CS_PIN    (0)

#define SPI2_CS_PORT   (GPIOB)
#define SPI2_CS_PIN    (1)

#endif
