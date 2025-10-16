#include "g4_testing.h"
#include "stm32g474xx.h"
#if (G4_TESTING_CHOSEN == TEST_USART)

#include <string.h>

#include "common/freertos/freertos.h"
#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/usart/usart.h"

// Prototypes
void HardFault_Handler();

// Clock Configuration
#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};
extern uint32_t APB1ClockRateHz;

// GPIO Configuration for LPUART1
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_USART2RX_PA3,
    GPIO_INIT_USART2TX_PA2,
};

// DMA Buffers
#define RX_BUFFER_SIZE 12
#define TX_BUFFER_SIZE 12
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 2);

// USART Configuration
usart_init_t usart_config = {.periph           = USART2,
                             .baud_rate        = 115200,
                             .word_length      = WORD_8,
                             .stop_bits        = SB_ONE,
                             .parity           = PT_NONE,
                             .ovsample         = OV_16,
                             .obsample         = OB_DISABLE,
                             .usart_active_num = USART2_ACTIVE_IDX,
                             .tx_dma_cfg       = &usart_tx_dma_config,
                             .rx_dma_cfg       = &usart_rx_dma_config};


int main() {
    osKernelInitialize();

    if (PHAL_configureClockRates(&clock_config))
        HardFault_Handler();
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
        HardFault_Handler();

    // Initialize USART, passing the peripheral clock frequency
    if (!PHAL_initUSART(&usart_config, APB1ClockRateHz))
        HardFault_Handler();

    // Start a continuous DMA reception. The callback will handle incoming data.
    PHAL_usartRxDma(&usart_config, rx_buffer, RX_BUFFER_SIZE, true);

    osKernelStart();

    return 0;
}

/**
 * @brief This callback is triggered by the HAL when an idle line is detected,
 * signifying the end of a DMA reception. It mirrors the received data back using TX DMA.
 *
 * @param handle Pointer to the usart_init_t struct for the active peripheral.
 */
void usart_receive_complete_callback(usart_init_t* handle) {
    // Mirror received data back using TX DMA
    while(PHAL_usartTxBusy(handle));
    PHAL_usartTxDma(handle, rx_buffer, RX_BUFFER_SIZE);
    // Clear RX buffer after echo
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
}

void HardFault_Handler() {
    // GPIO_write(GPIOC, 10, 1);
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_USART
