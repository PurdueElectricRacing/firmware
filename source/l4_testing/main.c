#include "stm32l432xx.h"
#include "can_parse.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/gpio/gpio.h"

#define BUTTON_1_Pin 5
#define BUTTON_1_GPIO_Port GPIOB
#define LED_GREEN_Pin 0
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin 1
#define LED_RED_GPIO_Port GPIOB
#define LED_BLUE_Pin 7
#define LED_BLUE_GPIO_Port GPIOB

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  GPIO_INIT_OUTPUT(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_INPUT(BUTTON_1_GPIO_Port, BUTTON_1_Pin, GPIO_INPUT_PULL_DOWN)
};

// Function Prototypes
void canReceiveTest();
void canSendTest();
void Error_Handler();
void SysTick_Handler();
void canTxUpdate();

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main (void)
{
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    // HAL Library Setup
    PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t));
    PHAL_initCAN(false);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    initCANParse(&q_rx_can);

    // Schedule Tasks
    schedInit(SystemCoreClock);
    taskCreate(canRxUpdate, RX_UPDATE_PERIOD);
    taskCreate(canTxUpdate, 5);
    taskCreate(canSendTest, 5);
    taskCreate(canReceiveTest, 5);
    schedStart();
    
    return 0;
}

uint16_t counter = 1;

void canSendTest()
{
    CanMsgTypeDef_t msg = {.ExtId=ID_TEST_MSG, .DLC=DLC_TEST_MSG, .IDE=1};
    CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;
    data_a->test_msg.test_sig = counter;

    PHAL_txCANMessage(&msg);

    counter += 2;
    if (counter >= 0xFFF)
    {
        counter = 1;
    }
}

void canReceiveTest()
{
    if (!can_data.throttle_brake.stale && can_data.throttle_brake.raw_throttle == 6853)
    {
        PHAL_writeGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin, true);
    } 
    else
    {
        PHAL_writeGPIO(LED_BLUE_GPIO_Port, LED_BLUE_Pin, false);
    }

    if (!can_data.wheel_speeds.stale && can_data.wheel_speeds.fl_speed == 5)
    {
        PHAL_writeGPIO(LED_RED_GPIO_Port, LED_RED_Pin, true);
    }
    else
    {
        PHAL_writeGPIO(LED_RED_GPIO_Port, LED_RED_Pin, false);
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

void CAN1_RX0_IRQHandler()
{
    if (CAN1->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
        CAN1->RF0R &= !(CAN_RF0R_FOVR0); 

    if (CAN1->RF0R & CAN_RF0R_FULL0) // FIFO Full
        CAN1->RF0R &= !(CAN_RF0R_FULL0); 

    if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        CanMsgTypeDef_t rx;

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