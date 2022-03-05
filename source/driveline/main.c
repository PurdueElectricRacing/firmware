#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"

#if (FTR_DRIVELINE_REAR) && (FTR_DRIVELINE_FRONT)
#error "Can not specify both front and rear driveline for the same binary!"
#elif (!FTR_DRIVELINE_REAR) && (!FTR_DRIVELINE_FRONT)
#error "You must define either FTR_DRIVELINE_REAR or FTR_DRIVELINE_FRONT"
#endif

/* System Includes */
#include "stm32l432xx.h"
// #include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include <math.h>

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "wheel_speeds.h"
// #include "common_defs.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_AF(WSPEEDR_GPIO_Port, WSPEEDR_Pin, WHEELSPEEDR_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
    GPIO_INIT_AF(WSPEEDL_GPIO_Port, WSPEEDL_Pin, WHEELSPEEDL_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void Error_Handler();
void SysTick_Handler();
void canTxUpdate();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main (void)
{
    /* Data Struct init */
    // qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    // qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

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

    if(!PHAL_initPWMIn(TIM1, APB2ClockRateHz / TIM_CLOCK_FREQ, TI1FP1))
    {
        HardFault_Handler();
    }
    if(!PHAL_initPWMChannel(TIM1, CC1, CC_INTERNAL, false))
    {
        HardFault_Handler();
    }
    // NVIC_EnableIRQ(CAN1_RX0_IRQn);

    // signify start of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    /* Module init */
    // initCANParse(&q_rx_can);
    wheelSpeedsInit();

    /* Task Creation */

    // signify end of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    while(1)
    {
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
        wheelSpeedsPeriodic();
    }
    
    return 0;
}

void ledBlink()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
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
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}