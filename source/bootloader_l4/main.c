/* System Includes */
#include "stm32l432xx.h"
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"


/* Module Includes */
#include "node_defs.h"
#include "process.h"

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
void check_boot_reason(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

extern uint32_t* __isr_vector_start;
extern char _eboot_flash;      /* End of the bootlaoder flash region, same as the application start address (isr_vector) */

static volatile uint32_t ms_ticks = 0;
bool bootloader_timeout           = false;

__attribute__((section("no_init_data"))) uint32_t reset_count;

int main (void)
{  
    check_boot_reason();

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
    
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    /* Module init */
    initCANParse(&q_rx_can);
    initBLProcess((uint32_t) &_eboot_flash);

    /* Timeout timer */
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);

    while(!bootloader_timeout)
        ;

    if (*(&_eboot_flash) != 0xFF)
    {
        reset_count = 0;
        NVIC_SystemReset();
        jump_to_application();
    }
    else
    {
        NVIC_SystemReset();
    }
}

void SysTick_Handler(void)
{
    ms_ticks++;
    if (ms_ticks == 5000)
    {
        bootloader_timeout = true;
    }
}

void check_boot_reason(void)
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
    
    if (reset_count >= 10)
    {
        // Reached the maximum number of resets
        asm("bkpt");
        // Send a FATAL CAN message
    }
}

void jump_to_application(void)
{
    // Make sure the interrupts are disabled before we start attempting to jump to the app
    // Getting an interrupt after we set VTOR would be bad.
    __disable_irq();

    uint32_t* app_code_start = (uint32_t*) _eboot_flash;
    // Set Vector Offset Table from the application
    SCB->VTOR = app_code_start[0];

    // Main stack pointer is saved as the first entry in the .isr_entry
    __set_MSP(app_code_start[0]);

    // Actually jump to application
    ((void (*)(void))app_code_start[1])();
}


void mainmodule_bl_cmd_IRQ(CanParsedData_t* msg_data_a)
{
    if(APP_ID != APP_MAINMODULE) return;
    process_bl_cmd((BLCmd_t) msg_data_a->mainmodule_bl_cmd.cmd, msg_data_a->mainmodule_bl_cmd.data);
}

void bootloader_bl_cmd_IRQ(CanParsedData_t* msg_data_a)
{
    if(APP_ID != APP_DASHBOARD) return;
    process_bl_cmd((BLCmd_t) msg_data_a->dashboard_bl_cmd.cmd, msg_data_a->dashboard_bl_cmd.data);
}