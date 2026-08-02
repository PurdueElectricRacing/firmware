#include "g4_testing.h"
#include "stm32g474xx.h"
#if (G4_TESTING_CHOSEN == TEST_USART)

#include <string.h>

#include "common/freertos/freertos.h"
#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/usart/usart.h"
#include "common/utils/countof.h"

// Prototypes
void HardFault_Handler();


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

PHAL_DMA_Handle_t usart_rx_dma = {
    .wiring = &USART2_RX_DMA_WIRING,
    .params = {
        .mem_addr = 0,
        .tx_size  = 0,
        .priority = DMA_PRIORITY_HIGH,
        .mode     = DMA_MODE_CIRCULAR,
        .mem_inc  = true,
        .tx_isr_en = true,
    },
};
PHAL_DMA_Handle_t usart_tx_dma = {
    .wiring = &USART2_TX_DMA_WIRING,
    .params = {
        .mem_addr = 0,
        .tx_size  = 0,
        .priority = DMA_PRIORITY_HIGH,
        .mode     = DMA_MODE_NORMAL,
        .mem_inc  = true,
        .tx_isr_en = true,
    },
};

// USART Configuration
usart_init_t usart_config = {
    .periph           = USART2,
    .baud_rate        = 115200,
    .word_length      = WORD_8,
    .stop_bits        = SB_ONE,
    .parity           = PT_NONE,
    .ovsample         = OV_16,
    .obsample         = OB_DISABLE,
    .usart_active_num = USART2_ACTIVE_IDX,
    .tx_dma           = &usart_tx_dma,
    .rx_dma           = &usart_rx_dma
};


int main() {
    PHAL_RCC_init(PHAL_RCC_HSI_16MHZ);

    if (!PHAL_initGPIO(gpio_config, countof(gpio_config)))
        HardFault_Handler();

    // Initialize USART, passing the peripheral clock frequency
    if (!PHAL_initUSART(&usart_config, PHAL_RCC_getAPB1ClockHz()))
        HardFault_Handler();

    // Start a continuous DMA reception. The callback will handle incoming data.
    PHAL_usartRxDma(&usart_config, rx_buffer, RX_BUFFER_SIZE, true);

    vTaskStartScheduler();

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
