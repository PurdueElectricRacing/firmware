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

#include "common/phal_L4/rcc/rcc.h"
#include "system_stm32l4xx.h"

// Local Macros
#define HZ_TO_MHZ(hz) ((uint32_t)((uint32_t) hz / ((uint32_t)1e6)))


bool PHAL_configurePLLVCO(PLL_SRC_t pll_source, uint32_t target_vco_output_rate_hz)
{
    // Valid range for PLL output is 64Mhx <= PLL_CLK <= 344Mhz
    target_vco_output_rate_hz = target_vco_output_rate_hz > (uint32_t) 344e6 ? (uint32_t) 344e6 : target_vco_output_rate_hz;
    target_vco_output_rate_hz = target_vco_output_rate_hz < (uint32_t) 64e6 ? (uint32_t) 64e6 : target_vco_output_rate_hz;
    
    RCC->CR &= ~RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY))
        ; // Wait for PLL to turn off

    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC_Msk | RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLM_Msk);

    /* Determine input clock rate from the selected source */
    uint32_t pll_input_f_hz;
    switch (pll_source)
    {
        case PLL_SRC_HSI16:
            RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;
            RCC->CR |= RCC_CR_HSION;
            while(!(RCC->CR & RCC_CR_HSIRDY))
                ; // Wait for HSI to turn on
            pll_input_f_hz = HSI_CLOCK_RATE_HZ;
            break;
        
        case PLL_SRC_MSI:
            RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI;
            pll_input_f_hz = 0; //TODO: Implement!
            break;

        case PLL_SRC_HSE:
            if (HSE_CLOCK_RATE_HZ == HSE_CLOCK_RATE_HZ_INVALID)
            {
                return false; // Make sure you set the HSE_CLOCK_RATE_HZ to what is on the PCB!
            }
            RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
            pll_input_f_hz = HSE_CLOCK_RATE_HZ;
            break;
    }

    /* Search for a possible PLL configuration */
    uint8_t pll_input_divisor = 1;
    uint8_t pll_output_multiplier = 8;
    bool valid_rate = false;
    for (; pll_input_divisor <= 8 && (!valid_rate); pll_input_divisor ++)
    {
        // VCO input clock is only allowed between 4MHz and 16MHz
        uint32_t pll_vco_in_rate = pll_input_f_hz / pll_input_divisor;
        if(pll_vco_in_rate < 4000000 || pll_vco_in_rate > 16000000)
        {
            continue;
        }

        for (; pll_output_multiplier <= 86; pll_output_multiplier ++)
        {
            if ((pll_input_f_hz / pll_input_divisor) * pll_output_multiplier == target_vco_output_rate_hz)
            {
                valid_rate = true;
                break;
            }
        }
        if (valid_rate)
            break;
    }

    if (!valid_rate)
    {
        return false; // Unable to find a valid clock rate!
    }

    RCC->PLLCFGR |= ((pll_input_divisor - 1) << RCC_PLLCFGR_PLLM_Pos) & RCC_PLLCFGR_PLLM_Msk;
    RCC->PLLCFGR |= ((pll_output_multiplier) << RCC_PLLCFGR_PLLN_Pos) & RCC_PLLCFGR_PLLN_Msk;

    // Update global variable used to reference the PLL
    PLLClockRateHz = (pll_input_f_hz / pll_input_divisor) * pll_output_multiplier;

    RCC->PLLCFGR |= RCC_PLLCFGR_PLLPEN;
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY))
        ; // Wait for PLL to turn on

    SystemCoreClockUpdate();
}

bool PHAL_configurePLLSystemClock(uint32_t system_clock_target)
{
    if (system_clock_target > 80000000)
    {
        system_clock_target = 80000000;
    }

    // Valid number for PLLR divisor are 2,4,6,8 (2 bit encoded)
    uint8_t pll_r_divisor = PLLClockRateHz / system_clock_target;
    if(pll_r_divisor == 0 || pll_r_divisor % 2 != 0 || pll_r_divisor > 8)
    {
        return false;
    }

    // Enable PLLR and set the PLLR divisor
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR_Msk;
    RCC->PLLCFGR |= (((pll_r_divisor / 2) - 1) << RCC_PLLCFGR_PLLR_Pos) & RCC_PLLCFGR_PLLR_Msk;

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS_PLL != RCC_CFGR_SWS_PLL))
        ; // Wait for PLL to be the new system clock

    SystemCoreClockUpdate();

}