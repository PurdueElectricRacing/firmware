/**
 * @file adc.h
 * @author Chris McGalliard - port of L4 HAL by Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2023-09-17
 */

#ifndef _PHAL_ADC_H
#define _PHAL_ADC_H

#include <stdbool.h>

#if defined(STM32F407xx)
#include "stm32f4xx.h"
#include "stm32f407xx.h"
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#include "stm32f732xx.h"
#else
#error "Please define a MCU arch"
#endif

typedef enum {
    ADC_RES_12_BIT = 0b00,
    ADC_RES_10_BIT = 0b01,
    ADC_RES_8_BIT = 0b10,
    ADC_RES_6_BIT = 0b11
} ADCResolution_t;

typedef enum {
    ADC_CLK_PRESC_2 = 0b00,
    ADC_CLK_PRESC_4 = 0b01,
    ADC_CLK_PRESC_6 = 0b10,
    ADC_CLK_PRESC_8 = 0b11,
} ADCClkPrescaler_t;

typedef enum {
    ADC_DMA_OFF      = 0b00,
    ADC_DMA_ONE_SHOT = 0b01,
    ADC_DMA_CIRCULAR = 0b11
} ADCDMAMode_t;

typedef enum {
    ADC_DATA_ALIGN_RIGHT = 0b0,
    ADC_DATA_ALIGN_LEFT = 0b1
} ADCDataAlign_t;

typedef struct {
    ADCClkPrescaler_t clock_prescaler;
    ADCResolution_t resolution;
    ADCDataAlign_t data_align;
    bool cont_conv_mode;
    ADCDMAMode_t dma_mode;
    uint8_t adc_number;
} ADCInitConfig_t;

typedef enum {
    ADC_CHN_SMP_CYCLES_3    = 0b000,
    ADC_CHN_SMP_CYCLES_15   = 0b001,
    ADC_CHN_SMP_CYCLES_28   = 0b010,
    ADC_CHN_SMP_CYCLES_56   = 0b011,
    ADC_CHN_SMP_CYCLES_84   = 0b100,
    ADC_CHN_SMP_CYCLES_112  = 0b101,
    ADC_CHN_SMP_CYCLES_144  = 0b110,
    ADC_CHN_SMP_CYCLES_480  = 0b111,
} ADCChannelSampleCycles_t;

typedef struct {
    uint32_t channel;                           // not the GPIO channel, use the ADC channel
    uint32_t rank;                              // order at which the channels will be polled, starting at 1
    ADCChannelSampleCycles_t sampling_time;
} ADCChannelConfig_t;

// TODO DMA CONFIGS FOR ADC


/**
 * @brief Initializes the ADC, requires GPIO config prior
 * 
 * @param adc ADC handle
 * @param config ADC initial config settings
 * @param channels List of channel configurations
 * @param num_channels Number of channels in the channel configuration list
**/
bool PHAL_initADC(ADC_TypeDef* adc, ADCInitConfig_t* config, ADCChannelConfig_t channels[], uint8_t num_channels);

/**
 * @brief Starts the ADC conversions, requires PHAL_initADC to be called prior
 * 
 * @param adc ADC handle
**/
bool PHAL_startADC(ADC_TypeDef* adc);
/**
 * @brief Stops the ADC conversions, requires PHAL_initADC to be called prior
 * 
 * @param adc ADC handle
**/
bool PHAL_stopADC(ADC_TypeDef* adc);

/**
 * @brief Reads the ADC data register
 * 
 * @param adc ADC handle
 * @return contents of the data register
**/
uint16_t PHAL_readADC(ADC_TypeDef* adc);


#endif
