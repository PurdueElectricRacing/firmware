#ifndef MAIN_H_
#define MAIN_H_

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_TEST

// Inputs
#define BUTTON_1_Pin (8)
#define BUTTON_1_GPIO_Port (GPIOA)
#define POT_Pin (0)
#define POT_GPIO_Port (GPIOA)
#define POT_ADC_Channel (5)
#define POT2_Pin (1)
#define POT2_GPIO_Port (GPIOA)
#define POT2_ADC_Channel (6)
#define POT3_Pin (3)
#define POT3_GPIO_Port (GPIOA)
#define POT3_ADC_Channel (8)

// Status LEDs
#define LED_GREEN_Pin (3)
#define LED_GREEN_GPIO_Port (GPIOB)
#define LED_RED_Pin (1)
#define LED_RED_GPIO_Port (GPIOB)
#define LED_BLUE_Pin (7)
#define LED_BLUE_GPIO_Port (GPIOB)

#define LED1_Pin (3)
#define LED1_GPIO_Port (GPIOB)

// TIM Pins
#define TIM1_GPIO_Port (GPIOA)
#define TIM1_Pin (8)
#define TIM1_AF (1)

#define TIM2_GPIO_Port (GPIOA)
#define TIM2_Pin (0)
#define TIM2_AF (1)

#define TIM16_GPIO_Port (GPIOA)
#define TIM16_Pin (6)
#define TIM16_AF (14)

// SPI
#define SPI_SCLK_GPIO_Port (GPIOA)
#define SPI_SCLK_Pin (5)
#define SPI_MISO_GPIO_Port (GPIOA)
#define SPI_MISO_Pin (6)
#define SPI_MOSI_GPIO_Port (GPIOA)
#define SPI_MOSI_Pin (7)
#define SPI_CS_EEPROM_GPIO_Port (GPIOA)
#define SPI_CS_EEPROM_Pin (4)

#endif