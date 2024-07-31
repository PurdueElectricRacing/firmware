/**
 * @file main.c
 * @author Purdue Electric Racing
 * @brief Motor Transition PCB
 * @version 0.1
 * @date 2024
 *
 * @copyright Copyright (c) 2024
 *
 */

/* -------------------------------------------------------
    System Includes 
-------------------------------------------------------- */
#include "common/common_defs/common_defs.h"
#include "common/faults/faults.h"
#include "common/faults/fault_nodes.h"
#include "common/phal_F4_F7/can/can.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"

#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------
    Module Includes 
-------------------------------------------------------- */
#include "can_parse.h"
#include "daq.h"
#include "main.h"

/* -------------------------------------------------------
    Pin Initialization
-------------------------------------------------------- */
GPIOInitConfig_t gpio_config[] = 
{
// TODO MCG: pin configuration

/* -------------------------------------------------------
    PORT A
-------------------------------------------------------- */

/* -------------------------------------------------------
    PORT B
-------------------------------------------------------- */

/* -------------------------------------------------------
    PORT C
-------------------------------------------------------- */

/* -------------------------------------------------------
    PORT D
-------------------------------------------------------- */

/* -------------------------------------------------------
    PORT E
-------------------------------------------------------- */

};

/* -------------------------------------------------------
    USART
-------------------------------------------------------- */
// TODO MSG: serial debug
q_handle_t q_tx_usart;  /* Global USART1 TX Queue */

/* -------------------------------------------------------
    USART1 DMA Configuration
-------------------------------------------------------- */
// TODO MCG: serial DMA

/* -------------------------------------------------------
    USART1 Configuration ( Serial Debug )
-------------------------------------------------------- */
usart_init_t serial_debug_usart = 
{
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .periph      = USART1,
   .wake_addr   = false,
   .usart_active_num = USART1_ACTIVE_IDX,
   .tx_dma_cfg = NULL,
   .rx_dma_cfg = NULL
};

/* -------------------------------------------------------
    Clock Configuration
-------------------------------------------------------- */
// TODO MCG: external oscillator support
ClockRateConfig_t clock_config = 
{
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

/* -------------------------------------------------------
    Clock Rates
-------------------------------------------------------- */
extern uint32_t APB1ClockRateHz;        /* Defined in rcc.c */
extern uint32_t APB2ClockRateHz;        /* Defined in rcc.c */
extern uint32_t AHBClockRateHz;         /* Defined in rcc.c */
extern uint32_t PLLClockRateHz;         /* Defined in rcc.c */

/* -------------------------------------------------------
    Procedures
-------------------------------------------------------- */
extern void HardFault_Handler(void);
void preflightAnimation(void); 
void preflightChecks(void); 
void usartTxUpdate();

/**
 * Procedure: CAN1_RX0_IRQHandler()
 * 
 * @brief Interrupt handler for CAN1 RX0.
 * 
 * This function handles the interrupt request for CAN1 RX0 by calling the 
 * `canParseIRQHandler` function, passing the CAN1 instance as a parameter.
 */
void CAN1_RX0_IRQHandler()
{
    canParseIRQHandler(CAN1);

} /* CAN1_RX0_IRQHandler() */


/**
 * Procedure: HardFault_Handler()
 * 
 * @brief Handler for HardFault exceptions.
 * 
 * This function is called when a HardFault exception occurs. It pauses the scheduler
 * and enters an infinite loop where the Independent Watchdog (IWDG) key register is 
 * repeatedly refreshed to prevent a system reset.
 */
void HardFault_Handler()
{
   schedPause();
   while(1)
    {
        IWDG->KR = 0xAAAA;
    }

} /* HardFault_Handler() */

// TODO MCG: determine debug msg payload size
uint8_t cmd[20] = {'\0'};     /* Command buffer for USART transmission */

/**
 * Procedure: main()
 * 
 * @brief entry point
 * 
 */
int main(void)
{
    // TODO MCG: determine debug msg payload size
    qConstruct(&q_tx_usart, 20U);  /* TX queue for USART1 */

    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }

    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    initFaultLibrary(FAULT_NODE_NAME, &q_tx_can1_s[0], ID_FAULT_SYNC_RHENIUM);
                                                                    /* Initialize fualt library*/

    schedInit(APB1ClockRateHz);                                     /* Initialize the scheduler */

    /* Preflight */
    configureAnim(preflightAnimation, preflightChecks, 60, 2500);

    /* Background Tasks */
    taskCreateBackground(usartTxUpdate);
    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    /* Start all tasks */
    schedStart();

    return 0;
}


/**
 * Procedure: preflightAnimation()
 * 
 * @brief Executes the preflight animation sequence.
 */
void preflightAnimation(void) 
{
// TODO MCG: preflight animation if we have GPIOS for it

} /* preflightAnimation() */


/**
 * Procedure: preflightChecks()
 * 
 * @brief Performs preflight checks and initialization for various modules.
 * 
 * This function performs a series of preflight checks and initializations for 
 * CAN, USART, and registers the preflight completion.
 */
void preflightChecks(void) 
{
    static uint8_t state;

    switch (state++)
    {
        case 0:
            if (false == PHAL_initCAN(CAN1, false, VCAN_BPS))
            {
                HardFault_Handler();
            }
            NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 1:
            if (false == PHAL_initUSART(&serial_debug_usart, APB2ClockRateHz))
            {
                HardFault_Handler();
            }
            break;
        case 2:
            break;
        case 3:
            initCANParse();
            if (daqInit(&q_tx_can1_s[2]))
            {
                HardFault_Handler();
            }
            break;
        default:
            registerPreflightComplete(1);
            state = 255; /* prevent wrap around */
    }

} /* preflightChecks() */


/**
 * Procedure: usartTxUpdate()
 * 
 * @brief Updates the USART transmission for the serial debug.
 * 
 * This function checks if the USART transmission for the serial debug is not busy and if there
 * is a command to send from the transmission queue. If both conditions are met, it 
 * initiates a DMA transmission of the command.
 */
void usartTxUpdate()
{
    if ((false == PHAL_usartTxBusy(&serial_debug_usart)) && (SUCCESS_G == qReceive(&q_tx_usart, cmd)))
    {
        PHAL_usartTxDma(&serial_debug_usart, (uint16_t *) cmd, strlen(cmd));
    }
    
} /* usartTxUpdate() */
