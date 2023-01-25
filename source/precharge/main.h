#ifndef MAIN_H_
#define MAIN_H_

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_PRECHARGE

// SPI Accel
#define SPI_SCLK_GPIO_Port (GPIOA)
#define SPI_SCLK_Pin (5)
#define SPI_MISO_GPIO_Port (GPIOA)
#define SPI_MISO_Pin (6)
#define SPI_MOSI_GPIO_Port (GPIOA)
#define SPI_MOSI_Pin (7)
#define SPI_CS_ACEL_GPIO_Port (GPIOA)
#define SPI_CS_ACEL_Pin (8)
#define SPI_CS_GYRO_GPIO_Port (GPIOA)
#define SPI_CS_GYRO_Pin (9)
#define SPI_CS_TMU_GPIO_Port (GPIOD)
#define SPI_CS_TMU_GPIO_Pin (15)

// Current Sense
#define I_SENSE_CH1_GPIO_Port (GPIOA) // ADC12_IN5
#define I_SENSE_CH1_Pin (0)
#define I_SENSE_CH2_GPIO_Port (GPIOA) // ADC12_IN6
#define I_SENSE_CH2_Pin (1)

// BMS Status
#define BMS_STATUS_GPIO_Port (GPIOA)
#define BMS_STATUS_Pin (3)

// IMD Data
#define IMD_HS_PWM_GPIO_Port (GPIOB)
#define IMD_HS_PWM_Pin (6)
#define IMD_LS_PWM_GPIO_Port (GPIOB)
#define IMD_LS_PWM_Pin (7)
#define IMD_STATUS_GPIO_Port (GPIOB)
#define IMD_STATUS_Pin (8)

// Status LEDs
#define HEARTBEAT_LED_GPIO_Port (GPIOE)
#define HEARTBEAT_LED_Pin       (13)
#define CONN_LED_GPIO_Port      (GPIOE)
#define CONN_LED_Pin            (14)
#define ERROR_LED_GPIO_Port     (GPIOE)
#define ERROR_LED_Pin           (15)


#endif