//testing playground for pwm hal on f4 devboard (cannot just use pdu code)

#include "common/phal_F4_F7/pwm/pwm.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "f4_testing.h"
#include "stm32f407xx.h"

#if (F4_TESTING_CHOSEN == TEST_PWM)

#include "common/freertos/freertos.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"

GPIOInitConfig_t gpio_config[] = {
    //pulled from main.c     GPIO_INIT_AF(FAN_1_PWM_GPIO_Port, FAN_1_PWM_Pin, FAN_1_PWM_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(GPIOA, 8, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOA, 9, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOA, 10, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOA, 11, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),

    // GPIO_INIT_AF(GPIOB, 4, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOB, 5, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOB, 6, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_AF(GPIOB, 7, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    // GPIO_INIT_OUTPUT(GPIOC, 13, GPIO_OUTPUT_LOW_SPEED),

// GPIO_INIT_AF(GPIOB, 3, 1, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN), //T2 CH2
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 160000000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / (1)),
};

void HardFault_Handler();

// TODO add thread definitions here

int main() {
    osKernelInitialize();

    // // Initialize hardware
    // if (0 != PHAL_configureClockRates(&clock_config)) {
    //     HardFault_Handler();
    // }
    // if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
    //     HardFault_Handler();
    // }

    //scope settings 2v/div, 20us/div
    PHAL_initPWM(25000, TIM1, 1);
    PHAL_PWMsetPercent(TIM1, 1, 20);
    // PHAL_PWMsetPercent(TIM1, 2, 40);
    // PHAL_PWMsetPercent(TIM1, 3, 60);
    // PHAL_PWMsetPercent(TIM1, 4, 80);

    // PHAL_initPWM(25000, TIM3, 1);
    // PHAL_PWMsetPercent(TIM3, 1, 30);
    // PHAL_PWMsetPercent(TIM3, 2, 50);
    
    // PHAL_initPWM(25000, TIM4, 2);
    // PHAL_PWMsetPercent(TIM4, 1, 70);
    // PHAL_PWMsetPercent(TIM4, 2, 90);
    
    // PHAL_initPWM(25000, TIM2, 2);
    // PHAL_PWMsetPercent(TIM2, 2, 70);
    osKernelStart(); 
    return 0;
}


void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_PWM