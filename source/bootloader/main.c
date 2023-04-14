/* System Includes */

#include "inttypes.h"

#ifdef STM32L432xx
#include "stm32l432xx.h"
#endif

#ifdef STM32L496xx
#include "stm32l496xx.h"
#endif
// #include "system_stm32l4xx.h"

#include "can_parse.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/bootloader/bootloader_common.h"


/* Module Includes */
#include "node_defs.h"
#include "bootloader.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    CAN_RX_GPIO_CONFIG,
    CAN_TX_GPIO_CONFIG,
    GPIO_INIT_OUTPUT(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    #if (APP_ID == APP_L4_TESTING)
    GPIO_INIT_OUTPUT(GPIOB, 7, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOB, 6, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOB, 1, GPIO_OUTPUT_LOW_SPEED),
    #endif
    #if (APP_ID == APP_MAIN_MODULE)
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    #endif
};
    /* TODO: remove ^ */

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
void HardFault_Handler();
extern void Default_Handler();
void jump_to_application(void);
bool check_boot_health(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

extern char __isr_vector_start; /* VA of the vector table for the bootloader */
extern char _eboot_flash;      /* End of the bootlaoder flash region, same as the application start address */
extern char _estack;      /* The start location of the stack */

/* Bootlaoder timing control */
static volatile uint32_t bootloader_ms = 0;
static volatile uint32_t bootloader_ms_2 = 0;
static volatile bool bootloader_timeout = false;
static volatile bool send_status_flag = false;
static bool send_flash_address = false;

int main (void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    bootloader_ms = 0;

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config))
        HardFault_Handler();

    if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
        HardFault_Handler();
        
    if (1 != PHAL_initCAN(CAN1, false))
        HardFault_Handler();

    #if (APP_ID == APP_MAIN_MODULE)
        PHAL_writeGPIO(GPIOD, 12, 1);
        PHAL_writeGPIO(GPIOD, 13, 1);
    #endif

    /* Module init */
    initCANParse(&q_rx_can);
    BL_init((uint32_t *) &_eboot_flash, &bootloader_ms);

    // Only enable application launch timeout if the device has not
    // boot looped more than allowed.
    bool allow_application_launch = check_boot_health();

    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    CanMsgTypeDef_t tx_msg;
    BL_sendStatusMessage(BLSTAT_BOOT, bootloader_shared_memory.reset_reason);

    /*
        Main bootloader loop.
        Will run until the systick timer times out or the bootloader process completes
    */
    while(!bootloader_timeout || !allow_application_launch)
    {
        while (q_rx_can.item_count > 0)
            canRxUpdate();

        if (send_status_flag)
        {
            send_status_flag = false;

            if (!BL_flashStarted())
                BL_sendStatusMessage(BLSTAT_WAIT, bootloader_ms);
            else
                BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t) BL_getCurrentFlashAddress());
        }

        while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
        {
            #if (APP_ID == APP_L4_TESTING)
            PHAL_writeGPIO(GPIOB, 1, 1);
            #endif
            if(!PHAL_txCANMessage(&tx_msg))
            {
                qSendToBack(&q_tx_can, &tx_msg); // retry later
                break;
            }
        }
        #if (APP_ID == APP_L4_TESTING)
        PHAL_writeGPIO(GPIOB, 1, 0);
        #endif

        /* 
            Check if firmware download is complete
                Attempt to launch the app if so
        */
        if (BL_flashComplete())
        {
            allow_application_launch = true;
            bootloader_timeout = true;
            break;
        }

    }

    NVIC_DisableIRQ(SysTick_IRQn);
    NVIC_DisableIRQ(CAN1_RX0_IRQn);

    // Check the first word of the application, it should contain the MSP
    // an address can not start with 0xFF for the MSP
    BL_sendStatusMessage(BLSTAT_JUMP_TO_APP, 0);
    while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
        PHAL_txCANMessage(&tx_msg);

    jump_to_application();

    // Only sent if first double-word is NULL (no app exists)
    BL_sendStatusMessage(BLSTAT_INVAID_APP, bootloader_shared_memory.reset_count);
    bootloader_shared_memory.reset_reason = RESET_REASON_BAD_FIRMWARE;
    while (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)
        PHAL_txCANMessage(&tx_msg);
    NVIC_SystemReset();
}

void SysTick_Handler(void)
{
    static uint16_t tick;
    tick++;
    bootloader_ms++;

    switch (bootloader_shared_memory.reset_reason)
    {
        case RESET_REASON_BAD_FIRMWARE:
        case RESET_REASON_INVALID:
            // Unknown reset cause or bad firmware
            // Will infinetly wait in bootloader mode
            if (tick % 100 == 0)
            {
                send_status_flag = true;
            }
            if (BL_flashStarted() && bootloader_ms >= 3000)
            {
               BL_timeout(); 
               bootloader_ms = 0;
            }
            break;
        case RESET_REASON_BUTTON:
        case RESET_REASON_DOWNLOAD_FW:
        case RESET_REASON_APP_WATCHDOG:
            // Watchdog reset or a bad firmware boot, 
            // stay in bootlaoder for 3 seconds before attempting to boot again
            if (tick % 100 == 0)
            {
                send_status_flag = true;
            }
            if (bootloader_ms >= 3000)
            {
                bootloader_timeout = true;
            }
            break;
        case RESET_REASON_POR:
            if (tick % 100 == 0)
            {
                send_status_flag = true;
            }
            // Allow some time in case bootloader request present
            if (bootloader_ms >= 500)
            {
                bootloader_timeout = true;
            }
            break;
        default:
            break;  
    }

    if ((BL_flashStarted() && tick % 50 == 0) ||
        !BL_flashStarted() && tick % 250 == 0)
        PHAL_toggleGPIO(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
}

bool check_boot_health(void)
{
    // Initilize the shared memory block if it was corrupted or a POR
    if(bootloader_shared_memory.magic_word != BOOTLOADER_SHARED_MEMORY_MAGIC)
    {
        bootloader_shared_memory.magic_word     = BOOTLOADER_SHARED_MEMORY_MAGIC;
        bootloader_shared_memory.reset_count    = 0;
        bootloader_shared_memory.reset_reason   = RESET_REASON_POR;
    }

    uint32_t reset_cause = RCC->CSR;
    RCC->CSR |= RCC_CSR_RMVF;

    if (reset_cause & (RCC_CSR_BORRSTF | RCC_CSR_LPWRRSTF))
    {
        // Power-on-reset, update reset count to initial value
        // reset_count could have been garbage otherwise
        bootloader_shared_memory.reset_reason = RESET_REASON_POR;
        bootloader_shared_memory.reset_count  = 0;
    }
    else if ((reset_cause & RCC_CSR_SFTRSTF) == RCC_CSR_SFTRSTF)
    {
        // Software reset
        // In this case, we are assuming that the software left a reason for this reboot.
        if (bootloader_shared_memory.reset_reason == RESET_REASON_DOWNLOAD_FW)
        {
            // We wanted to reset for a new firmware download
            bootloader_shared_memory.reset_count = 0;
        }
        else if (bootloader_shared_memory.reset_reason == RESET_REASON_APP_WATCHDOG)
        {
            // Reset from software watchdog (not necessicarly IWDG, just some form of bad software reboot)
            bootloader_shared_memory.reset_count++;
        }
        else if (bootloader_shared_memory.reset_reason == RESET_REASON_BAD_FIRMWARE)
        {
            // Reset from invalid app
            bootloader_shared_memory.reset_count++;
        }
        else
        {
            // Debug reset event
            bootloader_shared_memory.reset_reason = RESET_REASON_BUTTON;
        }
    }
    else if ((reset_cause & RCC_CSR_IWDGRSTF) == RCC_CSR_IWDGRSTF)
    {
        // Application Watchdog reset from hardware watchdog
        bootloader_shared_memory.reset_count++;
        bootloader_shared_memory.reset_reason = RESET_REASON_APP_WATCHDOG;
    }
    else if ((reset_cause & RCC_CSR_PINRSTF) == RCC_CSR_PINRSTF)
    {
        bootloader_shared_memory.reset_reason = RESET_REASON_BUTTON;
        bootloader_shared_memory.reset_count  = 0;
    } 
    else
    {
        bootloader_shared_memory.reset_reason = RESET_REASON_BUTTON;
    }
    
    if (bootloader_shared_memory.reset_count >= 3)
    {
        // Reached the maximum number of resets
        // Do not allow normal operation
        return false;
    }

    return true;
}

void jump_to_application(void)
{
    uint32_t app_reset_handler_address = *(uint32_t*) (((void *) &_eboot_flash + 4));
    uint32_t msp = (uint32_t) *((uint32_t*) (((void *) &_eboot_flash)));
    uint32_t estack = (uint32_t) ((uint32_t*) (((void *) &_estack)));
    // Confirm app exists
    if (app_reset_handler_address == 0xFFFFFFFF || 
        app_reset_handler_address < 0x8002000 || app_reset_handler_address > 0x807FFFF ||
        msp != estack) return;

    // Reset all of our used peripherals
    RCC->APB1RSTR1  |= RCC_APB1RSTR1_CAN1RST;
    RCC->AHB2RSTR   |= RCC_AHB2RSTR_GPIOBRST;       // Must change based on status led port
    RCC->AHB1RSTR   |= RCC_AHB1RSTR_CRCRST;
    RCC->APB1RSTR1  &= ~(RCC_APB1RSTR1_CAN1RST);
    RCC->AHB2RSTR   &= ~(RCC_AHB2RSTR_GPIOBRST);
    RCC->AHB1RSTR   &= ~(RCC_AHB1RSTR_CRCRST);
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // Make sure the interrupts are disabled before we start attempting to jump to the app
    // Getting an interrupt after we set VTOR would be bad.
    // Actually jump to application
    __disable_irq();
    __set_MSP(msp);
    SCB->VTOR = (uint32_t) (uint32_t*) (((void *) &_eboot_flash));
    __enable_irq();
   ((void(*)(void)) app_reset_handler_address)();
}

static uint32_t can_irq_hits = 0;
void CAN1_RX0_IRQHandler()
{
    #if (APP_ID == APP_L4_TESTING)
    PHAL_toggleGPIO(GPIOB, 6);
    #endif
    can_irq_hits ++;
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
    NVIC_SystemReset();
    while(1)
        ;
}