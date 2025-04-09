
#include "common/common_defs/common_defs.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"

// decode IMD status output PWM signal that changes both frequency and pwm

void imdDecodeInit(void)
{
    // Configure timer 2
    // Targeting an interrupt every 1 ms
    //if (RCC->CFGR & RCC_CFGR_PPRE1_2) freq *= 2; // RM0394 pg 188 (timer clock doubles if apb1 prescaler != 0)
    #if (defined(STM32F407xx) || defined(STM32F732xx))
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    #else
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    #endif

    TIM2->CR1   &= ~TIM_CR1_CEN;                     // Disable counter during configuration
    TIM2->PSC   =  0;                                // Do not scale the counter clock
    TIM2->CNT   =  0;                                // Reset the counter value
    TIM2->ARR   =  0xFFFFFFFF;                       // Set auto reload to allow for maximum counter value


    // Disable capture from the counter for channels 1 and 2
    TIM2->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);

    /** configure channel 1 for active rising input **/
    // select channel 1 as the active input
    TIM2->CCMR1 &= ~TIM_CCMR1_CC1S_Msk; // clear bits
    TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;    // 01 - maps ch1 to TI1

    // setup input capture filtering
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1F_Msk; // clear bits
    // select the edge of transition
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // 00 - active rising
    // set input Pre-Scaler
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1PSC_Msk; // clear bits



    /** configure channel 2 for active falling input **/
    // select channel 2 as the active input
    TIM2->CCMR1 &= ~TIM_CCMR1_CC2S_Msk; // clear bits
    TIM2->CCMR1 |= TIM_CCMR1_CC2S_1;    // 10 - maps channel 2 to TI1

    // setup input capture filtering
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2F_Msk; // clear bits
    // select the edge of transition
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // clear bits
    TIM2->CCER |= TIM_CCER_CC2P; // 01 - active falling

    // set input Pre-Scaler
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2PSC_Msk; // clear bits

    // select valid trigger input
    TIM2->SMCR &= ~TIM_SMCR_TS_Msk; // clear bits
    TIM2->SMCR |= (TIM_SMCR_TS_0 | TIM_SMCR_TS_2); // 101 - Filtered Timer Input 1 (TI1FP1)

    // configure the slave mode controller in reset mode
    TIM2->SMCR &= ~TIM_SMCR_SMS_Msk; // clear bits
    TIM2->SMCR |= TIM_SMCR_SMS_2; // 100 - rising edge resets the counter


    /** turn stuff on and configure DMA requests **/
    // Enable capture from the counter for channel 1/2
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    // enable interrupt on channel 1/2
    //TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;

    // enable counter
    TIM2->CR1 |= TIM_CR1_CEN;
    #if 0
    // set interrupt priority
    NVIC_SetPriority(TIM2_IRQn, 0); // highest priority
    // enable TIM5 interrupt on NVIC
    NVIC_EnableIRQ(TIM2_IRQn);
    #endif
}

void imdDecodePeriodic(void)
{
    uint16_t period = TIM2->CCR1;
    uint16_t duty = TIM2->CCR2;
    float frequency = (float)APB1ClockRateHz / (period + 1);
    float duty_cycle = ((duty + 1) * 100) / (period);
}

#if 0
void TIM2_IRQHandler(void)
{
  /* Clear TIM2 Capture compare interrupt pending bit */
  if ((TIM2->SR & TIM_DIER_CC1IE) && (TIM2->DIER & TIM_DIER_CC1IE))
  {
    TIM2->SR = (uint16_t)~TIM_DIER_CC1IE;
    uint16_t value = TIM2->CCR1;
  }
}
#endif
