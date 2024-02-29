#include "flow_rate.h"

bool fanControlInit();
void setFan1Speed(uint8_t fan_speed);

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;

bool flowRateInit()
{
    /* NOTE: This is just here for testing reading in a PWM signal */
    fanControlInit();
    setFan1Speed(50);

    /* FLOW_RATE_1_TIM */
    // CH1
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    FLOW_RATE_1_TIM->CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)

    FLOW_RATE_1_TIM->PSC = 1 - 1;
    FLOW_RATE_1_TIM->ARR = 0xFFFF - 1;

    /* Set input capture mode */
    FLOW_RATE_1_TIM->CCER &= ~TIM_CCER_CC1E; // Turn off the channel (necessary to write CC1S bits)
    FLOW_RATE_1_TIM->CCER &= ~TIM_CCER_CC2E; // Turn off the channel (necessary to write CC2S bits)

    /* Setup capture compare 1 (period) */
    FLOW_RATE_1_TIM->CCMR1 &= ~TIM_CCMR1_CC1S;
    FLOW_RATE_1_TIM->CCMR1 |= TIM_CCMR1_CC1S_0; // Map IC1 to TI1

    /* Setup capture compare 2 (duty cycle) */
    FLOW_RATE_1_TIM->CCMR1 &= ~TIM_CCMR1_CC2S;
    FLOW_RATE_1_TIM->CCMR1 |= TIM_CCMR1_CC2S_1; // Map IC2 to TI1

    /* CCR1 (period) needs rising edge */
    FLOW_RATE_1_TIM->CCER &= ~TIM_CCER_CC1P;
    FLOW_RATE_1_TIM->CCER &= ~TIM_CCER_CC1NP;

    /* CCR2 (duty cycle) needs falling edge */
    FLOW_RATE_1_TIM->CCER |= TIM_CCER_CC2P;
    FLOW_RATE_1_TIM->CCER &= ~TIM_CCER_CC2NP;

    /* Select trigger */
    FLOW_RATE_1_TIM->SMCR &= ~TIM_SMCR_TS;
    FLOW_RATE_1_TIM->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;

    /* Select trigger */
    FLOW_RATE_1_TIM->SMCR &= ~TIM_SMCR_SMS;
    FLOW_RATE_1_TIM->SMCR |= TIM_SMCR_SMS_2;

    /* Enable channels */
    FLOW_RATE_1_TIM->CCER |= TIM_CCER_CC1E; // Enable CCR1
    FLOW_RATE_1_TIM->CCER |= TIM_CCER_CC2E; // Enable CCR2

    FLOW_RATE_1_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

    /* FLOW_RATE_2_TIM */
    // CH2
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

    FLOW_RATE_2_TIM->CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)

    FLOW_RATE_2_TIM->PSC = 1 - 1;
    FLOW_RATE_2_TIM->ARR = 0xFFFF - 1;

    /* Set input capture mode */
    FLOW_RATE_2_TIM->CCER &= ~TIM_CCER_CC1E; // Turn off the channel (necessary to write CC1S bits)
    FLOW_RATE_2_TIM->CCER &= ~TIM_CCER_CC2E; // Turn off the channel (necessary to write CC2S bits)

    /* Setup capture compare 1 (period) */
    FLOW_RATE_2_TIM->CCMR1 &= ~TIM_CCMR1_CC1S;
    FLOW_RATE_2_TIM->CCMR1 |= TIM_CCMR1_CC1S_1; // Map IC1 to TI2

    /* Setup capture compare 2 (duty cycle) */
    FLOW_RATE_2_TIM->CCMR1 &= ~TIM_CCMR1_CC2S;
    FLOW_RATE_2_TIM->CCMR1 |= TIM_CCMR1_CC2S_0; // Map IC2 to TI2

    /* CCR1 (period) needs rising edge */
    FLOW_RATE_2_TIM->CCER &= ~TIM_CCER_CC1P;
    FLOW_RATE_2_TIM->CCER &= ~TIM_CCER_CC1NP;

    /* CCR2 (duty cycle) needs falling edge */
    FLOW_RATE_2_TIM->CCER |= TIM_CCER_CC2P;
    FLOW_RATE_2_TIM->CCER &= ~TIM_CCER_CC2NP;

    /* Select trigger */
    FLOW_RATE_2_TIM->SMCR &= ~TIM_SMCR_TS;
    FLOW_RATE_2_TIM->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_1;

    /* Select trigger */
    FLOW_RATE_2_TIM->SMCR &= ~TIM_SMCR_SMS;
    FLOW_RATE_2_TIM->SMCR |= TIM_SMCR_SMS_2;

    /* Enable channels */
    FLOW_RATE_2_TIM->CCER |= TIM_CCER_CC1E; // Enable CCR1
    FLOW_RATE_2_TIM->CCER |= TIM_CCER_CC2E; // Enable CCR2

    FLOW_RATE_2_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

    return true;
}

uint32_t getFlowRate1()
{
    uint32_t freq = (1.0 / ((FLOW_RATE_1_TIM->CCR1) * ((FLOW_RATE_1_TIM->PSC + 1.0) / APB1ClockRateHz)));
    // uint32_t duty_cycle = ((float)(FLOW_RATE_1_TIM->CCR1) / FLOW_RATE_1_TIM->CCR2) * 100;
    uint32_t flow_rate = freq / 7.5;
    return flow_rate;
}

uint32_t getFlowRate2()
{
    uint32_t freq = (1.0 / ((FLOW_RATE_2_TIM->CCR2) * ((FLOW_RATE_2_TIM->PSC + 1.0) / APB2ClockRateHz)));
    // uint32_t duty_cycle = ((float)(FLOW_RATE_2_TIM->CCR1) / FLOW_RATE_2_TIM->CCR2) * 100;
    uint32_t flow_rate = freq / 7.5;
    return flow_rate;
}
