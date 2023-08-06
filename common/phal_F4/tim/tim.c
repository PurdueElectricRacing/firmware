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

/**
 * @brief Enables the clock for a timer
 * 
 * @param timer Timer to clock
 * @return true  - success
 * @return false - fail
 */
bool PHAL_enableTIMClk(TIM_TypeDef* timer)
{
    // Enable Timer Clock 
    if (timer == TIM1)
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (timer == TIM2)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    #ifdef TIM5
    else if (timer == TIM5)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
    #endif
    else if (timer == TIM6)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    else if (timer == TIM7)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;
    else if (timer == TIM15)
        RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    else if (timer == TIM16)
        RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
    else
        return false;
    return true;
}

// available on:
// TIM1, TIM2: channel (1,2), (3,4)
// TIM15 (1,2)
// TIM2 is the only 32 bit resolution, currently 32 is not supported
bool PHAL_initPWMIn(TIM_TypeDef* timer, uint32_t prescaler, TimerTriggerSelection_t trigger_select)
{
    if (!PHAL_enableTIMClk(timer)) return false;
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
    if (chnl <= CC2)
    {
        timer->CCMR1 &= ~(TIM_CCMR1_CC1S << (chnl - 1)*TIM_CCMR1_CC2S_Pos);
        timer->CCMR1 |= (input_source & TIM_CCMR1_CC1S) << (chnl - 1)*TIM_CCMR1_CC2S_Pos;
    }
    else if (chnl <= CC4)
    {
        timer->CCMR2 &= ~(TIM_CCMR2_CC3S << (chnl - 3)*TIM_CCMR2_CC4S_Pos);
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

bool PHAL_initPWMOut(TIM_TypeDef* timer, uint16_t counter_period, uint16_t ccmr1, uint16_t prescaler)
{
    if (!PHAL_enableTIMClk(timer)) return false;

    // auto reload preload CR1 ARPE 
    timer->CR1 |= TIM_CR1_ARPE; // buffered
    // auto reload value ARR (period)
    timer->ARR = counter_period - 1;
    // prescaler PSC (prescaler)
    timer->PSC = prescaler - 1;
    // reload for prescaler and rep counter
    timer->EGR = TIM_EGR_UG;

    // -- channel config --
    // disable chnl 1 bit clear CCER CC1E
    timer->CCER &= ~(TIM_CCER_CC1E);
    // set to output  CC1s in CCMR1
    timer->CCMR1 &= ~(TIM_CCMR1_CC1S);
    // select OCMode OC1M in CCMR1 (pwm mode)
    timer->CCMR1 &= ~(TIM_CCMR1_OC1M);
    timer->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    // select output polarity CCER CC1P and CCER CC1N or something
    timer->CCER &= ~(TIM_CCER_CC1P);
    // set CCR1 to Pulse
    timer->CCR1 = ccmr1;
    // Preload enable bit CCMR1 TIM_CCMR1_OC1PE
    timer->CCMR1 |= TIM_CCMR1_OC1PE;
    // CCMR1 (OCFastMode) (leave disabled OC1FE)

    // enable cap compare chnl CCx_ENABLE TIM_CCxChannelCmd
    timer->CCER |= TIM_CCER_CC1E;
    //BDTR set MOE
    timer->BDTR |= TIM_BDTR_MOE;

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
