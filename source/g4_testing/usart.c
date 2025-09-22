#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_USART)

#include <string.h>

#include "common/freertos/freertos.h"
#include "common/phal_G4/usart/usart.h"
#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "main.h"

// Prototypes
void usart_recieve_complete_callback(usart_init_t* handle);
void HardFault_Handler();
static void usart_tx_task(void);

// Clock Configuration
#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source = CLOCK_SOURCE_HSI,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz = (TargetCoreClockrateHz / (1)),
};
extern uint32_t APB1ClockRateHz;

// GPIO Configuration for LPUART1
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,
};

// DMA Buffers
#define RX_BUFFER_SIZE 12
#define TX_BUFFER_SIZE 12
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

dma_init_t usart_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 2);

// USART Configuration
usart_init_t usart_config = {
    .periph = USART1,
    .baud_rate = 115200,
    .word_length = WORD_8,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .ovsample = OV_16,
    .obsample = OB_DISABLE,
    .usart_active_num = USART1_ACTIVE_IDX,
    .tx_dma_cfg = &usart_tx_dma_config,
    .rx_dma_cfg = &usart_rx_dma_config
};

// FreeRTOS Task Definition
defineThreadStack(usart_tx_task, 1000, osPriorityNormal, 256);

int main()
{
    osKernelInitialize();

    if (PHAL_configureClockRates(&clock_config)) HardFault_Handler();
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) HardFault_Handler();

    // Initialize USART, passing the peripheral clock frequency
    if (!PHAL_initUSART(&usart_config, APB1ClockRateHz)) HardFault_Handler();

    // Start a continuous DMA reception. The callback will handle incoming data.
    // PHAL_usartRxDma(&usart_config, rx_buffer, RX_BUFFER_SIZE, true);

    createThread(usart_tx_task);
    osKernelStart();

    return 0;
}

/**
 * @brief This callback is triggered by the HAL when an idle line is detected,
 * signifying the end of a DMA reception. It echoes the received data back.
 *
 * @param handle Pointer to the usart_init_t struct for the active peripheral.
 */
void usart_recieve_complete_callback(usart_init_t* handle)
{
    return;
}


/**
 * @brief Periodically transmits a "PING" message over USART.
 */
static void usart_tx_task(void)
{
    char* ping_msg = "PING!\r\n";
    uint32_t msg_len = strlen(ping_msg);
    // Wait until the USART peripheral is not busy transmitting
    while(PHAL_usartTxBusy(&usart_config));
    // Copy message to tx buffer and send
    memcpy(tx_buffer, ping_msg, msg_len);
    PHAL_usartTxDma(&usart_config, tx_buffer, msg_len);
}

void HardFault_Handler()
{
    // GPIO_write(GPIOC, 10, 1);
    while (1)
    {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_USART
