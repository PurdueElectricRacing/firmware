/**
 * @file adc.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 */

#include "common/phal_L4/adc/adc.h"

bool PHAL_initADC(ADC_TypeDef* adc, ADCInitConfig_t* config, ADCChannelConfig_t channels[], uint8_t num_channels)
{
#ifdef STM32L432xx
    // Use system clock as source
    RCC->CCIPR   |= RCC_CCIPR_ADCSEL;
    // Enable clock
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

    // get out of deep power down
    adc->CR &= ~(ADC_CR_DEEPPWD | PHAL_ADC_CR_BITS_RS);

    // enable voltage regulator
    adc->CR &= ~(PHAL_ADC_CR_BITS_RS);
    adc->CR |= ADC_CR_ADVREGEN;

    uint16_t wait_time = 400;
    while (wait_time != 0)
    {
        wait_time--;
    }

    // Verify regulator on
    if (!(adc->CR & ADC_CR_ADVREGEN))
    {
        // regulator failed to turn on, possible clock problem
        return false;
    }

    // Set clock mode
    ADC1_COMMON->CCR &= ~(ADC_CCR_CKMODE);

    // Set clock prescaler
    ADC1_COMMON->CCR &= ~(ADC_CCR_PRESC);
    ADC1_COMMON->CCR |= (config->clock_prescaler << ADC_CCR_PRESC_Pos) & ADC_CCR_PRESC_Msk;

    // Set cont/discont conv modes
    adc->CFGR &= ~(ADC_CFGR_CONT);
    adc->CFGR |= (config->cont_conv_mode << ADC_CFGR_CONT_Pos) & ADC_CFGR_CONT_Msk;

    // Overrun    
    adc->CFGR &= ~(ADC_CFGR_OVRMOD);
    adc->CFGR |= (config->overrun << ADC_CFGR_OVRMOD_Pos) & ADC_CFGR_OVRMOD_Msk;

    // Set resolution
    adc->CFGR &= ~(ADC_CFGR_RES);
    adc->CFGR |= (config->resolution << ADC_CFGR_RES_Pos) & ADC_CFGR_RES_Msk;

    // Data alignment
    adc->CFGR &= ~(ADC_CFGR_ALIGN);
    adc->CFGR |= (config->data_align << ADC_CFGR_ALIGN_Pos) & ADC_CFGR_ALIGN_Msk;

    // Regular channel sequence length
    adc->SQR1 &= ~(ADC_SQR1_L);
    adc->SQR1 |= ((num_channels - 1) << ADC_SQR1_L_Pos) & ADC_SQR1_L_Msk;

    // Channel configuration
    for (int i = 0; i < num_channels; i++)
    {
        // Configure sample time: https://controllerstech.com/adc-conversion-time-frequency-calculation-in-stm32/
        if (channels[i].channel < 10)
        {
            adc->SMPR1 &= ~(ADC_SMPR1_SMP0_Msk << (ADC_SMPR1_SMP1_Pos * channels[i].channel));
            adc->SMPR1 |= (channels[i].sampling_time & ADC_SMPR1_SMP0_Msk) << (ADC_SMPR1_SMP1_Pos * channels[i].channel);
        }
        else if (channels[i].channel > 9 && channels[i].channel < 19)
        {
            adc->SMPR2 &= ~(ADC_SMPR2_SMP10_Msk << (ADC_SMPR2_SMP11_Pos * (channels[i].channel - 10)));
            adc->SMPR2 |= (channels[i].sampling_time & ADC_SMPR2_SMP10_Msk) << (ADC_SMPR2_SMP11_Pos * (channels[i].channel - 10));
        }

        // Sequence rank
        if (channels[i].rank < 5)
        {
            adc->SQR1 &= ~(ADC_SQR1_SQ1_Msk << ((channels[i].rank - 1) * 6));
            adc->SQR1 |= ((channels[i].channel << 6) & ADC_SQR1_SQ1_Msk) << ((channels[i].rank - 1) * 6);
        }
        // TODO: more than 4 channels
    }

    // Enable
    /*
    * clear ADRDY in ADC_ISR by writing 1
    * set ADEN = 1
    * wait until ADRDY=1 using ADRDYIE=1 interrupt
    * clear ADRDY bit in ADC_ISR by writing 1 (optional)
    */
    //for(int j = 0; j<10000;j++);
    // clear ADRDY bit
    adc->ISR |= ADC_ISR_ADRDY;
    adc->CR &= ~(PHAL_ADC_CR_BITS_RS);
    adc->CR |= ADC_CR_ADEN;
    uint32_t timeout = 0;
    while(!(adc->ISR & ADC_ISR_ADRDY) && ++timeout < PHAL_ADC_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_ADC_INIT_TIMEOUT)
        return false;
#endif
    return true;
}

bool PHAL_startADC(ADC_TypeDef* adc)
{
    adc->CR |= ADC_CR_ADSTART;
    return true;
}

bool PHAL_stopADC(ADC_TypeDef* adc)
{
    adc->CR |= ADC_CR_ADSTP;
    return true;
}

uint16_t PHAL_readADC(ADC_TypeDef* adc)
{
    return (uint16_t) (adc->DR & ADC_DR_RDATA_Msk);
}

