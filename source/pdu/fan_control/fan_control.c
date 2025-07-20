#include "fan_control.h"

#include "common/phal/gpio.h"

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;

bool fanControlInit() {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable clock to TIM1
    FAN_PWM_TIM->CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)

    /* Solving for PSC:
     * PWMfreq = Fclk / ((ARR + 1) × (PSC + 1)).
     *
     * PWMfreq is known, and we are setting ARR to 99 for ease of calculations.
     *
     * Therefore we solve for the below formula for the desired PSC based on the
     * necessary PWMfreq specified by the fan vendor.
    */

    FAN_PWM_TIM->ARR = 100 - 1; // Using this for ease of calculations

    FAN_PWM_TIM->PSC = (APB2ClockRateHz / (PWM_FREQUENCY_HZ * (FAN_PWM_TIM->ARR + 1))) - 1;

    FAN_PWM_TIM->CCR1 = 0; // Start with it off
    FAN_PWM_TIM->CCR2 = 0; // Start with it off

    // Set PWM mode 1 (active while CNT <= CCR1)
    FAN_PWM_TIM->CCMR1 &= ~TIM_CCMR1_OC1M;
    FAN_PWM_TIM->CCMR1 &= ~TIM_CCMR1_OC2M;
    FAN_PWM_TIM->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // PWM Mode 1 ch1
    FAN_PWM_TIM->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; // PWM Mode 1 ch2
    FAN_PWM_TIM->CCMR1 |= TIM_CCMR1_OC2PE | TIM_CCMR1_OC1PE; // Enable preload register ch1 & ch2

    FAN_PWM_TIM->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E; // Enable output compare 1 & 2

    FAN_PWM_TIM->BDTR |= TIM_BDTR_MOE; // Enable main output

    FAN_PWM_TIM->CR1 &= ~TIM_CR1_DIR; // Set to upcounting mode
    FAN_PWM_TIM->CR1 |= TIM_CR1_ARPE; // Necessary in upcounting mode

    FAN_PWM_TIM->CR1 |= TIM_CR1_CEN; // Enable counter (turn on timer)

    FAN_PWM_TIM->CR1 |= TIM_CR1_CEN; // Enable FAN_PWM_TIM

    return true;
}

// This speed will be between 0-100%
void setFan1Speed(uint8_t fan_speed) {
    // Duty cycle is (CCR1 / ARR)%. So CCR1 = (ARR / duty cycle)
    FAN_PWM_TIM->CCR1 = (FAN_PWM_TIM->ARR + 1) * (fan_speed / 100.0);
}

// This speed will be between 0-100%
void setFan2Speed(uint8_t fan_speed) {
    // Duty cycle is (CCR1 / ARR)%. So CCR1 = (ARR / duty cycle)
    FAN_PWM_TIM->CCR2 = (FAN_PWM_TIM->ARR + 1) * (fan_speed / 100.0);
}
