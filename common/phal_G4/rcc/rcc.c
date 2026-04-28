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

#include "common/phal_G4/rcc/rcc.h"

/* Globals for Clock Rates */
uint32_t APB1ClockRateHz;
uint32_t APB2ClockRateHz;
uint32_t AHBClockRateHz;
uint32_t PLLClockRateHz;

uint8_t PHAL_configureClockRates(ClockRateConfig_t* config) {
    // Nonzero Bit Encoded Error Code Indicates Error
    uint8_t ret_code = 0;

    /* Configure System Clock */
    if (config->clock_source == CLOCK_SOURCE_HSE) {
        ret_code |= (!PHAL_configureHSESystemClock()) << RCC_ERROR_HSE_INIT;
    } else {
        ret_code |= (!PHAL_configureHSISystemClock()) << RCC_ERROR_HSI_INIT;
    }

    /* Configure PLL */
    if (config->use_pll) {
        ret_code |= (!PHAL_configurePLLVCO(config->pll_src, config->vco_output_rate_target_hz)) << RCC_ERROR_PLLVCO_INIT;
        ret_code |= (!PHAL_configurePLLSystemClock(config->system_clock_target_hz)) << RCC_ERROR_PLLSYS_INIT;
    }
    /* No PLL */
    else {
        if (config->clock_source == CLOCK_SOURCE_HSE) {
            config->system_clock_target_hz = HSE_CLOCK_RATE_HZ;
        } else {
            config->system_clock_target_hz = HSI_CLOCK_RATE_HZ;
        }
    }

    ret_code |= (!PHAL_configureAHBClock(config->ahb_clock_target_hz)) << RCC_ERROR_AHB_INIT; // Configure AHB Clock
    ret_code |= (!PHAL_configureAPB1Clock(config->apb1_clock_target_hz)) << RCC_ERROR_APB1_INIT; // Configure APB1 Clock
    ret_code |= (!PHAL_configureAPB2Clock(config->apb2_clock_target_hz)) << RCC_ERROR_APB2_INIT; // Configure APB2 Clock

    return ret_code;
}

bool PHAL_configurePLLVCO(PLLSrc_t pll_source, uint32_t vco_output_rate_target_hz) {
    // Ensure range for PLL output is 100Mhz <= PLL_CLK <= 432Mhz
    vco_output_rate_target_hz = vco_output_rate_target_hz > RCC_MAX_VCO_RATE_HZ ? RCC_MAX_VCO_RATE_HZ : vco_output_rate_target_hz;
    vco_output_rate_target_hz = vco_output_rate_target_hz < RCC_MIN_VCO_RATE_HZ ? RCC_MIN_VCO_RATE_HZ : vco_output_rate_target_hz;

    // Turn off and wait for PLL to disable
    RCC->CR &= ~RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY))
        ; // Wait for PLL to turn off
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC_Msk | RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLM_Msk); // Clear any PLL source, N, and M configuration

    // Select the PLL source and determine a desired input clock rate
    uint32_t pll_input_f_hz;
    switch (pll_source) {
        // 16 MHz HSI
        case PLL_SRC_HSI16:
            RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI; // Select HSI source
            while (!(RCC->CR & RCC_CR_HSIRDY)) // Wait for HSI to enable
                ;
            pll_input_f_hz = HSI_CLOCK_RATE_HZ; // Set local for PLL input clock rate
            break;
        case PLL_SRC_HSE:
            RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
            while (!(RCC->CR & RCC_CR_HSERDY))
                ;
            pll_input_f_hz = HSE_CLOCK_RATE_HZ;
            break;
        default:
            return false; // Invalid PLL source
    }

    /* Search for a possible PLL configuration */
    uint8_t pll_input_divisor     = RCC_MIN_PLL_INPUT_DIVISOR; // PLLM
    uint8_t pll_output_multiplier = RCC_MIN_PLL_OUTPUT_MULTIPLIER; // PLLN
    bool valid_rate               = false;
    for (; pll_input_divisor < RCC_MAX_PLL_INPUT_DIVISOR; pll_input_divisor++) // PLLM must be 1 to 16 
    {
        // VCO input frequency = PLL input clock frequency / PLLM with 1 <= PLLM <= 16
        uint32_t pll_vco_in_rate = pll_input_f_hz / pll_input_divisor;
        if (pll_vco_in_rate < 2'660'000 || pll_vco_in_rate > 16'000'000) // VCO input rate must be between 2.66 MHz and 16 MHz
        {
            continue;
        }
        pll_output_multiplier = RCC_MIN_PLL_OUTPUT_MULTIPLIER; // Reset PLLN for each new PLLM iteration
        // VCO output frequency = VCO input * PLLN
        for (; pll_output_multiplier <= RCC_MAX_PLL_OUTPUT_MULTIPLIER; pll_output_multiplier++) // PLLN must be 8 to 127
        {
            uint64_t calculated_vco_output = ((uint64_t)pll_input_f_hz * (uint64_t)pll_output_multiplier) / (uint64_t)pll_input_divisor;
            if (calculated_vco_output == vco_output_rate_target_hz) {
                valid_rate = true;
                break;
            }
        }
        if (valid_rate)
            break;
    }

    if (!valid_rate)
        return false; // Unable to find a valid clock rate!

    RCC->PLLCFGR |= ((pll_input_divisor - 1) << RCC_PLLCFGR_PLLM_Pos) & RCC_PLLCFGR_PLLM_Msk; // Set PLLM
    RCC->PLLCFGR |= ((pll_output_multiplier) << RCC_PLLCFGR_PLLN_Pos) & RCC_PLLCFGR_PLLN_Msk; // Set PLLN

    // Update global variable used to reference the PLL
    PLLClockRateHz = ((pll_input_f_hz / pll_input_divisor) * pll_output_multiplier);

    SystemCoreClockUpdate(); // Must be called each time the core clock HCLK changes
    return true;
}

bool PHAL_configurePLLSystemClock(uint32_t system_clock_target_hz) {
    // Ensure system clock target is valid (Must be under 170 MHz)
    if (system_clock_target_hz > RCC_MAX_SYSCLK_TARGET_HZ) {
        system_clock_target_hz = RCC_MAX_SYSCLK_TARGET_HZ;
    }

    // Valid number for PLLP divisor are 2,4,6,8 (2 bit encoded)
    uint8_t pll_p_divisor = PLLClockRateHz / system_clock_target_hz;
    if (pll_p_divisor == 0 || pll_p_divisor % 2 != 0 || pll_p_divisor > 8) {
        return false;
    }

    uint8_t pll_q_divisor = PLLClockRateHz / 48000000;
    if (pll_q_divisor <= 1 || pll_q_divisor > 15) {
        return false;
    }

    // 1. Clear the fields
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLPDIV_Msk | RCC_PLLCFGR_PLLR_Msk | RCC_PLLCFGR_PLLQ_Msk);

    // 2. Set PLLR to Divide by 2 (00)
    // Clearing mask sets all to divide by 0, but we're being explicit to be clear
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLR_Pos); 
    // PLLQ to divide by 2
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLQ_Pos);
    // PLLPDIV to divide by 2
    RCC->PLLCFGR |= (2 << RCC_PLLCFGR_PLLPDIV_Pos);

    // Enable output for P, Q and R
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN | RCC_PLLCFGR_PLLQEN | RCC_PLLCFGR_PLLPEN;

    __DSB(); // Wait for explicit memory accesses to finish

    // Enable power interface clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

    // RCC control training -- https://www.st.com/resource/en/product_training/STM32G4-System-Reset_and_clock_control_RCC.pdf

    // Ranges are defined in Table 50 of STM32G4 reference manual (RM0440) (Pg. 280)
    // Voltage scaling sequence is defined in Section 6.1.5 Dynamic voltage scaling management

    // Range 1 boost mode (150 MHz < SYSCLK <= 170 MHz) 
    if (system_clock_target_hz > 150'000'000) {
        //  1. The system clock must be divided by 2 using the AHB prescaler before switching to a higher system frequency
        PHAL_configureAHBClock(system_clock_target_hz / 2);         

        // 2. Clear the R1MODE bit is in the PWR_CR5 register (enables boost mode)
        PWR->CR5 &= ~PWR_CR5_R1MODE;
    }

    RCC->CR |= RCC_CR_PLLON; // Enable PLL
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ; // Wait for PLL to turn on
    __DSB();

    // 3. Adjust the number of wait states according to the new frequency target in range1 boost
    uint32_t flash_acr_temp = FLASH->ACR;
    flash_acr_temp &= ~(FLASH_ACR_LATENCY_Msk);

    if (system_clock_target_hz > 136'000'000)      // 136-170 MHz: 4 WS
        flash_acr_temp |= FLASH_ACR_LATENCY_4WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz > 102'000'000) // 102-136 MHz: 3 WS
        flash_acr_temp |= FLASH_ACR_LATENCY_3WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz > 68'000'000)  // 68-102 MHz:  2 WS
        flash_acr_temp |= FLASH_ACR_LATENCY_2WS << FLASH_ACR_LATENCY_Pos;
    else if (system_clock_target_hz > 34'000'000)  // 34-68 MHz:   1 WS
        flash_acr_temp |= FLASH_ACR_LATENCY_1WS << FLASH_ACR_LATENCY_Pos;
    else                                         // <= 34 MHz:   0 WS
        flash_acr_temp |= FLASH_ACR_LATENCY_0WS << FLASH_ACR_LATENCY_Pos;
    FLASH->ACR = flash_acr_temp;

    __DSB(); // Wait for explicit memory accesses to finish
    RCC->CFGR |= RCC_CFGR_SW_PLL; // Set system clock switch register to PLL
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL) // Wait for PLL to be the new system clock
        ;
    __DSB(); // Wait for explicit memory accesses to finish

    SystemCoreClockUpdate(); // Must be called each time the core clock HCLK changes
    return true;
}

bool PHAL_configureHSISystemClock() {
    // Turn on and wait for HSI to enable
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY))
        ;

    //Flash latency adjustment, see ST RM 0090 Pg. 80
    uint32_t flash_acr_temp = FLASH->ACR;
    flash_acr_temp &= ~(FLASH_ACR_LATENCY_Msk);
    flash_acr_temp |= FLASH_ACR_LATENCY_0WS << FLASH_ACR_LATENCY_Pos;
    FLASH->ACR = flash_acr_temp;

    __DSB(); // Wait for explicit memory accesses to finish
    RCC->CFGR |= RCC_CFGR_SW_HSI; // Set system clock switch register to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS_HSI) != RCC_CFGR_SWS_HSI) // Wait until the system clock switch register indicates that HSI is selected
        ;
    __DSB(); // Wait for explicit memory accesses to finish

    SystemCoreClockUpdate(); // Must be called each time the core clock HCLK changes
    return true; // Return true upon completion
}

bool PHAL_configureHSESystemClock() {
    /* Turn on and wait for HSE to enable */
    RCC->CR |= (RCC_CR_HSEON | RCC_CR_HSEBYP);
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    // Flash latency adjustment, see ST RM 0090 Pg. 80
    uint32_t flash_acr_temp = FLASH->ACR;
    flash_acr_temp &= ~(FLASH_ACR_LATENCY_Msk);
    flash_acr_temp |= FLASH_ACR_LATENCY_0WS << FLASH_ACR_LATENCY_Pos;
    FLASH->ACR = flash_acr_temp;

    __DSB(); // Wait for explicit memory accesses to finish
    /*
        From G4 Reference Manual 7.4.3 (RCC_CFGR)
        SWS = System clock switch status (hardware configured)
        SW = System clock switch (user/sw configured)

        SW[1:0] = 10: HSE selected as system clock
    */
    RCC->CFGR |= (0b10 & RCC_CFGR_SW_Msk); // ! the order matters (idk why)
    RCC->CFGR &= ~(0b01 & RCC_CFGR_SW_Msk);
    
    // Wait until the system clock switch status register indicates that HSE is selected
    while ((RCC->CFGR & RCC_CFGR_SWS_HSE) != RCC_CFGR_SWS_HSE)
        ;
    __DSB(); // Wait for explicit memory accesses to finish

    /* Turn off HSI */
    __DSB();
    RCC->CR &= ~(RCC_CR_HSION);
    while ((RCC->CR & RCC_CR_HSION))
        ;
    __DSB();

    SystemCoreClockUpdate(); // Must be called each time the core clock HCLK changes

    return true;
}

bool PHAL_configureAHBClock(uint32_t ahb_clock_target_hz) {
    // Map a required prescaler to achieve the target AHB clock speed from the SystemCoreClock
    // HCLK has a maximum frequency of 144MHz VOS = 0 and 168 MHz VOS = 1 RM 0090 pg. 80
    uint32_t desired_psc = SystemCoreClock / ahb_clock_target_hz;
    uint32_t sys_clk_div;

    // CPU Clock Prescalar (1,2,4,8,16,64,128,256,512 allowed)
    switch (desired_psc) // ST RM0090 pg. 230
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
            return false; // Return if invalid prescalar value
    }

    // Modify the RCC->CFGR HPRE bits to select prescaler
    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= ~(RCC_CFGR_HPRE_Msk);
    rcc_cfgr_temp |= (sys_clk_div);
    RCC->CFGR = rcc_cfgr_temp;

    AHBClockRateHz = ahb_clock_target_hz; // Set global for AHB Clock Rate
    return true;
}

// Low speed peripheral bus (Max speed 42 MHz)
bool PHAL_configureAPB1Clock(uint32_t apb1_clock_target_hz) {
    // Map a required prescaler to achieve the target APB1 clock speed from AHB
    uint32_t desired_psc = AHBClockRateHz / apb1_clock_target_hz;
    uint32_t ahb_clk_div;

    // APB1 Prescaler (1,2,4,8,16 allowed)
    switch (desired_psc) // ST RM0090 pg. 229
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
            return false; // Return if invalid prescaler value
    }

    // Modify the RCC->CFGR PPRE1 bits to select prescaler
    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= ~(RCC_CFGR_PPRE1_Msk);
    rcc_cfgr_temp |= (ahb_clk_div);
    RCC->CFGR = rcc_cfgr_temp;

    APB1ClockRateHz = apb1_clock_target_hz; // Set global for APB1
    return true;
}

// High speed peripheral bus (Max speed 84 MHz)
bool PHAL_configureAPB2Clock(uint32_t apb2_clock_target_hz) {
    // Map a required prescaler to achieve the target APB2 clock speed from AHB
    uint32_t desired_psc = AHBClockRateHz / apb2_clock_target_hz;
    uint32_t ahb_clk_div;

    // APB1 Prescaler (1,2,4,8,16 allowed)
    switch (desired_psc) // ST RM0090 pg. 229
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
            return false; // Invalid prescaler value
    }

    // Modify the RCC->CFGR PPRE2 bits to select prescaler
    uint32_t rcc_cfgr_temp = RCC->CFGR;
    rcc_cfgr_temp &= ~(RCC_CFGR_PPRE2_Msk);
    rcc_cfgr_temp |= ahb_clk_div;
    RCC->CFGR = rcc_cfgr_temp;

    APB2ClockRateHz = apb2_clock_target_hz; // Set global for APB2
    return true;
}
