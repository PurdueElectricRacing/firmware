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

/* Globals for Clock Rates */
uint32_t APB1ClockRateHz;
uint32_t APB2ClockRateHz;
uint32_t AHBClockRateHz;
uint32_t PLLClockRateHz;


uint8_t PHAL_configureClockRates(ClockRateConfig_t* config)
{
    uint8_t ret_code = 0;
    switch(config->system_source)
    {
        case SYSTEM_CLOCK_SRC_PLL:
            ret_code |= (!PHAL_configurePLLVCO(config->pll_src, config->vco_output_rate_target_hz)) << 7;
            ret_code |= (!PHAL_configurePLLSystemClock(config->system_clock_target_hz)) << 6;
            break;
        case SYSTEM_CLOCK_SRC_HSE: // TODO: Implement
        case SYSTEM_CLOCK_SRC_HSI: // TODO: Implement
        default: 
            return 0xFF;
    }
    ret_code |= (!PHAL_configureAHBClock(config->ahb_clock_target_hz))     << 0;
    ret_code |= (!PHAL_configureAPB1Clock(config->apb1_clock_target_hz))   << 1;
    ret_code |= (!PHAL_configureAPB2Clock(config->apb2_clock_target_hz))   << 2;

    return ret_code;
}
    

bool PHAL_configurePLLVCO(PLLSrc_t pll_source, uint32_t vco_output_rate_target_hz)
{
    // Valid range for PLL output is 64Mhx <= PLL_CLK <= 344Mhz
    vco_output_rate_target_hz = vco_output_rate_target_hz > (uint32_t) 344e6 ? (uint32_t) 344e6 : vco_output_rate_target_hz;
    vco_output_rate_target_hz = vco_output_rate_target_hz < (uint32_t) 64e6 ? (uint32_t) 64e6 : vco_output_rate_target_hz;
    
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
    for (; pll_input_divisor <= 8; pll_input_divisor ++)
    {
        // VCO input clock is only allowed between 4MHz and 16MHz
        uint32_t pll_vco_in_rate = pll_input_f_hz / pll_input_divisor;
        if(pll_vco_in_rate < 4000000 || pll_vco_in_rate > 16000000)
        {
            continue;
        }

        for (; pll_output_multiplier <= 86; pll_output_multiplier ++)
        {
            if ((pll_input_f_hz / pll_input_divisor) * pll_output_multiplier == vco_output_rate_target_hz)
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
    return true;
}

bool PHAL_configurePLLSystemClock(uint32_t system_clock_target_hz)
{
    if (system_clock_target_hz > 80000000)
    {
        system_clock_target_hz = 80000000;
    }

    // Valid number for PLLR divisor are 2,4,6,8 (2 bit encoded)
    uint8_t pll_r_divisor = PLLClockRateHz / system_clock_target_hz;
    if(pll_r_divisor == 0 || pll_r_divisor % 2 != 0 || pll_r_divisor > 8)
    {
        return false;
    }

    // Enable PLLR and set the PLLR divisor
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR_Msk;
    RCC->PLLCFGR |= (((pll_r_divisor / 2) - 1) << RCC_PLLCFGR_PLLR_Pos) & RCC_PLLCFGR_PLLR_Msk;


    // Flash latency adjustment, see ST RM0395 Table 9
    uint32_t flash_acr_temp = FLASH->ACR;
    flash_acr_temp &= !FLASH_ACR_LATENCY_Msk;
    if(system_clock_target_hz >= 80000000)
        flash_acr_temp |= FLASH_ACR_LATENCY_4WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz >= 64000000)
        flash_acr_temp |= FLASH_ACR_LATENCY_3WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz >= 48000000)
        flash_acr_temp |= FLASH_ACR_LATENCY_2WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz >= 32000000)
        flash_acr_temp |= FLASH_ACR_LATENCY_1WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz >= 16000000)
        flash_acr_temp |= FLASH_ACR_LATENCY_0WS << FLASH_ACR_LATENCY_Pos;

    FLASH->ACR = flash_acr_temp;

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS_PLL != RCC_CFGR_SWS_PLL))
        ; // Wait for PLL to be the new system clock

    SystemCoreClockUpdate();
    return true;
}

bool PHAL_configureAHBClock(uint32_t ahb_clock_target_hz)
{
    uint32_t desired_psc = SystemCoreClock / ahb_clock_target_hz;
    uint32_t sys_clk_div;
    switch(desired_psc) // ST RM0394 pg. 198
    {
        case 1:
            sys_clk_div = RCC_CFGR_HPRE_DIV1;
            break;
        case 2:
            sys_clk_div = RCC_CFGR_HPRE_DIV2;
            break;
        case 4:
            sys_clk_div = RCC_CFGR_HPRE_DIV4;
            break;
        case 8:
            sys_clk_div = RCC_CFGR_HPRE_DIV8;
            break;
        case 16:
            sys_clk_div = RCC_CFGR_HPRE_DIV16;
            break;
        case 64:
            sys_clk_div = RCC_CFGR_HPRE_DIV64;
            break;
        case 128:
            sys_clk_div = RCC_CFGR_HPRE_DIV128;
            break;
        case 256:
            sys_clk_div = RCC_CFGR_HPRE_DIV256;
            break;
        case 512:
            sys_clk_div = RCC_CFGR_HPRE_DIV512;
            break;
        default:
            return false; // Invalid prescalar value
    }

    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= !RCC_CFGR_HPRE_Msk;
    rcc_cfgr_temp |= (sys_clk_div  << RCC_CFGR_HPRE_Pos) & RCC_CFGR_HPRE_Msk;

    RCC->CFGR = rcc_cfgr_temp;

    AHBClockRateHz = ahb_clock_target_hz;
    return true;
}

bool PHAL_configureAPB1Clock(uint32_t apb1_clock_target_hz)
{
    uint32_t desired_psc = AHBClockRateHz / apb1_clock_target_hz;
    uint32_t ahb_clk_div;
    switch(desired_psc) // ST RM0394 pg. 198
    {
        case 1:
            ahb_clk_div = RCC_CFGR_PPRE1_DIV1;
            break;
        case 2:
            ahb_clk_div = RCC_CFGR_PPRE1_DIV2;
            break;
        case 4:
            ahb_clk_div = RCC_CFGR_PPRE1_DIV4;
            break;
        case 8:
            ahb_clk_div = RCC_CFGR_PPRE1_DIV8;
            break;
        case 16:
            ahb_clk_div = RCC_CFGR_PPRE1_DIV16;
            break;
        default:
            return false; // Invalid prescalar value
    }
    
    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= !RCC_CFGR_PPRE1_Msk;
    rcc_cfgr_temp |= (ahb_clk_div << RCC_CFGR_PPRE1_Pos) & RCC_CFGR_PPRE1_Msk;

    RCC->CFGR = rcc_cfgr_temp;

    APB1ClockRateHz = apb1_clock_target_hz;
    return true;
}

bool PHAL_configureAPB2Clock(uint32_t apb2_clock_target_hz)
{
    uint32_t desired_psc = AHBClockRateHz / apb2_clock_target_hz;
    uint32_t ahb_clk_div;
    switch(desired_psc) // ST RM0394 pg. 198
    {
        case 1:
            ahb_clk_div = RCC_CFGR_PPRE2_DIV1;
            break;
        case 2:
            ahb_clk_div = RCC_CFGR_PPRE2_DIV2;
            break;
        case 4:
            ahb_clk_div = RCC_CFGR_PPRE2_DIV4;
            break;
        case 8:
            ahb_clk_div = RCC_CFGR_PPRE2_DIV8;
            break;
        case 16:
            ahb_clk_div = RCC_CFGR_PPRE2_DIV16;
            break;
        default:
            return false; // Invalid prescalar value
    }
    
    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= !RCC_CFGR_PPRE2_Msk;
    rcc_cfgr_temp |= (ahb_clk_div << RCC_CFGR_PPRE2_Pos) & RCC_CFGR_PPRE2_Msk;

    RCC->CFGR = rcc_cfgr_temp;

    APB1ClockRateHz = apb2_clock_target_hz;
    return true;
}