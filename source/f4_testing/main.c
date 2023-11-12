#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/psched/psched.h"

#include "main.h"

volatile raw_adc_values_t raw_adc_values;

/* ADC Configuration */
ADCInitConfig_t adc_config = {
   .clock_prescaler = ADC_CLK_PRESC_6,
   .resolution      = ADC_RES_12_BIT,
   .data_align      = ADC_DATA_ALIGN_RIGHT,
   .cont_conv_mode  = true,
   .adc_number      = 1,
//    .overrun         = true,
   .dma_mode        = ADC_DMA_CIRCULAR
};
// TODO: check prescaler for udpate rate
ADCChannelConfig_t adc_channel_config[] = {
   {.channel=0, .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_480},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.testval), 0b01);

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_ANALOG(GPIOA, 0),
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED),
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

void ledblink();

int main()
{
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);
        /* Task Creation */
    schedInit(APB1ClockRateHz);
        taskCreate(ledblink, 15);
        /* Schedule Periodic tasks here */
    schedStart();
    return 0;
}

void ledblink()
{
    uint16_t val = raw_adc_values.testval;
    float voltage = (val / 4095.0) * 3.3f;
    if (voltage < 0.825)
    {
        PHAL_writeGPIO(GPIOD, GREEN, 1);
        PHAL_writeGPIO(GPIOD, ORANGE, 0);
        PHAL_writeGPIO(GPIOD, RED, 0);
        PHAL_writeGPIO(GPIOD, BLUE, 0);
    }
    else if (voltage < 1.65)
    {
        PHAL_writeGPIO(GPIOD, GREEN, 0);
        PHAL_writeGPIO(GPIOD, ORANGE, 0);
        PHAL_writeGPIO(GPIOD, RED, 0);
        PHAL_writeGPIO(GPIOD, BLUE, 1);
    }
    else if (voltage < 2.475)
    {
        PHAL_writeGPIO(GPIOD, GREEN, 0);
        PHAL_writeGPIO(GPIOD, ORANGE, 1);
        PHAL_writeGPIO(GPIOD, RED, 0);
        PHAL_writeGPIO(GPIOD, BLUE, 0);
    }
    else
    {
        PHAL_writeGPIO(GPIOD, GREEN, 0);
        PHAL_writeGPIO(GPIOD, ORANGE, 0);
        PHAL_writeGPIO(GPIOD, RED, 1);
        PHAL_writeGPIO(GPIOD, BLUE, 0);
    }
    // PHAL_toggleGPIO(GPIOD, 13);
}

void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}