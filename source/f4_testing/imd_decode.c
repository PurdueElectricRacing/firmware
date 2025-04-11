
#include "f4_testing.h"

// Guard so cmake doesn't compile all tests
#if (F4_TESTING_CHOSEN == TEST_IMD_DECODE)

// decode IMD status output PWM signal that changes both frequency and pwm

#include "common/common_defs/common_defs.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/log/log.h"

#define IMD_HS_PWM_GPIO_Port (GPIOB)
#define IMD_HS_PWM_Pin (3)
#define IMD_HS_PWM_AF  (1) // TIM2_CH2
#define IMD_LS_PWM_GPIO_Port (GPIOA)
#define IMD_LS_PWM_Pin (15)
#define IMD_LS_PWM_AF (1) // TIM2_CH1
#define IMD_STATUS_GPIO_Port (GPIOB)
#define IMD_STATUS_Pin (4)
#define IMD_STATUS_AF  (2) // TIM3_CH1

#define IMD_LS_PWM1_GPIO_Port (GPIOA)
#define IMD_LS_PWM1_Pin (0)
#define IMD_LS_PWM1_AF (1) // PA0 TIM2_CH1_ETR

#define IMD_LS_PWM2_GPIO_Port (GPIOA)
#define IMD_LS_PWM2_Pin (1)
#define IMD_LS_PWM2_AF (1) // PA1 TIM2_CH2

void HardFault_Handler();
void ledblink();
void imdDecodeInit(void);
void imdDecodePeriodic(void);

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // F407VGT disco LEDs
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED),

    GPIO_INIT_USART2TX_PA2,
    GPIO_INIT_USART2RX_PA3,

    GPIO_INIT_AF(IMD_LS_PWM1_GPIO_Port, IMD_LS_PWM1_Pin, IMD_LS_PWM1_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_AF(IMD_LS_PWM2_GPIO_Port, IMD_LS_PWM2_Pin, IMD_LS_PWM2_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_UP),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

volatile uint32_t tick_ms; // Systick 1ms counter

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source               =CLOCK_SOURCE_HSI,
    .use_pll                    =false,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (4)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (4)),
};

dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t usart_config = {
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .periph      = USART2,
   .wake_addr = false,
   .usart_active_num = USART2_ACTIVE_IDX,
   .tx_dma_cfg = &usart_tx_dma_config,
   .rx_dma_cfg = &usart_rx_dma_config
};
DEBUG_PRINTF_USART_DEFINE(&usart_config) // use LTE uart lmao

volatile uint16_t period = 0;
volatile uint16_t duty = 0;
volatile float frequency = 0;
volatile float duty_cycle = 0;
volatile uint64_t overflow_counts = 0;
volatile float resistance = 0;

int main()
{
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    //SysTick_Config(SystemCoreClock / 1000);
    //NVIC_EnableIRQ(SysTick_IRQn);

    if(!PHAL_initUSART(&usart_config, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    log_yellow("PER PER PER\n");
    imdDecodeInit();

    schedInit(APB1ClockRateHz);
    taskCreate(ledblink, 500);
    //taskCreate(imdDecodePeriodic, 5);
    schedStart();

    return 0;
}

void ledblink()
{
    PHAL_toggleGPIO(GPIOD, 13);
}

void imdDecodeInit(void)
{
    // Configure timer 2
    // Targeting an interrupt every 1 ms
    //if (RCC->CFGR & RCC_CFGR_PPRE1_2) freq *= 2; // RM0394 pg 188 (timer clock doubles if apb1 prescaler != 0)
    #if (defined(STM32F407xx) || defined(STM32F732xx))
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    #else
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    #endif

    TIM2->CR1   &= ~(TIM_CR1_CEN);                     // Disable counter during configuration
    TIM2->PSC   = 0;                                // Do not scale the counter clock
    TIM2->CNT   = 0;                                // Reset the counter value
    TIM2->ARR   = 0xffffff - 1;                       // Set auto reload to allow for maximum counter value

    // Disable capture from the counter for channels 1 and 2
    TIM2->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);

    /** configure channel 1 for active rising input **/
    // select channel 1 as the active input
    TIM2->CCMR1 &= ~TIM_CCMR1_CC1S_Msk; // clear bits
    //TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;    // 01 - maps ch1 to TI1
	/*Set CH1 to input capture*/
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_1;

    // setup input capture filtering
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1F_Msk; // clear bits
    //TIM2->CCMR1 |= (0b0011 << 4);
    // select the edge of transition
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // 00 - active rising
    //TIM2->CCER = TIM_CCER_CC1E;
    // set input Pre-Scaler
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1PSC_Msk; // clear bits

#if 1
    /** configure channel 2 for active falling input **/
    // select channel 2 as the active input
    TIM2->CCMR1 &= ~TIM_CCMR1_CC2S_Msk; // clear bits
    TIM2->CCMR1 |= TIM_CCMR1_CC2S_1;    // 10 - maps channel 2 to TI1

    // setup input capture filtering
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2F_Msk; // clear bits
    // select the edge of transition
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // clear bits
    TIM2->CCER |= TIM_CCER_CC2P; // 01 - active falling

    // set input Pre-Scaler
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2PSC_Msk; // clear bits

    // select valid trigger input
    TIM2->SMCR &= ~TIM_SMCR_TS_Msk; // clear bits
    TIM2->SMCR |= (TIM_SMCR_TS_0 | TIM_SMCR_TS_2); // 101 - Filtered Timer Input 1 (TI1FP1)

    // configure the slave mode controller in reset mode
    TIM2->SMCR &= ~TIM_SMCR_SMS_Msk; // clear bits
    TIM2->SMCR |= TIM_SMCR_SMS_2; // 100 - rising edge resets the counter
#endif

    /** turn stuff on and configure DMA requests **/
    // Enable capture from the counter for channel 1/2
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    // enable interrupt on channel 1/2
    TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;

    // enable counter
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC_SetPriority(TIM2_IRQn, 0); // highest priority
    NVIC_EnableIRQ(TIM2_IRQn);
}

void imdDecodePeriodic(void)
{
    frequency = (float)(APB1ClockRateHz * 2) / (period + 1); // psc * 2
    duty_cycle = (float)((duty + 1) * 100) / (period);

    // Condition “Normal” and “Undervoltage detected”
    if (1) // frequency >= 1000 && frequency <= 2000
    {
        if (duty_cycle < 5)
            resistance = 50000; // 50M
        else if (duty_cycle > 95)
            resistance = 1200; // 1200K
        else
            resistance = ((90 * 1200) / (duty_cycle - 5)) - 1200;
    }

    // Speed start measurement
    if (1) // Condition “SST” (30 Hz)
    {
        if (duty_cycle > 5 && duty_cycle < 10)
            ;// good
        else if (duty_cycle > 90 && duty_cycle < 95)
            ; //bad
    }

    if ()
    // Condition “Device error” and “Kl.31 fault” (40 Hz; 50 Hz;)

    //SEND_IMD_STATUS_RAW(period, duty);
    //debug_printf("period: %d duty: %d\n", period, duty);
}

void TIM2_IRQHandler(void)
{
 /* check for timer overflow */
 if ((TIM2->SR & TIM_SR_UIF) != 0)
 {
  // clear UIF flag to prevent re-entry
  TIM2->SR &= ~TIM_SR_UIF;
  overflow_counts++;
 }

 /* check for rising edge interrupt on channel 1*/
 if ((TIM2->SR & TIM_SR_CC1IF) != 0)
 {
  // read CCR1 to clear CC1IF flag
  // compute period
  period = TIM2->CCR1 + ((1 + TIM2->ARR) * overflow_counts);
  // reset overflows since timer restarts on rising edge
  overflow_counts = 0;
 }

 /* check for falling edge interrupt */
 if ((TIM2->SR & TIM_SR_CC2IF) != 0)
 {
  // read CCR2 to clear CC2IF flag
  /* since the counter resets at each rising edge, the falling edge
   * is equal to the pulse width */
  // compute Pulse Width in clocks
  // pulse_width = count_at_falling + (1 + TIM5->ARR)*overflow_counts;
  duty = TIM2->CCR2;
 }

 frequency = (float)(APB1ClockRateHz * 2) / (period + 1); // psc * 2
 duty_cycle = (float)((duty + 1) * 100) / (period);
}

#if 0
void TIM2_IRQHandler(void)
{
  /* Clear TIM2 Capture compare interrupt pending bit */
  if ((TIM2->SR & TIM_DIER_CC1IE) && (TIM2->DIER & TIM_DIER_CC1IE))
  {
    TIM2->SR = (uint16_t)~TIM_DIER_CC1IE;
    uint16_t value = TIM2->CCR1;
  }
}
#endif

void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_IMD_DECODE
