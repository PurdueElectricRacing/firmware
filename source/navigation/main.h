/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software for controlling navigation
            sensor acquisition
 * @version 0.1
 * @date 2022-12-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_

// STM32L471RET

// Status Indicators
#define ERR_LED_GPIO_Port (GPIOB)
#define ERR_LED_Pin (5)
#define CONN_LED_GPIO_Port (GPIOB)
#define CONN_LED_Pin (7)
#define CONN_LED_MS_THRESH (500)
#define HEARTBEAT_GPIO_Port (GPIOB)
#define HEARTBEAT_Pin (6)

// SPI IMU
#define SPI_SCLK_GPIO_Port (GPIOA)
#define SPI_SCLK_Pin (5)
#define SPI_MISO_GPIO_Port (GPIOA)
#define SPI_MISO_Pin (6)
#define SPI_MOSI_GPIO_Port (GPIOA)
#define SPI_MOSI_Pin (7)

#define SPI_CS_ACEL_GPIO_Port (GPIOA)
#define SPI_CS_ACEL_Pin (3)
#define SPI_CS_GYRO_GPIO_Port (GPIOA)
#define SPI_CS_GYRO_Pin (2)
#define SPI_CS_MAG_GPIO_Port (GPIOB)
#define SPI_CS_MAG_Pin (0)

// USART GPS
#define GPS_RX_GPIO_Port (GPIOC)
#define GPS_RX_Pin (5)
#define GPS_TX_GPIO_Port (GPIOC)
#define GPS_TX_Pin (4)

// EEPROM
#define NAV_EEPROM_CS_GPIO_PORT (GPIOB)
#define NAV_EEPROM_CS_PIN (12)
#define NAV_WP_GPIO_PORT (GPIOB)
#define NAV_WP_PIN (13)

#endif