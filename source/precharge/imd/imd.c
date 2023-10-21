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
* @brief Enabling and configuring Timer 3 and 4 for reading IMD frequency.
*        The capture frequency (100Hz) is twice the maximum frequency of the IMD (50Hz).
*/
void PHAL_setupTIMClk() {
    // Enabling Timer Clock 3
    RCC -> APB1ENR |= RCC_AHB1ENR_TIM3EN;
    TIM3 -> PSC = (TargetCoreClockrateHz / 10000) - 1;  // Divides 80MHz by 8000 (=10000Hz)
    TIM3 -> ARR = 100 - 1;                              // Divides 10kHz by 100 (=100Hz)
    
    TIM3 -> DIER |= TIM_DIER_UIE;                       // Enables update interrupt
    TIM3 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1 << TIM3_DAC_IRQn;


    // Enabling Timer Clock 4
    RCC -> APB1ENR |= RCC_AHB1ENR_TIM4EN;
    TIM4 -> PSC = (TargetCoreClockrateHz / 10000) - 1;  // Divides 80MHz by 8000 (=10000Hz)
    TIM4 -> ARR = 100 - 1;                              // Divides 10kHz by 100 (=100Hz)

    TIM4 -> DIER |= TIM_DIER_UIE;                       // Enables update interrupt
    TIM4 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1 << TIM4_DAC_IRQn;
}

/**
* @brief                Checks if the IMD's OKhs signal is high, signaling that there
*                       is no faulty resistance
*
*/
bool checkIMD_signal_OKhs() {
    if(((GPIOB -> IDR >> 7) & 0x1) == 1) { // Checking GPIOB pin 7 to see if input is high
        return true;                // No fault detected
    }
    return false;                   // Fault detected
}