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

volatile extern raw_adc_values_t raw_adc_values;

#endif