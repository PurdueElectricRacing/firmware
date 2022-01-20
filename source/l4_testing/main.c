/* System Includes */
#include "stm32l432xx.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/eeprom/eeprom.h"
#include <math.h>

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "daq.h"


GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  GPIO_INIT_I2C3_SCL_PA7,
  GPIO_INIT_I2C3_SDA_PB4,
  GPIO_INIT_OUTPUT(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_INPUT(BUTTON_1_GPIO_Port, BUTTON_1_Pin, GPIO_INPUT_PULL_DOWN),
  GPIO_INIT_AF(TIM1_GPIO_Port, TIM1_Pin, TIM1_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
  GPIO_INIT_AF(TIM16_GPIO_Port, TIM16_Pin, TIM16_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP)
};

ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_PLL,
    .system_clock_target_hz     =80000000,
    .pll_src                    =PLL_SRC_HSI16,
    .vco_output_rate_target_hz  =160000000,
    .ahb_clock_target_hz        =80000000,
    .apb1_clock_target_hz       =80000000,// / 16,
    .apb2_clock_target_hz       =80000000,
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void pwmTest();
void myCounterTest();
void canReceiveTest();
void canSendTest();
void Error_Handler();
void SysTick_Handler();
void canTxUpdate();
void setRed(uint8_t* on);
void setGreen(uint8_t* on);
void setBlue(uint8_t* on);
void readRed(uint8_t* on);
void readGreen(uint8_t* on);
void readBlue(uint8_t* on);
void ledBlink();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;

uint8_t my_counter = 0;
uint16_t my_counter2 = 85; // Warning: daq variables with eeprom capability may
                           // initialize to something else

int main (void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    // if(!PHAL_initCAN(CAN1, false))
    // {
    //     HardFault_Handler();
    // }
    // if(!PHAL_initI2C())
    // {
    //     HardFault_Handler();
    // }

    TIM1->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;

    //NVIC_EnableIRQ(CAN1_RX0_IRQn);
    /*##-2- Configure the NVIC for TIMx #########################################*/
    //HAL_NVIC_SetPriority(TIMx_IRQn, 0, 1);
    
    /* Enable the TIMx global Interrupt */
    NVIC_EnableIRQ(TIM1_CC_IRQn);
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    // signify start of initialization
    PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);

    /* Module init */
    //initCANParse(&q_rx_can);

    // linkReada(DAQ_ID_TEST_VAR, &my_counter);
    // linkReada(DAQ_ID_TEST_VAR2, &my_counter2);
    // linkWritea(DAQ_ID_TEST_VAR2, &my_counter2);
    // linkReadFunc(DAQ_ID_RED_ON, (read_func_ptr_t) readRed);
    // linkReadFunc(DAQ_ID_GREEN_ON, (read_func_ptr_t) readGreen);
    // linkReadFunc(DAQ_ID_BLUE_ON, (read_func_ptr_t) readBlue);
    // linkWriteFunc(DAQ_ID_RED_ON, (write_func_ptr_t) setRed);
    // linkWriteFunc(DAQ_ID_GREEN_ON, (write_func_ptr_t) setGreen);
    // linkWriteFunc(DAQ_ID_BLUE_ON, (write_func_ptr_t) setBlue);
    // if(daqInit(&q_tx_can))
    // {
    //     HardFault_Handler();
    // }

    /* Task Creation */
    //schedInit(APB1ClockRateHz);
    // taskCreate(canRxUpdate, RX_UPDATE_PERIOD);
    // taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);
    // taskCreate(canSendTest, 15);
    //taskCreate(canReceiveTest, 500);
    //taskCreate(myCounterTest, 50);
    //taskCreate(ledBlink, 500);
    //taskCreateBackground(canTxUpdate);

    // signify end of initialization
    PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0);
    pwmTest();
    //schedStart();
    
    return 0;
}

#define DUTY_TO_PULSE(duty, period) ((uint16_t) (duty * (period + 1) / 100))
float meas_freq = 0;
//float meas_duty = 0;
uint16_t overflows = 0;
uint16_t ccr = 0;

void pwmTest()
{
    /*
    
    uint16_t frequency = 100; // Hz
    uint16_t counter_period = 16;//(16000 / 1) - 1;//666 - 1;
    uint8_t duty = 50;
    uint16_t counter_freq = counter_period * frequency;
    uint16_t prescaler = 3952000 / counter_freq; //(SystemCoreClock / 20 / counter_freq) - 1;
    
    // enable timer 16 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
    // counter mode: CR1 DIR and CR1 CMS (not available for 16)
    // clock division: CR1 CKD (leave at 0)
    // auto reload preload CR1 ARPE 
    TIM16->CR1 |= TIM_CR1_ARPE; // buffered
    // auto reload value ARR (period)
    TIM16->ARR = counter_period - 1;
    // prescaler PSC (prescaler)
    TIM16->PSC = prescaler - 1;//(uint16_t) (SystemCoreClock/16000 - 1); // 16 kHz counter clock
    // repetition counter RCR (repetition counter) (leave to 0)
    // reload for prescaler and rep counter
    TIM16->EGR = TIM_EGR_UG;

    // -- channel config --
    // disable chnl 1 bit clear CCER CC1E
    TIM16->CCER &= ~(TIM_CCER_CC1E);
    // set to output  CC1s in CCMR1
    TIM16->CCMR1 &= ~(TIM_CCMR1_CC1S);
    // select OCMode OC1M in CCMR1 (pwm mode)
    TIM16->CCMR1 &= ~(TIM_CCMR1_OC1M);
    TIM16->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    // select output polarity CCER CC1P and CCER CC1N or something
    //TIM16->CCER &= ~(TIM_CCER_CC1P)
    // set CCR1 to Pulse
    TIM16->CCR1 = DUTY_TO_PULSE(duty, counter_period);
    // Preload enable bit CCMR1 TIM_CCMR1_OC1PE
    TIM16->CCMR1 |= TIM_CCMR1_OC1PE;
    // CCMR1 (OCFastMode) (leave disabled OC1FE)
    

    // -- start --
    // enable cap compare chnl CCx_ENABLE TIM_CCxChannelCmd
    TIM16->CCER |= TIM_CCER_CC1E;
    // main output _HAL_TIM_MOE_ENABLE
    //BDTR set MOE
    TIM16->BDTR |= TIM_BDTR_MOE;
    // _HAL_TIM_ENABLE
    TIM16->CR1 |= TIM_CR1_CEN;
    */


    //uint16_t presc_in = 32;//150;
    // PWM --input-- ;D
    // can set the input prescaler (capture / events ICPSC)
    // enable timer 1 clock
    /*
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    // configure prescaler
    TIM1->PSC = presc_in - 1;
    // configure inputs: CH1 for IC1 (freq) AND IC2 (dooty)
    TIM1->CCMR1 |= TIM_CCMR1_CC1S_0;
    // configure edge select: rising for CC1, falling for CC2
    TIM1->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
    // Select the trigger input TI1FP1
    TIM1->SMCR |= (0b101 << TIM_SMCR_TS_Pos) & TIM_SMCR_TS;
    // interrupt enable
    // master, set to update mode for rising edges on overflow
    TIM1->CR2 |= TIM_CR2_MMS_1;
    // Set slave controller to reset mode
    TIM1->SMCR |= TIM_SMCR_SMS_2;
    // enable captures
    TIM1->CCER |= TIM_CCER_CC1E; //| TIM_CCER_CC2E;
    // enable counter
    TIM1->CR1 |= TIM_CR1_CEN;

    // TIM 15 (slave to TIM1)
    //RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    //TIM15->PSC = presc_in - 1;
    // trigger select TS (ITR0) TIM1
    TIM15->SMCR &= ~(TIM_SMCR_TS);
    // clock by rising edge of TRGI (ext clock mode)
    TIM15->SMCR |= (0b0111 << TIM_SMCR_SMS_Pos) & (TIM_SMCR_SMS);
    // input select (TS for CC1)
    TIM15->CCMR1 |= TIM_CCMR1_CC1S;
    // enable capture compare
    TIM15->CCER |= TIM_CCER_CC1E;
    // enable counter
    TIM15->CR1 |= TIM_CR1_CEN;


    // TIM 2
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    TIM2->PSC = presc_in - 1;
    // input, select CH1 for IC1
    TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
    // configure edge select (rising) for chnl 1
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
    // select trigger input TI1FP1
    TIM2->SMCR |= (0b101 << TIM_SMCR_TS_Pos) & TIM_SMCR_TS;
    // Set slave controller to reset mode
    TIM2->SMCR |= TIM_SMCR_SMS_2;
    // enable capture
    TIM2->CCER |= TIM_CCER_CC1E;
    // enable counter
    TIM2->CR1 |= TIM_CR1_CEN;
    */

    while(1)
    {
        // for (uint16_t i = 0; i < 0XFFF; i++);
        // duty++;
        // if (duty > 99) duty = 0;
        // TIM16->CCR1 = DUTY_TO_PULSE(duty, period);

        uint32_t counts = (overflows << 16) | ccr;
        meas_freq = 3952000.0 / presc_in / counts;
        //meas_duty = 100.0 * TIM1->CCR2 / ccr1;
        
    }

}

void ledBlink()
{
    PHAL_toggleGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

void myCounterTest()
{
    my_counter += 1;
    if (my_counter >= 0xFF)
    {
        my_counter = 0;
    }
}

void setRed(uint8_t* on)
{
    PHAL_writeGPIO(LED_RED_GPIO_Port, LED_RED_Pin, *on);
}
void setBlue(uint8_t* on)
{
    PHAL_writeGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin, *on);
}
void setGreen(uint8_t* on)
{
    PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, *on);
}
void readRed(uint8_t* on)
{
    *on = PHAL_readGPIO(LED_RED_GPIO_Port, LED_RED_Pin);
}
void readGreen(uint8_t* on)
{
    *on = PHAL_readGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}
void readBlue(uint8_t* on)
{
    *on = PHAL_readGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

uint16_t counter = 1;
uint16_t counter2 = 1;

void canSendTest()
{
    SEND_TEST_MSG(q_tx_can, (uint16_t) (500 * sin(((double) counter)/100) + 501));
    SEND_TEST_MSG2(q_tx_can, counter2);

    counter += 1;
    counter2 += 2;
    if (counter2 >= 0xFFF)
    {
        counter2 = 1;
    }

    SEND_TEST_MSG3(q_tx_can, counter2);
    SEND_TEST_MSG4(q_tx_can, counter2);
    SEND_TEST_MSG5(q_tx_can, 0xFFF - counter2);
}

void canReceiveTest()
{
    if (!can_data.raw_throttle_brake.stale && can_data.raw_throttle_brake.throttle0 == 6853)
    {
        PHAL_writeGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin, true);
    } 
    else
    {
        PHAL_writeGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin, false);
    }
}

void canTxUpdate()
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
    }
}

uint16_t overflow_ctr = 0;

void TIM1_UP_TIM16_IRQHandler()
{
    if ((TIM1->SR & TIM_SR_UIF) && (TIM1->DIER & TIM_DIER_UIE))
    {
        overflow_ctr++;
        TIM1->SR = ~TIM_SR_UIF;
    }
}

uint32_t funky_counter_XD = 0;
void TIM1_CC_IRQHandler()
{
    if ((TIM1->SR & TIM_SR_CC1IF) && (TIM1->DIER & TIM_DIER_CC1IE))
    {
        overflows = overflow_ctr - 1;
        if (overflow_ctr == 0)
        {
            // shouldn't be happening like at all, maybe at beginning
            __asm__("nop");
            overflows = 1; // idk, makes it think slow
            funky_counter_XD++;
        }
        overflow_ctr = 0;
        ccr = TIM1->CCR1;
        TIM1->SR = ~(TIM_SR_CC1IF);
    }
}


void CAN1_RX0_IRQHandler()
{
    if (CAN1->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
        CAN1->RF0R &= !(CAN_RF0R_FOVR0); 

    if (CAN1->RF0R & CAN_RF0R_FULL0) // FIFO Full
        CAN1->RF0R &= !(CAN_RF0R_FULL0); 

    if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        CanMsgTypeDef_t rx;
        rx.Bus = CAN1;

        // Get either StdId or ExtId
        if (CAN_RI0R_IDE & CAN1->sFIFOMailBox[0].RIR)
        { 
          rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
        }
        else
        {
          rx.StdId = (CAN_RI0R_STID & CAN1->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
        }

        rx.DLC = (CAN_RDT0R_DLC & CAN1->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;

        rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
        rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
        rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
        rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        CAN1->RF0R     |= (CAN_RF0R_RFOM0); 

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(LED_RED_GPIO_Port, LED_RED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}