/**
 * @file onboarding_26.c
 * @author Anson Lam (lam217@purdue.edu)
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

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOB, 9, GPIO_OUTPUT_LOW_SPEED), 
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // Green LED
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED), // Orange LED
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED), // Red LED
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED), // Blue LED
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

// TODO add more function definitions here
void ledblink1();
void hb_led_blink_task();
void orange_led_blink_task();
void red_led_blink_task();
void blue_led_blink_task();

// TODO add thread definitions here
defineThreadStack(ledblink1, 100, osPriorityNormal, 64);
defineThreadStack(hb_led_blink_task, 500, osPriorityNormal, 64);
defineThreadStack(orange_led_blink_task, 250, osPriorityNormal, 64);
defineThreadStack(red_led_blink_task, 500, osPriorityNormal, 64);
defineThreadStack(blue_led_blink_task, 1000, osPriorityNormal, 64);

int main() {
    osKernelInitialize();

    // Initialize hardware
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // Create threads
    createThread(ledblink1);
    
    // TODO: Create threads here
    createThread(hb_led_blink_task);
    createThread(orange_led_blink_task);
    createThread(red_led_blink_task);
    createThread(blue_led_blink_task);

    osKernelStart(); // Go!

    return 0;
}

void ledblink1() {
    PHAL_toggleGPIO(GPIOD, 12);

}

// TODO: add more function definitions here

void hb_led_blink_task() {
    PHAL_toggleGPIO(GPIOB, 9); 
}

void orange_led_blink_task() {
    PHAL_toggleGPIO(GPIOD, 13); 
}

void red_led_blink_task() {
    PHAL_toggleGPIO(GPIOD, 14);
}

void blue_led_blink_task() {
    PHAL_toggleGPIO(GPIOD, 15);
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_ONBOARDING_26
