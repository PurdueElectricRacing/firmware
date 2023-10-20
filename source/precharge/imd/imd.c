/**
* @file imd.c
* @author Michael Gliane (mgliane@purdue.edu)
* @brief
* @version 0.1
* @date 2023-9-30
* 
* @copyright Copyright (c) 2023
*
*/

#include "imd.h"

/**
* @brief                enables and sets the proper GPIO pins to read IMD channels
* 
*/
void setup_IMDReading()  {
    RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB -> MODER &= ~(0x3 << 14); // Setting GPIOB pin 7 to general input mode (to read OKhs signal)
    GPIOB -> MODER &= ~(0xf << 10); // Setting GPIOB pin 5-6 to general mode (to read duty signals Mhs and Mls)
}



/**
* @brief Enabling and configuring Timer 3 and 4 for reading IMD frequency
*
* @param prescl_tim3 prescaler for Timer 3 clock
* @param prescl_tim4 prescaler for Timer 4 clock
* @param tim3_trigger trigger type for Timer 3
* @param tim4_trigger trigger type for Timer 4
*/
void PHAL_setupTIMClk(uint32_t prescl_tim3, uint32_t prescl_tim4, TimerTriggerSelection_t tim3_trigger, TimerTriggerSelection_t tim4_trigger) {
    // Enabling Timer Clock 3
    RCC -> APB1ENR |= RCC_AHB1ENR_TIM3EN;
    TIM3 -> PSC = prescl_tim3 - 1;
    TIM3 -> CR1 |= TIM_CR1_URS;
    TIM3 -> SMCR |= (trigger_select << TIM_SMCR_TS_Pos) & TIM_SMCR_TS;
    TIM3 -> SMCR |= TIM_SMCR_SMS_2;
    TIM3 -> CR1 |= TIM_CR1_CEN;

    // Enabling Timer Clock 4
    RCC -> APB1ENR |= RCC_AHB1ENR_TIM4EN;
    TIM4 -> PSC = prescl_tim4 - 1;
    TIM4 -> CR1 |= TIM_CR1_URS;
    TIM4 -> SMCR |= (trigger_select << TIM_SMCR_TS_Pos) & TIM_SMCR_TS;
    TIM4 -> SMCR |= TIM_SMCR_SMS_2;
    TIM4 -> CR1 |= TIM_CR1_CEN;
}

/**
* @brief                Checks if the IMD's OKhs signal is high, signaling that there
*                       is no faulty resistance
*
*/
bool checkIMD_signal_OKhs() {
    if((GPIOB -> IDR >> 7) & 0x1) { // Checking GPIOB pin 7 to see if input is high
        return true;                // No fault detected
    }
    return false;                   // Fault detected
}