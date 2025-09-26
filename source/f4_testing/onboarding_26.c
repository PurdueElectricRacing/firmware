<<<<<<< Updated upstream
/**
 * @file onboarding_26.c
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief  Onboarding 26 starter file
 * @version 0.1
 * @date 2026-07-27
 *
 * @copyright Copyright (c) 2026
 *
 */
=======
    /**
    * @file onboarding_26.c
    * @author Ronit Podder rpodder@purdue.edu
    * @brief  Onboarding 26 starter file
    * @version 0.1
    * @date 2026-07-27
    *
    * @copyright Copyright (c) 2026
    *
    */
>>>>>>> Stashed changes

    #include "f4_testing.h"

    #if (F4_TESTING_CHOSEN == TEST_ONBOARDING_26)

    #include "common/freertos/freertos.h"
    #include "common/phal/gpio.h"
    #include "common/phal/rcc.h"

<<<<<<< Updated upstream
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
    // TODO: Add GPIO LEDs here...
};
=======
    GPIOInitConfig_t gpio_config[] = {
    
        GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
        GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED), 
        // TODO: Add GPIO LEDs here...  
    };
>>>>>>> Stashed changes

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

<<<<<<< Updated upstream
void HardFault_Handler();
void ledblink1();
// TODO add more function definitions here

defineThreadStack(ledblink1, 250, osPriorityNormal, 64);

// TODO add thread definitions here

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

    osKernelStart(); // Go!

    return 0;
}

void ledblink1() {
    // TODO: add blink function here
}

// TODO: add more function definitions here

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}
=======
    volatile bool interruptMode = false; 
    void HardFault_Handler();
    void interrupt();
    void ledblink1() {
        while (1) {
            PHAL_toggleGPIO(GPIOD, 12);
            osDelay(interruptMode ? 50 : 100);
        }
    }

    void ledblink2() {
        while (1) {
            PHAL_toggleGPIO(GPIOD, 13);
            osDelay(interruptMode ? 125 : 250);
        }
    }
>>>>>>> Stashed changes

    void ledblink3() {
        while (1) {
            PHAL_toggleGPIO(GPIOD, 14);
            osDelay(interruptMode ? 250 : 500);
        }
    }

    void ledblink4() {
        while (1) {
            PHAL_toggleGPIO(GPIOD, 15);
            osDelay(interruptMode ? 500 : 1000);
        }
    } 



    // TODO add more function definitions here

    defineThreadStack(ledblink1, 100 , osPriorityNormal, 64);
    defineThreadStack(ledblink2, 250 , osPriorityNormal, 64);
    defineThreadStack(ledblink3, 500 , osPriorityNormal, 64);
    defineThreadStack(ledblink4, 1000 , osPriorityNormal, 64);


    // TODO add thread definitions here


    int main() {
        osKernelInitialize();

        // Initialize hardware
        if (0 != PHAL_configureClockRates(&clock_config)) {
            HardFault_Handler();
        }
        if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
            HardFault_Handler();
        }
        interrupt();
        // Create threads
        createThread(ledblink1);
        createThread(ledblink2);
        createThread(ledblink3);
        createThread(ledblink4);

    // TODO: Create threads here
        

        osKernelStart(); // Go!

        return 0;
    }


    // TODO: add blink function here


    void interrupt() {
        // Enable SYSCFG clock
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
        // Route EXTI7 to Port B
        SYSCFG->EXTICR[1] |= (0b0000 << 0);
        // Unmask EXTI7 interrupt
        EXTI->IMR |= (0x1 << 0);
        // Enable rising edge trigger on EXTI7
        EXTI->RTSR |= (0x1 << 0);

    // Disable falling edge trigger on EXTI7
        EXTI->FTSR &= ~(0x1 << 0);
        // Enable EXTI0 in NVIC
        NVIC_EnableIRQ(EXTI0_IRQn);

    // Interrupt handler for EXTI7    
    }

    // TODO: add more function definitions here

        void HardFault_Handler() {
        while (1) {
            __asm__("nop");
        }
        }
        void EXTI0_IRQHandler() {
        // Check who triggered this interrupt
        if (EXTI->PR & (1 << 0)) {
            // Clear the pending bit
            EXTI->PR |= (1 << 0);
        
            interruptMode = !interruptMode;  
            // do something cool here
        }
    }

    #endif // F4_TESTING_CHOSEN == TEST_ONBOARDING_26
