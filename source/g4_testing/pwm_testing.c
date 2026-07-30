#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_PWM)

#include <stdint.h>

#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/pwm/pwm.h"
#include "common/utils/countof.h"

/*
* Initial test config:
*
* TIM1_CH1
* 1 kHz
* 50% duty cycle
*/
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_AF(GPIOA, 8, 6, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;

ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 16'000'000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = TargetCoreClockrateHz,
    .apb1_clock_target_hz      = TargetCoreClockrateHz,
    .apb2_clock_target_hz      = TargetCoreClockrateHz,
};

void HardFault_Handler(void);

int main() {
    if(PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    
    if(!PHAL_initGPIO(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }

    if(!PHAL_initPWM(TIM1, 1'000, 1)) {
        HardFault_Handler();
    }

    PHAL_PWMsetPercent(TIM1, 1, 50);

    while(1) {
        __asm__("nop");
    }

    return 0;
}

void HardFault_Handler(void) {
    __disable_irq();
    SysTick->CTRL = 0;

    while(1) {
        __asm__("nop");
    }
}

#endif