#include "stm32l432xx.h"

#include "apps.h"
#include "common/psched/psched.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/can/can.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,
    // GPIO_INIT_INPUT(GPIOA, 10, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(GPIOA, 8, GPIO_OUTPUT_LOW_SPEED)
};

void blinkTask(void);

int main (void)
{
    PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t));

    PHAL_initCAN(true);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    schedInit(SystemCoreClock);
    taskCreate((func_ptr_t) &blinkTask, 100);

    schedStart();


    return 0;
}

CanMsgTypeDef_t tx_msg, rx0_msg, rx1_msg;
uint8_t counter = 0;

void blinkTask(void)
{
    tx_msg.StdId   = 0x123;
    tx_msg.IDE     = 0;
    tx_msg.DLC     = 2;
    tx_msg.Data[0] = 0xAB;
    tx_msg.Data[1] = 0xCD;
    
    if (++counter < 2)
    {
        PHAL_toggleGPIO(GPIOA, 8);
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
        CAN1->RF0R     |= (CAN_RF0R_RFOM0); 
        rx0_msg.StdId   = CAN1->sFIFOMailBox[0].RIR;
        //*((uint32_t *)(rx0_msg.Data[0])) = CAN1->sFIFOMailBox[0].RDLR;
        //*((uint32_t *)(rx0_msg.Data[4])) = CAN1->sFIFOMailBox[0].RDHR;
        // Put into a queue
    }
}
