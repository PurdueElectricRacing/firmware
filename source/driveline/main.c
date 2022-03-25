/* System Includes */
#include "stm32l432xx.h"
#include "common/psched/psched.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include <math.h>
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/phal_L4/can/can.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/eeprom/eeprom.h"
#include "common/phal_L4/usart/usart.h"
#include <math.h>
#include <stdbool.h>

#if (FTR_DRIVELINE_REAR) && (FTR_DRIVELINE_FRONT)
#error "Can not specify both front and rear driveline for the same binary!"
#elif (!FTR_DRIVELINE_REAR) && (!FTR_DRIVELINE_FRONT)
#error "You must define either FTR_DRIVELINE_REAR or FTR_DRIVELINE_FRONT"
#endif

/* Module Includes */
#include "main.h"
#include "can_parse.h"
//#include "wheel_speeds.h"
#include "shockpot.h"
#include "plettenberg.h"

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  // Shock Pots
  GPIO_INIT_ANALOG(POT_AMP_LEFT_GPIO_Port,  POT_AMP_LEFT_Pin),
  GPIO_INIT_ANALOG(POT_AMP_RIGHT_GPIO_Port, POT_AMP_RIGHT_Pin),
  // Motor Controllers
  GPIO_INIT_USART1TX_PA9,
  GPIO_INIT_USART1RX_PA10,
  GPIO_INIT_USART2TX_PA2,
  GPIO_INIT_USART2RX_PA3,
  // Wheel Speed
//   GPIO_INIT_AF(WSPEEDR_GPIO_Port, WSPEEDR_Pin, WHEELSPEEDR_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
//   GPIO_INIT_AF(WSPEEDL_GPIO_Port, WSPEEDL_Pin, WHEELSPEEDL_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
  // EEPROM
  GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_I2C1_SCL_PB6,
  GPIO_INIT_I2C1_SDA_PB7,
  // Status LEDs
  GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
};

/* USART Configuration */
// Left Motor Controller
dma_init_t usart_l_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_l_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_l = {
    .baud_rate   = 115000,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = true,
    .adv_feature.tx_inv    = true,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_l_tx_dma_config,
    .rx_dma_cfg = &usart_l_rx_dma_config
};
// Right Motor Controller
dma_init_t usart_r_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 3);
dma_init_t usart_r_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 4);
usart_init_t huart_r = {
    .baud_rate   = 115000,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = true,
    .adv_feature.tx_inv    = true,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_r_tx_dma_config,
    .rx_dma_cfg = &usart_r_rx_dma_config
};

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .overrun         = true,
    .dma_mode        = ADC_DMA_CIRCULAR
};
ADCChannelConfig_t adc_channel_config[] = {
    {.channel=POT_AMP_LEFT_ADC_CHNL,  .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=POT_AMP_RIGHT_ADC_CHNL, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_shock_pots, 
                            sizeof(raw_shock_pots) / sizeof(raw_shock_pots.pot_left), 0b01);

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source          = SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void mc_analog_init(USART_TypeDef*);
void mc_serial_init(USART_TypeDef*);
void mc_forward(float, USART_TypeDef*);
void mc_reverse(float, USART_TypeDef*);
void mc_brake(float, USART_TypeDef*);
void mc_stop(USART_TypeDef*);
void runMC(void);
void read_motor_controller();
void run_user_commands(void);
void mc_test(void);
void Error_Handler();
void SysTick_Handler();
void canTxUpdate();
void usartTxUpdate();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;
q_handle_t q_tx_usart_l;
q_handle_t q_tx_usart_r;
motor_t motor_left, motor_right;
// TODO: possibly setup is_inverted as daq vars

int main(void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_usart_l, MC_MAX_TX_LENGTH);
    qConstruct(&q_tx_usart_r, MC_MAX_TX_LENGTH);

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initUSART(USART_L, &huart_l, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    if(!PHAL_initUSART(USART_R, &huart_r, APB2ClockRateHz))
    {
        HardFault_Handler();
    }
    if(!PHAL_initCAN(CAN1, false))
    {
        HardFault_Handler();
    }
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    // if(!PHAL_initPWMIn(TIM1, APB2ClockRateHz / TIM_CLOCK_FREQ, TI1FP1))
    // {
    //     HardFault_Handler();
    // }
    // TODO: configure TIM2
    // if(!PHAL_initPWMChannel(TIM1, CC1, CC_INTERNAL, false))
    // {
    //     HardFault_Handler();
    // }
    if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, 
       sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    // Signify start of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    /* Module init */
    initCANParse(&q_rx_can);
    wheelSpeedsInit();
    mc_init(&motor_left,  M_INVERT_LEFT,  &q_tx_usart_l);
    mc_init(&motor_right, M_INVERT_RIGHT, &q_tx_usart_r);

    /* Task Creation */
    schedInit(SystemCoreClock);
    taskCreate(mc_test, 15);
    // TODO: shock is very fast, but contains a bunch of floating point arithmetic
    //taskCreate(shockpot1000Hz, 1);

    // taskCreate(runMC_old, 15);
    // taskCreate(runMC, 1);
    // taskCreate(run_user_commands, 1);

    // TODO: convert to usart DMA
    // TODO: move MC stuff to separate folder
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);
    taskCreateBackground(usartTxUpdate);

    // signify end of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    // while(1)
    // {
    //     PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    //     wheelSpeedsPeriodic();
    // }
    schedStart();
    
    return 0;
}

void mc_test(void) {
    static int state_curr;
    static int count;
    int state;

    state = state_curr;

    switch (state_curr)
    {
        uint16_t data[10];

        case 0:
        {
            data[0] = 's';
            PHAL_usartTxBl(USART1, data, 1);
            data[0] = 'f';
            PHAL_usartTxBl(USART1, data, 1);

            state = 1;

            break;
        }

        case 1:
        {
            data[0] = '1';
            PHAL_usartTxBl(USART1, data, 1);

            state = 2;

            break;
        }

        case 2:
        {
            if (count++ * 15 > 5000) {
                count = 0;

                state = 3;
            }
            
            break;
        }

        case 3:
        {
            data[0] = '0';
            PHAL_usartTxBl(USART1, data, 1);

            state = 4;

            break;
        }

        case 4:
        {
            return;
        }
    }

    state_curr = state;
}

void run_user_commands(void) {
    // TODO: make torque request signed
    // TODO: use torque request
    // TODO: fault checks or whatevs
    static uint16_t time;
    static uint16_t current_power;
    uint16_t goal_power = 0;
    uint16_t goal_power_right = 0;
    if (FTR_DRIVELINE_FRONT) {
        goal_power = can_data.torque_request.front_left;
        goal_power_right = can_data.torque_request.front_right;
    }
    else {
        goal_power = can_data.torque_request.rear_left;
        goal_power_right = can_data.torque_request.rear_right;
    }
    if (goal_power < 0) {
        current_power = goal_power;
        current_power *= -1;
        if (current_power % 2 == 0) {
             mc_brake(2.0, handle);
             current_power -= 2;
        }
        else {
            mc_brake(current_power, handle);
            current_power == 0;
        }
    }
    else if (goal_power > 0) {
        current_power = goal_power;
        current_power *= -1;
        if (current_power % 2 == 0) {
             mc_brake(2.0, handle);
             current_power -= 2;
        }
        else {
            mc_brake(current_power, handle);
            current_power == 0;
        }    }
    else if (goal_power == 0) {
        mc_stop(handle);
    }
    if (goal_power_right < 0) {
        current_power = goal_power;
        current_power *= -1;
        if (current_power % 2 == 0) {
             mc_brake(2.0, handle_two);
             current_power -= 2;
        }
        else {
            mc_brake(current_power, handle_two);
            current_power == 0;
        }
    }
    else if (goal_power_right > 0) {
        current_power = goal_power;
        current_power *= -1;
        if (current_power % 2 == 0) {
             mc_brake(2.0, handle_two);
             current_power -= 2;
        }
        else {
            mc_brake(current_power, handle_two);
            current_power == 0;
        }    }
    else if (goal_power_right == 0) {
        mc_stop(handle_two);
    }
    if (++time == 1001) {
        read_motor_controller();
        time = 0;
    }
}

void ledBlink()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
}

uint8_t usart_cmd[MC_MAX_TX_LENGTH] = {'\0'};
void usartTxUpdate()
{
    // LEFT
    if (PHAL_usartTxDmaComplete(&huart_l) && 
        qReceive(&q_tx_usart_l, usart_cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_L, &huart_l, (uint16_t *) usart_cmd, strlen(usart_cmd));
    }
    // RIGHT
    if (PHAL_usartTxDmaComplete(&huart_r) && 
        qReceive(&q_tx_usart_r, usart_cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_R, &huart_r, (uint16_t *) usart_cmd, strlen(usart_cmd));
    }
}

void usartRxUpdate()
{

}

void canTxUpdate()
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
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

        rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0)  & 0xFF;
        rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8)  & 0xFF;
        rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0)  & 0xFF;
        rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8)  & 0xFF;
        rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        CAN1->RF0R |= (CAN_RF0R_RFOM0); 

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}