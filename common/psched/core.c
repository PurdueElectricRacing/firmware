#include "common/psched/psched_core.h"

// Local functions
static void disable_irq();
static void enable_irq();

// @funcname: pschedInit()
//
// @brief: Initializes the RTOS
//
// @param: freq: Frequency of MCU in Hz
void pschedInit(uint32_t freq)
{
    /*
        Note: This system uses timer 2 and the watchdog peripheral
        If you want/need to use this timer, it is on you to edit the configuration
        to use a different timer.
        DO NOT ATTEMPT TO CONFIGURE THIS TIMER IN CUBE!
        The configuration for this timer is done manually, right here.
        The watchdog will trigger a reset if all loops take longer than 10 ms to return

        Also, functions need to return to work properly. The scheduler works on a timer interrupt.
        If the functions called in the timer interrupt, you're going to have a stack overflow.

        Frequencies are given in OS ticks.
    */

    // Configure timer 2
    // Targeting a base time unit of 1μs
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    TIM2->PSC = (freq / 1000000) - 1;
    TIM2->ARR = 1000;                       // Will get changed at first run of scheduler
    TIM2->CR1 &= ~(TIM_CR1_DIR);
    TIM2->DIER |= TIM_DIER_UIE;

    // Default all values
    sched.of = freq;
}

// @funcname: pschedStart()
//
// @brief: Starts tasks. Will never return
void pschedStart()
{
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM2_IRQn;
    IWDG->KR  =  0xCCCC;     
    IWDG->KR  =  0x5555;
    IWDG->PR  |= 2;
    IWDG->RLR =  20;

    while ((IWDG->SR & 0b111) != 0);

    IWDG->KR = 0xAAAA;
}

// @funcname: TIM2_IRQHandler()
//
// @brief: Timer 2 IRQ. Handles scheduling of next
//         task, as well as context switching
void TIM2_IRQHandler()
{
	TIM2->SR &= ~TIM_SR_UIF;

}

static void disable_irq() {
    asm("cpsid i");
}

static void enable_irq() {
    asm("cpsie i");
}