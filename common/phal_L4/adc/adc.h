/**
 * @file adc.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 */

#ifndef _PHAL_ADC_H
#define _PHAL_ADC_H

#ifdef STM32L496xx
//#include "stm32l496xx.h"
#error "STM32L496xx is currently not supported for adc"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include <stdbool.h>

#define PHAL_ADC_INIT_TIMEOUT 1000000
#define PHAL_ADC_CR_BITS_RS 0b10000000000000000000000000111111

typedef enum {
    ADC_RES_12_BIT = 0b00,
    ADC_RES_10_BIT = 0b01,
    ADC_RES_8_BIT = 0b10,
    ADC_RES_6_BIT = 0b11
} ADCResolution_t;

typedef enum {
    ADC_CLK_PRESC_0 = 0b0000,
    ADC_CLK_PRESC_2 = 0b0001,
    ADC_CLK_PRESC_4 = 0b0010,
    ADC_CLK_PRESC_6 = 0b0011,
    ADC_CLK_PRESC_8 = 0b0100,
    ADC_CLK_PRESC_10 = 0b0101,
    ADC_CLK_PRESC_12 = 0b0110,
    ADC_CLK_PRESC_16 = 0b0111,
    ADC_CLK_PRESC_32 = 0b1000,
    ADC_CLK_PRESC_64 = 0b1001,
    ADC_CLK_PRESC_128 = 0b1010,
    ADC_CLK_PRESC_256 = 0b1011
} ADCClkPrescaler_t;

typedef enum {
    ADC_DATA_ALIGN_RIGHT = 0b0,
    ADC_DATA_ALIGN_LEFT = 0b1
} ADCDataAlign_t;

typedef struct {
    ADCClkPrescaler_t clock_prescaler; // required to have high enough prescaler to operate within ADC maximum freq
    ADCResolution_t resolution; // bit resolution of readings
    ADCDataAlign_t data_align;
    //uint32_t ext_trig_conv;
    //uint32_t ext_trig_conv_edge;
    bool cont_conv_mode;
    //bool discont_conv_mode;
    bool overrun; // set true if data register can be overwritten before being read
    //uint32_t nbr_of_disc_conv;
} ADCInitConfig_t;

typedef enum {
    ADC_CHN_SMP_CYCLES_2_5   = 0b000,
    ADC_CHN_SMP_CYCLES_6_5   = 0b001,
    ADC_CHN_SMP_CYCLES_12_5  = 0b010,
    ADC_CHN_SMP_CYCLES_24_5  = 0b011,
    ADC_CHN_SMP_CYCLES_47_5  = 0b100,
    ADC_CHN_SMP_CYCLES_92_5  = 0b101,
    ADC_CHN_SMP_CYCLES_247_5 = 0b110,
    ADC_CHN_SMP_CYCLES_640_5 = 0b111,
} ADCChannelSampleCycles_t;

typedef struct {
    uint32_t channel; // not the GPIO channel, use the ADC channel (ie. PA0 = channel 5)
    uint32_t rank; // order at which the channels will be polled, starting at 1
    ADCChannelSampleCycles_t sampling_time; // 2_5 works, set higher for large imedances
    //uint32_t single_diff;
    //uint32_t offset_num;
    //uint32_t offset;
} ADCChannelConfig_t;

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