/**
 * @file imd.c
 * @brief ABOX IMD PWM measurement and state reporting
 *
 * @author Aditya Saini (saini19@purdue.edu)
 */

#include "imd.h"

#include "can_library/generated/A_BOX.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "main.h"

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
void imd_init(void) {
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
    tim->CR1 = TIM_CR1_URS | TIM_CR1_CEN;
}

// reads the latched capture and clears overflow. true if the capture is usable
static bool imd_read_capture(uint32_t *period_ticks, uint32_t *high_ticks) {
    TIM_TypeDef *tim = IMD_PWM_LS_TIM;

    uint32_t sr = tim->SR;
    bool captured = (sr & TIM_SR_CC1IF) != 0U; // rising edge capture
    *period_ticks = tim->CCR1;
    *high_ticks = tim->CCR2;
    bool overflow = (sr & TIM_SR_UIF) != 0U;
    tim->SR &= ~TIM_SR_UIF;

    // need a capture, no overflow, and period != 0
    return captured && !overflow && (*period_ticks != 0U);
}

static uint8_t imd_ticks_to_hz(uint32_t period_ticks) {
    return (uint8_t)((IMD_TIMER_TICK_HZ + (period_ticks / 2U)) / period_ticks);
}

static uint8_t imd_ticks_to_duty_pct(uint32_t high_ticks, uint32_t period_ticks) {
    return (uint8_t)(((high_ticks * 100U) + (period_ticks / 2U)) / period_ticks);
}

// push the newest sample and return the median of the last 3
static uint8_t imd_filter_freq(uint8_t freq_hz) {
    g_imd_freq_history[2] = g_imd_freq_history[1];
    g_imd_freq_history[1] = g_imd_freq_history[0];
    g_imd_freq_history[0] = freq_hz;
    return imd_median3(g_imd_freq_history[0], g_imd_freq_history[1], g_imd_freq_history[2]);
}

// TIM1 latches the latest PWM period (CCR1) and high time (CCR2) every cycle.
// called once a second to read them and report the IMD state.
void imd_periodic(void) {
    uint32_t period_ticks;
    uint32_t high_ticks;
    bool valid = imd_read_capture(&period_ticks, &high_ticks);

    uint8_t freq_hz = valid ? imd_ticks_to_hz(period_ticks) : 0U;
    g_imd_duty_pct = valid ? imd_ticks_to_duty_pct(high_ticks, period_ticks) : 0U;

    uint8_t filtered_hz = imd_filter_freq(freq_hz);
    imd_state_t state = imd_state_from_measurement(filtered_hz != 0U, filtered_hz);
    bool imd_status = PHAL_GPIO_read(IMD_STATUS_PORT, IMD_STATUS_PIN);

    CAN_SEND_imd_state(state, imd_status);
}

static_assert(IMD_STATE_PERIOD_MS == IMD_PERIOD_MS);
