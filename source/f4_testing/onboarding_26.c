/**
 * @file onboarding_26.c
 * @author Danny Proano
 * @brief  Onboarding 26 starter file
 * @version 0.1
 * @date 2026-07-27
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "f4_testing.h"

#if (F4_TESTING_CHOSEN == TEST_ONBOARDING_26)

#include "common/freertos/freertos.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include <stdbool.h>

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOB, 9, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED)
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source = CLOCK_SOURCE_HSI,
    .use_pll = false,
    .vco_output_rate_target_hz = 160000000,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz = (TargetCoreClockrateHz / (1)),
};

void HardFault_Handler();
void ledblink1();
void ledblink2();
void ledblink3();
void ledblink4();
void ledblink5();

volatile bool fast = false;

defineThreadStack(ledblink1, 100, osPriorityNormal, 64);
defineThreadStack(ledblink2, 100, osPriorityNormal, 64);
defineThreadStack(ledblink3, 40, osPriorityNormal, 64);
defineThreadStack(ledblink4, 20, osPriorityNormal, 64);
defineThreadStack(ledblink5, 10, osPriorityNormal, 64);

// Setup for interrupts
void interrupt_setup() {
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	SYSCFg->EXTICR[1] |= (0b0001 << 12);

	EXTI->RTSR |= (1 << 7);
	EXTI->FTSR &= ~(1 << 7);

	EXTI->IMR |= (1 << 7);

	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler() {
    if (EXTI->PR & (1 << 7)) {
        EXTI->PR |= (1 << 7);
        
        fast = !fast;
    }
}

int main() {
    osKernelInitialize();

    interrupt_setup();

    // Initialize hardware
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // Create threads
    createThread(ledblink1);
    createThread(ledblink2);
    createThread(ledblink3);
    createThread(ledblink4);
    createThread(ledblink5);

    osKernelStart(); // Go!

    return 0;
}

void ledblink1() {
    while (1) {
        PHAL_toggleGPIO(GPIOB, 9);
        
        if (fast) {
          osDelay(125);
        } else {
          osDelay(250);
        }
    }
}

void ledblink2() {
    while (1) {
        PHAL_toggleGPIO(GPIOD, 12);
        osDelay(250);
    }

}

void ledblink3() {
    while (1) {
        PHAL_toggleGPIO(GPIOD, 13);
        osDelay(250);
    }

}

void ledblink4() {
    while (1) {
        PHAL_toggleGPIO(GPIOD, 14);
        osDelay(250);
    }

}

void ledblink5() {
    while (1) {
        PHAL_toggleGPIO(GPIOD, 15);
        osDelay(250);
    }

}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_ONBOARDING_26
