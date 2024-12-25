#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdint.h>
#include "common/faults/fault_nodes.h"
#include "common/phal_F4_F7/can/can.h"

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
    uint16_t shock_left;
    uint16_t shock_right;
    uint16_t lv_5v_sense;
    uint16_t lv_3v3_sense;
    uint16_t lv_12v_sense;
    uint16_t lv_24_v_sense;
    uint16_t load_l;
    uint16_t load_r;
    uint16_t pot_val;
} raw_adc_values_t;

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

#define BRK_STAT_TAP_GPIO_Port      (GPIOB)
#define BRK_STAT_TAP_Pin            (9)
#define BRK_FAIL_TAP_GPIO_Port      (GPIOA)
#define BRK_FAIL_TAP_Pin            (6)

volatile extern raw_adc_values_t raw_adc_values;

#endif