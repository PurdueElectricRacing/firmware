/**
* @file imd.c
* @author Michael Gliane (mgliane@purdue.edu)
* @brief
* @version 2.0
* @date 2023-9-30
* 
* @copyright Copyright (c) 2023
*
*/

#include <stdio.h>
#include "imd.h"

int mhs_frequency[100];
int mls_frequency[100];
int mhs_array_idx = 0;
int mls_array_idx = 0;

/**
* @brief                Isolated testing environment using GPIOB Pin 3
*                       *Make sure to comment out if errrors are occuring
*/
int main() {
    // Setting up GPIO pin for PWM output
    RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB -> MODER &= ~(0x3 << 6);
    GPIOB -> MODER |= 0x2 << 6;
    GPIOB -> AFR[0] &= ~(0xf << 12);
    GPIOB -> AFR[0] |= 0x1 << 12;

    // Setting up PWM output on TIM 3
    RCC -> AHB1ENR |= RCC_AHB1ENR_TIM3EN;
    TIM2 -> PSC = (TargetCoreClockrateHz / 10000) - 1;  // Divides 80MHz by 8000 (=10000Hz)
    TIM2 -> ARR = 200 - 1;                              // Divides 10kHz by 200 (=50Hz)
    TIM2 -> CCR2 = 100;
    TIM2 -> CCMR1 &= ~TIM_CCMR1_OC2M_0;
    TIM2 -> CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

    TIM2 -> CCER |= TIM_CCER_CC2E;
    
    TIM2 -> CR1 |= TIM_CR1_CEN;

    while(1) {
    }
    return 0;
}

/**
* @brief                Enabling and configuring the proper GPIO pins (GPIOB pins 5-7)
*                       to read IMD channels
*/
void setup_IMDReading()  {
    RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB -> MODER &= ~(0x3 << 14); // Setting GPIOB pin 7 to general input mode (to read OKhs signal)
    GPIOB -> PUPDR |= 0x2 << 14;
    GPIOB -> MODER &= ~(0xf << 10); // Setting GPIOB pin 5-6 to general mode (to read duty signals Mhs and Mls)
    GPIOB -> MODER |= 0xa << 10;
    GPIOB -> AFR[0] &= ~((0xf << (4 * 5)) | (0xf << (4*6)));
    GPIOB -> AFR[0] |= (0x2 << (4 * 5)) | (0x2 << (4*6));
}

/**
* @brief                Enabling and configuring Timer 3 and 4 for reading IMD
*                       frequency. The capture frequency (100Hz) is twice the maximum
*                       frequency of the IMD (50Hz).
*/
void PHAL_setupTIMClk() {

    // Enabling Timer Clock 3
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3 -> PSC = (TargetCoreClockrateHz / 10000) - 1;  // Divides 16MHz by 1600 (=10000Hz)
    TIM3 -> ARR = 0xFFFF - 1;                           // Divides 10kHz by 100 (=100Hz)
    
    // Setting TIM3 to PWM input mode
    TIM3 -> CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1E);
    TIM3 -> CCMR1 &= ~(TIM_CCMR1_CC1S);                 // Selecting active input TI2
    TIM3 -> CCMR1 |= TIM_CCMR1_CC1S_1;
    TIM3 -> CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);  // Selecting active polarity for TI2FP1
    TIM3 -> CCMR1 &= ~(TIM_CCMR1_CC2S);                 // Selecting active input TI2
    TIM3 -> CCMR1 |= TIM_CCMR1_CC2S_0;
    TIM3 -> CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);  // Selecting falling edge for TI2FP1
    TIM3 -> CCER |= TIM_CCER_CC2P;
    TIM3 -> SMCR &= ~(TIM_SMCR_TS);                     // Selecting valid trigger input TI2FP2
    TIM3 -> SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_1;
    TIM3 -> SMCR &= ~(TIM_SMCR_SMS);                    // Configuring SM controller to reset mode
    TIM3 -> SMCR |= TIM_SMCR_SMS_2; 
    TIM3 -> CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;      // Enabling bits
    TIM3 -> CR1 |= TIM_CR1_CEN;
    
    
    // Enabling Timer Clock 4
    RCC -> APB1ENR |= RCC_APB1ENR_TIM4EN;
    TIM4 -> PSC = (TargetCoreClockrateHz / 10000) - 1;  // Divides 16MHz by 16000 (=10000Hz)
    TIM4 -> ARR = 0xFFFF - 1;                              // Divides 10kHz by 200 (=100Hz)

    // Setting TIM4 to PWM input
    TIM4 -> CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1E);
    TIM4 -> CCMR1 &= ~(TIM_CCMR1_CC1S);                 // Selecting active input TI1
    TIM4 -> CCMR1 |= TIM_CCMR1_CC1S_0;
    TIM4 -> CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);  // Selecting active polarity for TI1FP1
    TIM4 -> CCMR1 &= ~(TIM_CCMR1_CC2S);                 // Selecting active input TI2
    TIM4 -> CCMR1 |= TIM_CCMR1_CC2S_1;
    TIM4 -> CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);  // Selecting falling edge for TI1FP2
    TIM4 -> CCER |= TIM_CCER_CC2P;
    TIM4 -> SMCR &= ~(TIM_SMCR_TS);                     // Selecting valid trigger input TI1FP1
    TIM4 -> SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;
    TIM4 -> SMCR &= ~(TIM_SMCR_SMS);                    // Configuring SM controller to reset mode
    TIM4 -> SMCR |= TIM_SMCR_SMS_2; 
    TIM4 -> CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;      // Enabling bits
    TIM4 -> CR1 |= TIM_CR1_CEN;
}

/**
* @brief                Checks if the IMD's OKhs signal is high, signaling that there
*                       is no faulty resistance
*
*/
bool checkIMD_signal_OKhs() {
    TIM3 -> CR1 &= TIM_CR1_CEN;     // Temporarily turning off TIM3 to avoid new values whilst reading
    TIM4 -> CR1 &= TIM_CR1_CEN;     // Temporarily turning off TIM4 to avoid new values whilst reading
    if(((GPIOB -> IDR) & (0x1 << 7))) { // Checking GPIOB pin 7 to see if input is high
        TIM3 -> CR1 |= TIM_CR1_CEN;
        TIM4 -> CR1 |= TIM_CR1_CEN;
        return true;                // No fault detected
    }
    TIM3 -> CR1 |= TIM_CR1_CEN;
    TIM4 -> CR1 |= TIM_CR1_CEN;
    return false;                   // Fault detected
}

/**
* @brief                Checks and returns the frequency of the Mhs
*/
float checkIMD_signal_Mhs() {
    float cap_frequency = 0;
    int read_val = TIM3 -> CCR2;        // Reading period portion
    cap_frequency = (float) read_val;
    cap_frequency /= 10000;             // Shifting value into range
    cap_frequency = 1 / cap_frequency;  // Converting to frequency

    return cap_frequency;
}

/**
* @brief                Checks and returns the frequency of the Mls
*/
float checkIMD_signal_Mls() {
    float cap_frequency = 0;
    int read_val = TIM4 -> CCR1;        // Reading period portion
    cap_frequency = (float) read_val;
    cap_frequency /= 10000;             // Shifting value into range
    cap_frequency = 1 / cap_frequency;  // Converting to frequency

    return cap_frequency;
}