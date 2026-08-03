#include "g4_testing.h"
#include "stm32g474xx.h"
#if (G4_TESTING_CHOSEN == TEST_USART)

#include <string.h>

#include "common/freertos/freertos.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/usart/usart.h"
#include "common/utils/countof.h"

// Prototypes
void HardFault_Handler();


// GPIO Configuration for USART2
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_USART2RX_PA3,
    GPIO_INIT_USART2TX_PA2,
};

// DMA Buffers
#define RX_BUFFER_SIZE 12
#define TX_BUFFER_SIZE 12
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

// USART Configuration
PHAL_USART_Handle_t usart_config = {
    .periph    = USART2_IDX,
    .baud_rate = 115200,
};

int main() {
    PHAL_RCC_init(PHAL_RCC_HSI_16MHZ);

    if (!PHAL_initGPIO(gpio_config, countof(gpio_config)))
        HardFault_Handler();

    // Initialize USART, passing the peripheral clock frequency
    if (!PHAL_USART_init(&usart_config, PHAL_RCC_getAPB1ClockHz()))
        HardFault_Handler();

    // Start a continuous DMA reception. PHAL_USART_rxCallback handles incoming data.
    if (!PHAL_USART_rxDMA(&usart_config, rx_buffer, RX_BUFFER_SIZE, true))
        HardFault_Handler();

    vTaskStartScheduler();

    return 0;
}

/**
 * @brief Invoked by the HAL when an idle line is detected, signifying the end
 * of a DMA reception. Mirrors the received data back using TX DMA.
 *
 * @param handle Handle of the USART that received the frame
 */
void PHAL_USART_rxCallback(PHAL_USART_Handle_t *handle) {
    // Mirror received data back using TX DMA
    while (PHAL_USART_txBusy(handle));
    PHAL_USART_txDMA(handle, rx_buffer, RX_BUFFER_SIZE);
    // Clear RX buffer after echo
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_USART
