/**
 * @file rcc.c
 * @brief G4 RCC public API implementation.
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "common/phal_G4/rcc/rcc.h"

#include "common/phal_G4/rcc/rcc_priv.h"


static uint32_t g_system_clock_hz = HSI_CLOCK_RATE_HZ;
static uint32_t g_ahb_clock_hz    = HSI_CLOCK_RATE_HZ;
static uint32_t g_apb1_clock_hz   = HSI_CLOCK_RATE_HZ;
static uint32_t g_apb2_clock_hz   = HSI_CLOCK_RATE_HZ;


static void rcc_initFromOscillator(bool use_hse, bool use_pll) {
    } else {
    }

        } else {
            config->system_clock_target_hz = HSI_CLOCK_RATE_HZ;
        }
    }

    }

    g_ahb_clock_hz  = g_system_clock_hz;
    g_apb1_clock_hz = g_system_clock_hz;
    g_apb2_clock_hz = g_system_clock_hz;

}

void PHAL_RCC_init(PHAL_RCC_Mode_t mode) {
    switch (mode) {
        case PHAL_RCC_HSI_16MHZ:  rcc_initFromOscillator(false, false); break;
        case PHAL_RCC_HSI_170MHZ: rcc_initFromOscillator(false, true);  break;
        case PHAL_RCC_HSE_16MHZ:  rcc_initFromOscillator(true, false);  break;
        case PHAL_RCC_HSE_170MHZ: rcc_initFromOscillator(true, true);   break;
        default:
            __builtin_trap();
    }
}

uint32_t PHAL_RCC_getSystemClockHz(void) {
    return g_system_clock_hz;
}

uint32_t PHAL_RCC_getAHBClockHz(void) {
    return g_ahb_clock_hz;
}

uint32_t PHAL_RCC_getAPB1ClockHz(void) {
    return g_apb1_clock_hz;
}

uint32_t PHAL_RCC_getAPB2ClockHz(void) {
    return g_apb2_clock_hz;
}