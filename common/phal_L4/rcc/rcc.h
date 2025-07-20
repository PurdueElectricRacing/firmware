/**
 * @file rcc.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief RCC Configuration Driver for STM32L432 Devices
 * @version 0.1
 * @date 2021-10-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _PHAL_RCC_H_
#define _PHAL_RCC_H_

#include <inttypes.h>
#include <stdbool.h>

#include "stm32l4xx.h"

#define HSE_CLOCK_RATE_HZ_INVALID (1) /* High Speed External oscilator value */
#ifndef HSE_CLOCK_RATE_HZ
#define HSE_CLOCK_RATE_HZ HSE_CLOCK_RATE_HZ_INVALID /* Define this in order to configure clocks to use the HSE clock */
#endif // HSE_CLOCK_RATE_HZ

#define HSI_CLOCK_RATE_HZ (16000000)

typedef enum {
    PLL_SRC_MSI,
    PLL_SRC_HSI16,
    PLL_SRC_HSE,
} PLLSrc_t;

typedef enum {
    SYSTEM_CLOCK_SRC_PLL,
    SYSTEM_CLOCK_SRC_HSI,
    SYSTEM_CLOCK_SRC_HSE,
} SystemClockSrc_t;

typedef struct {
    SystemClockSrc_t system_source; /* System Core Clock source */
    uint32_t system_clock_target_hz; /* System Core Clock rate */
    uint32_t ahb_clock_target_hz; /* AHB clock rate target */
    uint32_t apb1_clock_target_hz; /* APB1 clock rate target */
    uint32_t apb2_clock_target_hz; /* APB2 clock rate target */

    /* Only used for system_source == PLL */
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

/**
 * @brief Configure MSI Clock rate
 * 
 * @param target_hz
 * @return true Successfully configured MSI clock rate to @param target_hz
 * @return false 
 */
bool PHAL_configureMSIClock(uint32_t target_hz);

#endif // _PHAL_PLL_H_