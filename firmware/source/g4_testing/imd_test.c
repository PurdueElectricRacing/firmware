/**
 * @file imd_test.c
 * @brief Bench test for the A_BOX IMD PWM decoding logic on a Nucleo-G474RE.
 *
 * Mirrors source/a_box/imd/imd.c line for line, except the CAN send at the end
 * of imd_periodic() is replaced with writes to volatile globals so the result
 * can be watched in a debugger.
 *
 * Wiring:
 *   PA8 (TIM1_CH1, Nucleo D7)  <- function generator output (3.3 V square wave)
 *   PA9 (IMD status, Nucleo D8) <- optional, tie high/low to exercise imd_test_status
 *
 * Expected states (+/- 3 Hz):
 *   10 Hz NORMAL, 20 Hz UNDERVOLTAGE, 30 Hz SPEED_START,
 *   40 Hz DEVICE_ERROR, 50 Hz CONNECTION_FAULT, no signal OFF_OR_SHORT_CIRCUIT
 *
 * The median-of-3 filter and 1 s period mean a new state takes ~2 s to settle.
 */

#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_IMD)

#include <stdbool.h>
#include <stdint.h>

#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/pin_defs/g474ret6.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/rtos/rtos.h"
#include "common/utils/countof.h"
#include "main.h"

// Same pins as A_BOX (source/a_box/main.h)
#define IMD_STATUS_PORT (GPIOA)
#define IMD_STATUS_PIN  (9)
#define IMD_PWM_LS_TIM  (TIM1)

// Copied from source/a_box/imd/imd.h
static constexpr uint32_t IMD_PERIOD_MS = 1000;

// timer ticks this many times per sec
static constexpr uint32_t IMD_TIMER_TICK_HZ = 100000;
// max count before overflow
static constexpr uint32_t IMD_TIMER_ARR     = 0xFFFFU;

static constexpr uint8_t IMD_FREQ_TOLERANCE_HZ    = 3;
static constexpr uint8_t IMD_FREQ_NORMAL_HZ       = 10;
static constexpr uint8_t IMD_FREQ_UNDERVOLTAGE_HZ = 20;
static constexpr uint8_t IMD_FREQ_SPEED_START_HZ  = 30;
static constexpr uint8_t IMD_FREQ_DEVICE_ERROR_HZ = 40;
static constexpr uint8_t IMD_FREQ_CONN_FAULT_HZ   = 50;

// Matches imd_state_t in generators/configs/system/common_types.json
typedef enum : uint8_t {
    IMD_STATE_OFF_OR_SHORT_CIRCUIT = 0,
    IMD_STATE_NORMAL               = 1,
    IMD_STATE_UNDERVOLTAGE         = 2,
    IMD_STATE_SPEED_START          = 3,
    IMD_STATE_DEVICE_ERROR         = 4,
    IMD_STATE_CONNECTION_FAULT     = 5,
    IMD_STATE_UNKNOWN              = 6,
} imd_state_t;

// Watch these in the debugger
volatile imd_state_t imd_test_state   = IMD_STATE_OFF_OR_SHORT_CIRCUIT;
volatile bool imd_test_status         = false;
volatile uint8_t imd_test_raw_freq_hz = 0;
volatile uint8_t imd_test_filtered_hz = 0;
volatile uint8_t imd_test_duty_pct    = 0;
volatile uint32_t imd_test_period_ticks = 0;
volatile uint32_t imd_test_high_ticks   = 0;
volatile bool imd_test_valid          = false;
volatile uint32_t imd_test_run_count  = 0;

PHAL_GPIO_InitConfig_t gpio_config[] = {
    PHAL_GPIO_INIT_INPUT(IMD_STATUS_PORT, IMD_STATUS_PIN, GPIO_INPUT_OPEN_DRAIN),
    // IMD PWM measurement (TIM1_CH1 input)
    PHAL_PIN_DEFS_TIM1_CH1_PA8,
    PHAL_GPIO_INIT_OUTPUT(LED_NUCLEO_GREEN_PORT, LED_NUCLEO_GREEN_PIN, GPIO_OUTPUT_LOW_SPEED),
};

void HardFault_Handler();

static void imd_init(void);
static void imd_periodic(void);

RTOS_DEFINE_TASK(imd_periodic, IMD_PERIOD_MS, TASK_PRIORITY_LOW, STACK_512);

int main() {
    PHAL_RCC_init(PHAL_RCC_HSI_16MHZ);

    if (!PHAL_GPIO_init(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }
    imd_init();

    RTOS_START_TASK(imd_periodic);

    vTaskStartScheduler();

    return 0;
}

// ---------------------------------------------------------------------------
// Everything below is identical to source/a_box/imd/imd.c except where noted
// ---------------------------------------------------------------------------

static uint8_t g_imd_duty_pct = 0;
// last 3 frequency samples for noise filtering
static uint8_t g_imd_freq_history[3] = {0};

// for noise resilience
static uint8_t imd_median3(uint8_t a, uint8_t b, uint8_t c) {
    if (a > b) { uint8_t t = a; a = b; b = t; }
    if (b > c) { uint8_t t = b; b = c; c = t; }
    if (a > b) { uint8_t t = a; a = b; b = t; }
    return b;
}

static bool imd_freq_near(uint8_t freq_hz, uint8_t nominal_hz) {
    uint8_t low  = (nominal_hz > IMD_FREQ_TOLERANCE_HZ) ? (nominal_hz - IMD_FREQ_TOLERANCE_HZ) : 0;
    uint8_t high = nominal_hz + IMD_FREQ_TOLERANCE_HZ;
    return (freq_hz >= low) && (freq_hz <= high);
}

static imd_state_t imd_state_from_measurement(bool valid, uint8_t freq_hz) {
    if (!valid) {
        return IMD_STATE_OFF_OR_SHORT_CIRCUIT;
    }
    if (imd_freq_near(freq_hz, IMD_FREQ_NORMAL_HZ)) return IMD_STATE_NORMAL;
    if (imd_freq_near(freq_hz, IMD_FREQ_UNDERVOLTAGE_HZ)) return IMD_STATE_UNDERVOLTAGE;
    if (imd_freq_near(freq_hz, IMD_FREQ_SPEED_START_HZ)) return IMD_STATE_SPEED_START;
    if (imd_freq_near(freq_hz, IMD_FREQ_DEVICE_ERROR_HZ)) return IMD_STATE_DEVICE_ERROR;
    if (imd_freq_near(freq_hz, IMD_FREQ_CONN_FAULT_HZ)) return IMD_STATE_CONNECTION_FAULT;
    return IMD_STATE_UNKNOWN;
}

// set up TIM1 to measure PWM on PA8 for us
static void imd_init(void) {
    TIM_TypeDef *tim = IMD_PWM_LS_TIM;

    // turn TIM1's clock on
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    tim->CR1 = 0U;

    // get clock speed
    uint32_t timer_clock_hz = PHAL_RCC_getAPB2ClockHz();
    // divide to get 100 kHz (1 tick = 10 us)
    uint32_t divider = (timer_clock_hz + (IMD_TIMER_TICK_HZ / 2U)) / IMD_TIMER_TICK_HZ;
    if (divider == 0U) {
        divider = 1U;
    }
    // Prescaler and Autoreload (timers max count)
    tim->PSC = (uint16_t)(divider - 1U);
    tim->ARR = IMD_TIMER_ARR;

    // look at the same pin. apply input filter to noise
    tim->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_0
                | TIM_CCMR1_CC2S_1 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_0;

    // capture rising edges (period), falling edges (time high)
    tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;
    // reset the counter on every rising edge
    tim->SMCR = TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;

    tim->EGR = TIM_EGR_UG;
    tim->SR  = 0U;
    // URS: only a real overflow sets UIF, not the reset on each rising edge
    tim->CR1 = TIM_CR1_URS | TIM_CR1_CEN;
}

// TIM1 captures the PWM period (CCR1) and high time (CCR2) on every
// cycle, so the most recent cycle is always sitting in the capture registers.
// imd_periodic() samples those latched values once a
// second.
static void imd_periodic(void) {
    TIM_TypeDef *tim = IMD_PWM_LS_TIM;

    uint32_t sr = tim->SR;
    bool captured = (sr & TIM_SR_CC1IF) != 0U; // rising edge capture
    uint32_t period_ticks = tim->CCR1;
    uint32_t high_ticks = tim->CCR2;
    bool overflow = (sr & TIM_SR_UIF) != 0U;
    tim->SR &= ~TIM_SR_UIF;

    // need a capture, no overflow, and period != 0
    bool valid = captured && !overflow && (period_ticks != 0U);
    uint8_t freq_hz = 0;
    if (valid) {
        freq_hz = (uint8_t)((IMD_TIMER_TICK_HZ + (period_ticks / 2U)) / period_ticks);
        g_imd_duty_pct = (uint8_t)(((high_ticks * 100U) + (period_ticks / 2U)) / period_ticks);
    } else {
        g_imd_duty_pct = 0;
    }

    g_imd_freq_history[2] = g_imd_freq_history[1];
    g_imd_freq_history[1] = g_imd_freq_history[0];
    g_imd_freq_history[0] = valid ? freq_hz : 0U;
    uint8_t filtered_hz = imd_median3(g_imd_freq_history[0], g_imd_freq_history[1], g_imd_freq_history[2]);

    imd_state_t state = imd_state_from_measurement(filtered_hz != 0U, filtered_hz);
    bool imd_status = PHAL_GPIO_read(IMD_STATUS_PORT, IMD_STATUS_PIN);

    // Test-only: replaces CAN_SEND_imd_state(state, imd_status)
    imd_test_state        = state;
    imd_test_status       = imd_status;
    imd_test_valid        = valid;
    imd_test_raw_freq_hz  = freq_hz;
    imd_test_filtered_hz  = filtered_hz;
    imd_test_duty_pct     = g_imd_duty_pct;
    imd_test_period_ticks = period_ticks;
    imd_test_high_ticks   = high_ticks;
    imd_test_run_count++;
    PHAL_GPIO_toggle(LED_NUCLEO_GREEN_PORT, LED_NUCLEO_GREEN_PIN);
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_IMD
