/**
 * @file rcc.h
 * @author Eileen Yoon - Port of L4 RCC by Adam Busch (busch8@purdue.edu)
 * @brief RCC Configuration Driver for STM32F4 Devices
 * @version 0.1
 * @date 2023-08-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __PHAL_G4_RCC_H__
#define __PHAL_G4_RCC_H__

#include "common/phal_G4/phal_G4.h"

#define HSE_CLOCK_RATE_HZ (16000000)

#define HSE_CLOCK_RATE_HZ_INVALID (1) /* High Speed External oscilator value */
#ifndef HSE_CLOCK_RATE_HZ
#define HSE_CLOCK_RATE_HZ HSE_CLOCK_RATE_HZ_INVALID /* Define this in order to configure clocks to use the HSE clock */
#endif // HSE_CLOCK_RATE_HZ

#define HSI_CLOCK_RATE_HZ (16000000)
#define MCO_OUT_PIN       (8)

// RCC Constants
#if defined(STM32G474xx)

#define RCC_MAX_VCO_RATE_HZ           ((uint32_t)128e6)
#define RCC_MIN_VCO_RATE_HZ           ((uint32_t)96e6)
#define RCC_MIN_PLL_INPUT_DIVISOR     (2U)
#define RCC_MAX_PLL_INPUT_DIVISOR     (63U)
#define RCC_MIN_PLL_OUTPUT_MULTIPLIER (50U)
#define RCC_MAX_PLL_OUTPUT_MULTIPLIER (432U)
#define RCC_MAX_SYSCLK_TARGET_HZ      (168000000)

#else
#error "Please define a MCU arch"
#endif

typedef enum {
    PLL_SRC_HSI16,
    PLL_SRC_HSE
} PLLSrc_t;

typedef enum {
    MCO1_SRC_HSI = 0,
    MCO1_SRC_LSE = 1,
    MCO1_SRC_HSE = 2,
    MCO1_SRC_PLL = 3,

} MCO1Source_t;

typedef enum {
    MCO_DIV_NONE = 0,
    MCO_DIV_2    = 4,
    MCO_DIV_3    = 5,
    MCO_DIV_4    = 6,
    MCO_DIV_5    = 7

} MCODivisor_t;

typedef enum {
    CLOCK_SOURCE_HSI = 0,
    CLOCK_SOURCE_HSE = 1,
} ClockSrc_t;

typedef enum {
    RCC_ERROR_AHB_INIT    = 0,
    RCC_ERROR_APB1_INIT   = 1,
    RCC_ERROR_APB2_INIT   = 2,
    RCC_ERROR_HSI_INIT    = 3,
    RCC_ERROR_PLLSYS_INIT = 4,
    RCC_ERROR_PLLVCO_INIT = 5,
    RCC_ERROR_HSE_INIT    = 6,
} RCCErrors_t;

typedef struct {
    ClockSrc_t clock_source; /* Use HSE or not */
    bool use_pll; /* Use PLL or not */
    uint32_t system_clock_target_hz; /* System Core Clock rate */
    uint32_t ahb_clock_target_hz; /* AHB clock rate target */
    uint32_t apb1_clock_target_hz; /* APB1 clock rate target */
    uint32_t apb2_clock_target_hz; /* APB2 clock rate target */

    /* Only used for use_pll == true */
    PLLSrc_t pll_src; /* Input source for PLL VCO */
    uint32_t vco_output_rate_target_hz; /* VCO output target rate */
    uint32_t msi_output_rate_target_hz; /* Use if pll_src == MSI */
} ClockRateConfig_t;

/**
 * @brief Configure all AHB/APB/System clocks from the provided configuration.
 *
 * @param config Configuration to try to match
 * @return Binary encoded representation of which clocks were unsuccessfuly configured.
 *  return value of 0 means all clocks were sucessfully configured.
 */
uint8_t PHAL_configureClockRates(ClockRateConfig_t* config);

/**
 * @brief Configure PLL VCO Clock rate
 * The VCO clock is the input clock for the different PLL outputs.
 * Each PLL output will divide the VCO clock to get its output.
 *
 * @param PLL Input rate for PLL, determined by PLL source
 * @param vco_output_rate_target_hz Target rate for PLL output
 * @return true
 * @return false
 */
bool PHAL_configurePLLVCO(PLLSrc_t pll_source, uint32_t vco_output_rate_target_hz);

/**
 * @brief Configure PLL CLK as the System Clock at the desired target frequency.
 * SHOULD BE DONE BEFORE ANY OF THE AHB OR APB CLOCKS ARE CHANGED
 *
 * @param system_clock_target_hz
 * @return true Successfully configured PLL clock as system clock
 * @return false
 */
bool PHAL_configurePLLSystemClock(uint32_t system_clock_target_hz);

/**
 * @brief Configure HSI CLK as the System Clock.
 * SHOULD BE DONE BEFORE ANY OF THE AHB OR APB CLOCKS ARE CHANGED
 *
 * @return true Successfully configured HSI clock as system clock
 * @return false
 */
bool PHAL_configureHSISystemClock();

/**
 * @brief Configure HSE CLK as the System Clock.
 * SHOULD BE DONE BEFORE ANY OF THE AHB OR APB CLOCKS ARE CHANGED
 *
 * @return true Successfully configured HSE clock as system clock
 * @return false
 */
bool PHAL_configureHSESystemClock();

/**
 * @brief Configure AHB Clock rate by modifying the AHB prescaler value.
 *
 * @param ahb_clock_target_hz
 * @return true Successfully configured AHB clock rate to @param ahb_clock_target_hz
 * @return false
 */
bool PHAL_configureAHBClock(uint32_t ahb_clock_target_hz);

/**
 * @brief Configure APB1 Clock rate by modifying the APB1 prescaler value.
 *
 * @param apb1_clock_target_hz
 * @return true Successfully configured AHB clock rate to @param apb1_clock_target_hz
 * @return false
 */
bool PHAL_configureAPB1Clock(uint32_t apb1_clock_target_hz);

/**
 * @brief Configure APB1 Clock rate by modifying the APB2 prescaler value.
 *
 * @param apb2_clock_target_hz
 * @return true Successfully configured AHB clock rate to @param apb2_clock_target_hz
 * @return false
 */
bool PHAL_configureAPB2Clock(uint32_t apb2_clock_target_hz);

#endif // __PHAL_G4_RCC_H__
