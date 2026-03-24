/**
 * @file main.c
 * @brief Bootloader entry point - bare-metal, no FreeRTOS
 */

#include "node_defs.h"
#include "bootloader/bootloader.h"
#include "common/bootloader/bootloader_common.h"

#ifdef STM32G474xx
#include "common/phal_G4/phal_G4.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/fdcan/fdcan.h"
#elif defined(STM32F407xx)
#include "common/phal_F4_F7/phal_F4_F7.h"
#include "common/phal/rcc.h"
#include "common/phal/gpio.h"
#include "common/phal/can.h"
#endif

/* Bootloader timeout - wait for CAN commands */
#define BOOTLOADER_INITIAL_TIMEOUT 3000 /* 3 seconds */
#define BOOTLOADER_FAST_BLINK_MS 500U
#define BOOTLOADER_SLOW_BLINK_MS 1000U

/* Clock configuration - 16 MHz HSE, no PLL */
static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSE,
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / 1),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / 1),
};

/* GPIO configuration for CAN and LED */
GPIOInitConfig_t gpio_config[] = {
    /* CAN RX/TX pins - configured per node */
    /* Status LED */
    GPIO_INIT_OUTPUT(BL_LED_PORT, BL_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
};

int main(void) {
#if defined(BOOTLOADER_ENABLED)
    // Confirm application launch to bootloader (not expected for bootloader target)
    Bootloader_ConfirmApplicationLaunch();
#endif

    uint32_t last_fast_led_toggle_ms = 0;
    uint32_t last_slow_led_toggle_ms = 0;

    /* Configure clocks */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        /* Clock config failed - stay in bootloader */
        while (1);
    }

    /* Initialize GPIO */
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        while (1);
    }

    /* Initialize SysTick for 1ms tick */
    SysTick_Config(SystemCoreClock / 1000);

    /* Anchor blink timers to avoid edge drift on loop jitter */
    last_fast_led_toggle_ms = bootloader_ms;
    last_slow_led_toggle_ms = bootloader_ms;

    /* Initialize CAN */
    BL_CANInit();

    /* Main loop - wait for CAN commands or timeout */
    while (bootloader_ms < BOOTLOADER_INITIAL_TIMEOUT || BL_flashStarted()) {
        BL_CANPoll();

        /* Toggle LED to indicate bootloader is active */
        if ((uint32_t)(bootloader_ms - last_fast_led_toggle_ms) >= BOOTLOADER_FAST_BLINK_MS) {
            last_fast_led_toggle_ms += BOOTLOADER_FAST_BLINK_MS;
            PHAL_writeGPIO(BL_LED_PORT, BL_LED_PIN, !PHAL_readGPIO(BL_LED_PORT, BL_LED_PIN));
        }
    }

    /* Attempt to boot application */
    BL_checkAndBoot();

    /* If boot fails, stay in bootloader forever */
    while (1) {
        BL_CANPoll();

        /* Slow blink LED to indicate boot failure */
        if ((uint32_t)(bootloader_ms - last_slow_led_toggle_ms) >= BOOTLOADER_SLOW_BLINK_MS) {
            last_slow_led_toggle_ms += BOOTLOADER_SLOW_BLINK_MS;
            PHAL_writeGPIO(BL_LED_PORT, BL_LED_PIN, !PHAL_readGPIO(BL_LED_PORT, BL_LED_PIN));
        }
    }

    return 0;
}
