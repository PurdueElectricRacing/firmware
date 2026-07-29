/**
 * @file rcc_priv.c
 * @brief G4 RCC private/register level implementation
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "common/phal_G4/rcc/rcc_priv.h"

void PHAL_RCC_priv_enableHSI(void) {
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {
        __asm__("nop");
    }
}

void PHAL_RCC_priv_disableHSI(void) {
    RCC->CR &= ~RCC_CR_HSION;
    while (RCC->CR & RCC_CR_HSION) {
        __asm__("nop");
    }
}

void PHAL_RCC_priv_enableHSE(void) {
    RCC->CR |= (RCC_CR_HSEON | RCC_CR_HSEBYP);
    while (!(RCC->CR & RCC_CR_HSERDY)) {
        __asm__("nop");
    }
}

void PHAL_RCC_priv_setFlashLatency(uint32_t latency_ws) {
    uint32_t acr = FLASH->ACR;
    acr &= ~FLASH_ACR_LATENCY_Msk;
    acr |= (latency_ws << FLASH_ACR_LATENCY_Pos);
    FLASH->ACR = acr;
    __DSB();
}

void PHAL_RCC_priv_configurePLL(RCC_PRIV_PLLSource_t src) {
    // PLL must be off before its configuration fields can be changed.
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) {
        __asm__("nop");
    }

    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC_Msk | RCC_PLLCFGR_PLLM_Msk | RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLR_Msk);
    RCC->PLLCFGR |= (uint32_t)src
        | ((RCC_PRIV_PLL_M - 1U) << RCC_PLLCFGR_PLLM_Pos)  // PLLM field is (divisor - 1)
        | (RCC_PRIV_PLL_N << RCC_PLLCFGR_PLLN_Pos)         // PLLN field is the raw multiplier
        | (((RCC_PRIV_PLL_R / 2U) - 1U) << RCC_PLLCFGR_PLLR_Pos); // PLLR field is (divisor/2 - 1)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN; // enable the R output (-> SYSCLK). P/Q outputs unused

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)){
        __asm__("nop");
    }
}

void PHAL_RCC_priv_enableBoostMode(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

    // AHB must be temporarily halved while boost mode is
    // enabled, then restored once R1MODE has taken effect
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) | RCC_CFGR_HPRE_DIV2;
    __DSB();

    PWR->CR5 &= ~PWR_CR5_R1MODE;
    // let the regulator settle
    for (volatile uint32_t i = 0; i < 100U; i++) {
        __asm__("nop");
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) | RCC_CFGR_HPRE_DIV1;
    __DSB();
}

void PHAL_RCC_priv_switchSysclkToHSI(void) {
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI){
        __asm__("nop");
    }
}

void PHAL_RCC_priv_switchSysclkToHSE(void) {
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSE){
        __asm__("nop");
    }
}

void PHAL_RCC_priv_switchSysclkToPLL(void) {
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL){
        __asm__("nop");
    }
}

void PHAL_RCC_priv_setBusPrescalersToDiv1(void) {
    uint32_t cfgr = RCC->CFGR;
    cfgr &= ~(RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk);
    cfgr |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1;
    RCC->CFGR = cfgr;
}