/**
 * @file adc.h
 * @author Chris McGalliard - port of L4 HAL by Luke Oxley (lcoxley@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2023-09-17
 */

#ifndef __PHAL_G4_ADC_H__
#define __PHAL_G4_ADC_H__

#include "common/phal_G4/phal_G4.h"

typedef enum {
    ADC_RES_12_BIT = 0b00,
    ADC_RES_10_BIT = 0b01,
    ADC_RES_8_BIT  = 0b10,
    ADC_RES_6_BIT  = 0b11
} ADCResolution_t;

typedef enum {
    ADC_CLK_PRESC_0 = 0b0000,
    ADC_CLK_PRESC_2 = 0b0001,
    ADC_CLK_PRESC_4 = 0b0010,
    ADC_CLK_PRESC_6 = 0b0011,
    ADC_CLK_PRESC_8 = 0b0100,
} ADCClkPrescaler_t;

typedef enum {
    ADC_DMA_OFF      = 0b00,
    ADC_DMA_ONESHOT  = 0b01,
    ADC_DMA_CIRCULAR = 0b11
} ADCDMAMode_t;

typedef enum {
    ADC_DATA_ALIGN_RIGHT = 0b0,
    ADC_DATA_ALIGN_LEFT  = 0b1
} ADCDataAlign_t;

typedef enum {
    ADC_OVERSAMPLE_NONE = 0,
    ADC_OVERSAMPLE_2    = 2,
    ADC_OVERSAMPLE_4    = 4,
    ADC_OVERSAMPLE_8    = 8,
    ADC_OVERSAMPLE_16   = 16,
    ADC_OVERSAMPLE_32   = 32,
    ADC_OVERSAMPLE_64   = 64,
    ADC_OVERSAMPLE_128  = 128,
    ADC_OVERSAMPLE_256  = 256,
} ADCOversampleCount_t;

typedef struct {
    ADCClkPrescaler_t prescaler;
    ADCResolution_t resolution;
    ADCDataAlign_t data_align;
    bool cont_conv_mode;
    ADCOversampleCount_t oversample;
    ADCDMAMode_t dma_mode;
    ADC_TypeDef* periph;
} ADCInitConfig_t;

typedef enum {
    ADC_CHN_SMP_CYCLES_3   = 0b000,
    ADC_CHN_SMP_CYCLES_15  = 0b001,
    ADC_CHN_SMP_CYCLES_28  = 0b010,
    ADC_CHN_SMP_CYCLES_56  = 0b011,
    ADC_CHN_SMP_CYCLES_84  = 0b100,
    ADC_CHN_SMP_CYCLES_112 = 0b101,
    ADC_CHN_SMP_CYCLES_144 = 0b110,
    ADC_CHN_SMP_CYCLES_480 = 0b111,
} ADCChannelSampleCycles_t;

typedef enum {
    ADC_CHANNEL_1 = 1,
    ADC_CHANNEL_2 = 2,
    ADC_CHANNEL_3 = 3,
    ADC_CHANNEL_4 = 4,
} ADCChannel_t;

typedef struct {
    ADCChannel_t channel; // not the GPIO channel, use the ADC channel
    uint32_t rank; // order at which the channels will be polled, starting at 0
    ADCChannelSampleCycles_t sampling_time;
} ADCChannelConfig_t;

#define ADC1_CH1_GPIO_Port (GPIOA)
#define ADC1_CH1_Pin       (0)
#define ADC1_CH2_GPIO_Port (GPIOA)
#define ADC1_CH2_Pin       (1)
#define ADC1_CH3_GPIO_Port (GPIOA)
#define ADC1_CH3_Pin       (2)
#define ADC1_CH4_GPIO_Port (GPIOA)
#define ADC1_CH4_Pin       (3)

/**
 * @brief Initializes the ADC, requires GPIO config prior
 *
 * @param adc ADC handle
 * @param config ADC initial config settings
 * @param channels List of channel configurations
 * @param num_channels Number of channels in the channel configuration list
**/
bool PHAL_initADC(ADCInitConfig_t* config, ADCChannelConfig_t channels[], uint8_t num_channels);

/**
 * @brief Starts the ADC conversions, requires PHAL_initADC to be called prior
 *
 * @param adc ADC handle
**/
bool PHAL_startADC(ADCInitConfig_t* config);

/**
 * @brief Stops the ADC conversions, requires PHAL_initADC to be called prior
 *
 * @param adc ADC handle
**/
bool PHAL_stopADC(ADCInitConfig_t* config);

/**
 * @brief Reads the ADC data register
 *
 * @param adc ADC handle
 * @return contents of the data register
**/
uint16_t PHAL_readADC(ADCInitConfig_t* config);

// TODO ADC3 config (ADC2 doesn't support DMA)
#define ADC1_DMA_CONT_CONFIG(mem_addr_, tx_size_, priority_) \
    {.periph_addr      = (uint32_t)&(ADC1->DR), \
     .mem_addr         = mem_addr_, \
     .tx_size          = tx_size_, \
     .increment        = true, \
     .circular         = true, \
     .dir              = 0b0, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = priority_, \
     .mem_size         = DMA_SIZE_16BIT, \
     .periph_size      = DMA_SIZE_16BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0b0000, \
     .channel_idx      = 1, \
     .mux_request      = DMA_REQUEST_ADC1, \
     .periph           = DMA1, \
     .channel          = DMA1_Channel1}

#endif // __PHAL_G4_ADC_H__
