/* System Includes */
#include "stm32l432xx.h"
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"


/* Module Includes */
#include "node_defs.h"
#include "bootloader.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    CAN_RX_GPIO_CONFIG,
    CAN_TX_GPIO_CONFIG
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
extern void HardFault_Handler();
void jump_to_application(void);
bool check_boot_health(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

extern char __isr_vector_start; /* VA of the vector table for the bootloader */
extern char _eboot_flash;      /* End of the bootlaoder flash region, same as the application start address */

/* Bootlaoder timing control */
static volatile uint32_t timeout_ticks = 0;
static volatile bool bootloader_timeout = false;
static volatile bool wait_flag = false;

__attribute__((section(".no_init_data"))) uint32_t reset_count; /* Reset counter that does not get reset in Reset_Handler during warm boot*/

int main (void)
{  
    // This will make sure that the interrupts are fetched from RAM and not Flash
    // IVT is not copied by default, 
    // memcpy((void*)0x20000000,(void const*)0x08000000,0x1FF);
    // SCB->VTOR = (uint32_t)&__isr_vector_start;

    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config))
        HardFault_Handler();

    if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
        HardFault_Handler();
        
    if (1 != PHAL_initCAN(CAN1, false))
        HardFault_Handler();

    /* Module init */
    initCANParse(&q_rx_can);
    BL_init((uint32_t *) &_eboot_flash, &timeout_ticks);

    // Only enable application launch timeout if the device has not
    // boot looped more than allowed.
    bool allow_application_launch = check_boot_health() & false;

    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    CanMsgTypeDef_t tx_msg;
    BL_sendStatusMessage(BLSTAT_BOOT, reset_count);
    /*
        Main bootloader loop.
        Will run until the systick timer times out or the bootloader process completes
    */
    while(!bootloader_timeout || !allow_application_launch)
    {
        /* Process CAN signals */
        if (wait_flag && !BL_flashStarted())
        {
            BL_sendStatusMessage(BLSTAT_WAIT, timeout_ticks);
            wait_flag = false;
        }
        while (q_rx_can.item_count > 0)
            canRxUpdate();
        while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
            PHAL_txCANMessage(&tx_msg);


        /* Check if firmware download is complete */
        if (BL_flashComplete())
        {
            allow_application_launch = true;
            bootloader_timeout = true;
        }
    }

    NVIC_DisableIRQ(SysTick_IRQn);
    NVIC_DisableIRQ(CAN1_RX0_IRQn);

    // Check the first word of the application, it should contain the MSP
    // an address can not start with 0xFF for the MSP
    if (*((uint8_t *) &_eboot_flash) != 0xFF)
    {
        reset_count = 0;
        BL_sendStatusMessage(BLSTAT_JUMP_TO_APP, 0);
        while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
            PHAL_txCANMessage(&tx_msg);
        asm("bkpt");
        jump_to_application();
    }
    else
    {
        BL_sendStatusMessage(BLSTAT_INVAID_APP, reset_count);
        while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
            PHAL_txCANMessage(&tx_msg);
        asm("bkpt");
        NVIC_SystemReset();
    }

    while(1)
        asm("bkpt");
}

void SysTick_Handler(void)
{
    if (timeout_ticks % 1000 == 0)
    {
        wait_flag = true;
    }

    if (timeout_ticks == 5000)
    {
        bootloader_timeout = true;
    }

    timeout_ticks++;
}

bool check_boot_health(void)
{
    uint32_t reset_cause = RCC->CSR;
    RCC->CSR |= RCC_CSR_RMVF;

    if ((reset_cause & RCC_CSR_BORRSTF) == RCC_CSR_BORRSTF || reset_count > 0xFF)
    {
        // Power-on-reset, update reset count to initial value
        // reset_count could have been garbage otherwise
        reset_count = 0;
    }

    if ((reset_cause & RCC_CSR_SFTRSTF) == RCC_CSR_SFTRSTF || (reset_cause & RCC_CSR_IWDGRSTF) == RCC_CSR_IWDGRSTF)
    {
       // Software reset, watchdog reset 
       reset_count++; 
    }
    
    if (reset_count >= 3)
    {
        // Reached the maximum number of resets
        return false;
    }

    return true;
}

void jump_to_application(void)
{
    // Reset all of our used peripherals
    RCC->APB1RSTR1  |= RCC_APB1RSTR1_CAN1RST;
    RCC->AHB2RSTR   |= RCC_AHB2RSTR_GPIOARST;
    RCC->APB1RSTR1  &= ~(RCC_APB1RSTR1_CAN1RST);
    RCC->AHB2RSTR   &= ~(RCC_AHB2RSTR_GPIOARST);
    SysTick->CTRL = 0;

    // Make sure the interrupts are disabled before we start attempting to jump to the app
    // Getting an interrupt after we set VTOR would be bad.
    __disable_irq();

    uint32_t* app_code_start = (uint32_t *) &_eboot_flash;
    // Set Vector Offset Table from the application
    SCB->VTOR = app_code_start[0];

    // Main stack pointer is saved as the first entry in the .isr_entry
    __set_MSP(app_code_start[0]);

    __enable_irq();

    // Actually jump to application
    ((void (*)(void))app_code_start[1])();
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