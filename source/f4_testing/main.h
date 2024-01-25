#ifndef _MAIN_H_
#define _MAIN_H_

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t testval;
} raw_adc_values_t;

#define RED 14
#define BLUE 15
#define GREEN 12
#define ORANGE 13

#define SPI_CS_PIN 3
#define SPI_CS_PORT GPIOE

#define SPI_SCK_PIN 5
#define SPI_SCK_PORT GPIOA

#define SPI_MISO_PIN 6
#define SPI_MISO_PORT GPIOA

#define SPI_MOSI_PIN 7
#define SPI_MOSI_PORT GPIOA


#define USART2_TX_PIN 2
#define USART2_TX_GPIO_PORT GPIOA

#define USART2_RX_PIN 3
#define USART2_RX_GPIO_PORT GPIOA
volatile extern raw_adc_values_t raw_adc_values;

#endif