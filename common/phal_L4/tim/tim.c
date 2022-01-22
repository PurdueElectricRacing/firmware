/**
 * @file tim.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief Timer library meant for PWM input / output
 * @version 0.1
 * @date 2021-01-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "common/phal_L4/tim/tim.h"

// prototypes
bool enableTIMClk(TIM_TypeDef* timer);

bool enableTIMClk(TIM_TypeDef* timer)
{
    // Enable Timer Clock 
    if (timer == TIM1)
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    }
    else if (timer == TIM2)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    }
    else if (timer == TIM6)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    }
    else if (timer == TIM7)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;
    }
    else if (timer == TIM15)
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    }
    else if (timer == TIM16)
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
    }
    else
    {
        return false;
    }
    return true;
}

// available on:
// TIM1, TIM2: channel (1,2), (3,4)
// TIM15 (1,2)
// TIM2 is the only 32 bit resolution, currently 32 is not supported
bool PHAL_initPWMIn(TIM_TypeDef* timer, uint32_t prescaler, TimerTriggerSelection_t trigger_select)
{
    if (!enableTIMClk(timer)) return false;
    // can set the input prescaler (capture / events ICPSC)
    // this is done if using interrupts and you want less of them

    // configure prescaler
    timer->PSC = prescaler - 1;
    
    // Update interrupt only on overflow
    timer->CR1 |= TIM_CR1_URS;

    // Select the trigger input TI1FP1
    timer->SMCR |= (trigger_select << TIM_SMCR_TS_Pos) & TIM_SMCR_TS;
    //TIM1->CR2 |= TIM_CR2_MMS_1;
    // Set slave controller to reset mode (used for PWM input)
    timer->SMCR |= TIM_SMCR_SMS_2;

    // enable counter
    timer->CR1 |= TIM_CR1_CEN;

    return true;
    // interrupt request through CC1IE in TIM_DIER
}

void PHAL_startTIM(TIM_TypeDef* timer)
{
    timer->CR1 |= TIM_CR1_CEN;
}

// initialize a capture compare channel
bool PHAL_initPWMChannel(TIM_TypeDef* timer, TimerCCRegister_t chnl, TimerInputMode_t input_source, bool is_falling)
{
    // Input selection
    if (chnl < 3)
    {
        timer->CCMR1 &= ~((input_source & TIM_CCMR1_CC1S) << (chnl - 1)*TIM_CCMR1_CC2S_Pos);
        timer->CCMR1 |= (input_source & TIM_CCMR1_CC1S) << (chnl - 1)*TIM_CCMR1_CC2S_Pos;
    }
    else if (chnl < 5)
    {
        timer->CCMR2 &= ~((input_source & TIM_CCMR2_CC3S) << (chnl - 3)*TIM_CCMR2_CC4S_Pos);
        timer->CCMR2 |= (input_source & TIM_CCMR2_CC3S) << (chnl - 3)*TIM_CCMR2_CC4S_Pos;
    }
    else 
    {
        return false;
    }

    // polarity
    timer->CCER &= ~((TIM_CCER_CC1P | TIM_CCER_CC1NP) << (chnl - 1) * 4);
    if (is_falling)
    {
        timer->CCER |= (TIM_CCER_CC2P << (chnl - 1) * 4);
    }

    // enable channel
    timer->CCER |= TIM_CCER_CC1E << (chnl - 1) * 4;

    return true;
}

bool PHAL_initPWMOut(TIM_TypeDef* timer)
{
    if (!enableTIMClk(timer)) return false;

    // uint16_t frequency = 100; // Hz
    // uint16_t counter_period = 16;//(16000 / 1) - 1;//666 - 1;
    // uint8_t duty = 50;
    // uint16_t counter_freq = counter_period * frequency;
    // uint16_t prescaler = 3952000 / counter_freq; //(SystemCoreClock / 20 / counter_freq) - 1;

    // counter mode: CR1 DIR and CR1 CMS (not available for 16)
    // clock division: CR1 CKD (leave at 0)
    // auto reload preload CR1 ARPE 
    TIM16->CR1 |= TIM_CR1_ARPE; // buffered
    // auto reload value ARR (period)
    //TIM16->ARR = counter_period - 1;
    // prescaler PSC (prescaler)
    //TIM16->PSC = prescaler - 1;//(uint16_t) (SystemCoreClock/16000 - 1); // 16 kHz counter clock
    // repetition counter RCR (repetition counter) (leave to 0)
    // reload for prescaler and rep counter
    TIM16->EGR = TIM_EGR_UG;

    // -- channel config --
    // disable chnl 1 bit clear CCER CC1E
    TIM16->CCER &= ~(TIM_CCER_CC1E);
    // set to output  CC1s in CCMR1
    TIM16->CCMR1 &= ~(TIM_CCMR1_CC1S);
    // select OCMode OC1M in CCMR1 (pwm mode)
    TIM16->CCMR1 &= ~(TIM_CCMR1_OC1M);
    TIM16->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    // select output polarity CCER CC1P and CCER CC1N or something
    //TIM16->CCER &= ~(TIM_CCER_CC1P)
    // set CCR1 to Pulse
    //TIM16->CCR1 = DUTY_TO_PULSE(duty, counter_period);
    // Preload enable bit CCMR1 TIM_CCMR1_OC1PE
    TIM16->CCMR1 |= TIM_CCMR1_OC1PE;
    // CCMR1 (OCFastMode) (leave disabled OC1FE)

    // -- start --
    // enable cap compare chnl CCx_ENABLE TIM_CCxChannelCmd
    //TIM16->CCER |= TIM_CCER_CC1E;
    // main output _HAL_TIM_MOE_ENABLE
    //BDTR set MOE
    TIM16->BDTR |= TIM_BDTR_MOE;
    // _HAL_TIM_ENABLE
    //TIM16->CR1 |= TIM_CR1_CEN;

    return true;
}


void  __attribute__((weak)) TIM1_UP_TIM16_IRQHandler()
{
    // implement
}

void  __attribute__((weak)) TIM1_CC_IRQHandler()
{
    // implement
}

void  __attribute__((weak)) TIM2_IRQHandler()
{
    // implement
}
