
#include "common/common_defs/common_defs.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"

/**
 * @brief Configures timers for quadrature encoder operation
 *        Assumes channels A and B are on CH1 and CH2
 *
 * @param ws    Initialized wheel speed handle
 * @return      True on success, False on fail
 */
void wheelSpeedsInit(void)
{
    // Configure timer 2
    // Targeting an interrupt every 1 ms
    if (RCC->CFGR & RCC_CFGR_PPRE1_2) freq *= 2; // RM0394 pg 188 (timer clock doubles if apb1 prescaler != 0)
    #if (defined(STM32F407xx) || defined(STM32F732xx))
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    #else
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
    #endif
    TIM3->PSC = (freq / 1000000) - 1;
    TIM3->ARR = ARR_SET;
    TIM3->CR1 &= ~(TIM_CR1_DIR);
    TIM3->DIER |= TIM_DIER_UIE;
    /* Connect pins to TIM3 AF2 */
    #define GPIO_AF_TIM2 2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_TIM2 );  GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_TIM2 );  GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_TIM2 );  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_TIM2 );
}
#if 0
/**
 * @brief Configures a timer for quadrature encoder operation
 *        assumes the corresponding timer clock has already
 *        been enabled. The inversion is done by inverting
 *        the polarity on CC1.
 *        Note: currently setup for use with TIM2 and TIM5
 *        Warning: DOES NOT ENABLE
 *
 * @param tim The timer to configure
 */
static void configEncoder(TIM_TypeDef* tim, bool inverted)
{
    tim->CR1   &= ~TIM_CR1_CEN;                     // Disable counter during configuration
    tim->PSC   =  0;                                // Do not scale the counter clock
    tim->CNT   =  0;                                // Reset the counter value
    tim->ARR   =  0xFFFFFFFF;                       // Set auto reload to allow for maximum counter value
    tim->SMCR  |= (0b0011 << TIM_SMCR_SMS_Pos);     // Encoder mode 3 - counts up/down on TI1FP1 and TI2FP2
    tim->CCMR1 |= (0b01 << TIM_CCMR1_CC1S_Pos) |
                  (0b01 << TIM_CCMR1_CC2S_Pos);     // Map IC1/2 on TI1/2
    //tim->CCMR1 = 0b01;
    //tim->CCMR2 = 0b01;
    tim->CCER  &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P); // Clear any inversion
    if (inverted) tim->CCER |= TIM_CCER_CC1P;       // Invert CC1 if desired
    // TODO: configure input filter if necessary
    //tim->CCER = 0b00;
}

/**
 * @brief Updates radians per second calculation
 *
 */
void wheelSpeedsPeriodic()
{
    uint32_t cnt_l, cnt_r, ms;
    float rad_s_cnt;
    if (!_ws) return;               // Only run if initialized

    cnt_l = _ws->l->tim->CNT;        // Record counter values
    cnt_r = _ws->r->tim->CNT;
    ms = sched.os_ticks;            // Record measurement time

    // rad / s = (count / ms) * (1000 ms / s) * (1 / counts_per_rev) * (2PI rad / rev)
    rad_s_cnt = 2.0 * PI / WHEEL_COUNTS_PER_REV / (ms - _ws->last_update_ms) * 1000.0f;
    _ws->l->rad_s = (cnt_l - _ws->l->last_count) * rad_s_cnt;
    _ws->r->rad_s = (cnt_r - _ws->r->last_count) * rad_s_cnt;

    // Update last state values
    _ws->last_update_ms = ms;
    _ws->l->last_count = cnt_l;
    _ws->r->last_count = cnt_r;
}
#endif
