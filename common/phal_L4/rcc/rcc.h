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

#include "stm32l432xx.h"


#define HSE_CLOCK_RATE_HZ_INVALID (1)
#ifndef HSE_CLOCK_RATE_HZ
#define HSE_CLOCK_RATE_HZ HSE_CLOCK_RATE_HZ_INVALID /* Define this in order to configure clocks to use the HSE clock */
#endif  // HSE_CLOCK_RATE_HZ

#define HSI_CLOCK_RATE_HZ (16000000)

typedef enum {
    PLL_SRC_MSI,
    PLL_SRC_HSI16,
    PLL_SRC_HSE,
} PLL_SRC_t;

/* Globals for Clock Rates */
uint32_t APB1ClockRateHz;
uint32_t APB2ClockRateHz;
uint32_t AHB1ClockRateHz;
uint32_t AHB2ClockRateHz;
uint32_t PLLClockRateHz;

/**
 * @brief Configure PLL VCO Clock rate
 * The VCO clock is the input clock for the different PLL outputs.
 * Each PLL output will divide the VCO clock to get its output.
 * 
 * @param PLL Input rate for PLL, determined by PLL source
 * @param target_vco_output_rate_hz Target rate for PLL output
 * @return true 
 * @return false 
 */
bool PHAL_configurePLLVCO(PLL_SRC_t pll_source, uint32_t target_vco_output_rate_hz);

bool PHAL_configurePLLSystemClock(uint32_t system_clock_target);


#endif // _PHAL_PLL_H_