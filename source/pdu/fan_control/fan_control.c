#include "fan_control.h"

extern uint32_t APB2ClockRateHz;

bool fanControlInit()
{
    RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable clock to TIM1
    FAN_PWM_TIM -> CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)

    // Set PWM mode 1 (active while CNT <= CCR1)
    FAN_PWM_TIM -> CCMR1 &= ~TIM_CCMR1_OC1M_0;
    FAN_PWM_TIM -> CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

    FAN_PWM_TIM -> CCER |= TIM_CCER_CC1E; // Enable output compare 1

    // NOTE: Set up regs so that CCR1 can be directly set from 0-100
    FAN_PWM_TIM -> PSC = (APB2ClockRateHz / PWM_FREQUENCY_HZ) - 1;
    
    FAN_PWM_TIM -> CR1 |= TIM_CR1_CEN; // Enable counter (turn on timer)
    
    return true;
}

// This speed will be between 0-100%
void setFanSpeed(uint8_t fan_speed)
{
    FAN_PWM_TIM -> CCR1 = fan_speed;
}
