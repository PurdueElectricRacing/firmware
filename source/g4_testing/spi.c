#include "g4_testing.h"
#include "stm32g474xx.h"
#if (G4_TESTING_CHOSEN == TEST_SPI)

// #include <string.h>

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/phal_G4/spi/spi.h"

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
extern uint32_t APB2ClockRateHz;

// GPIO Configuration: example pins for SPI1 (master) and SPI2 (slave)
// Adjust to your board wiring. SPI1: PA5=SCK, PA7=MOSI, PA6=MISO; PA4=CS
// SPI2: PB13=SCK, PB15=MOSI, PB14=MISO; PB12=CS
GPIOInitConfig_t gpio_config[] = {
    // SPI1 - Standard Pins
    GPIO_INIT_SPI1SCK_PA5,
    GPIO_INIT_SPI1MOSI_PA7,
    GPIO_INIT_SPI1MISO_PA6,
    GPIO_INIT_OUTPUT(GPIOA, 4, GPIO_OUTPUT_ULTRA_SPEED), // NSS as software output for master

    // SPI2 - RET Specific (Port B)
    GPIO_INIT_SPI2SCK_RET_PB13,
    GPIO_INIT_SPI2MOSI_RET_PB15,
    GPIO_INIT_SPI2MISO_RET_PB14,
    GPIO_INIT_SPI2NSS_RET_PB12, // Hardware NSS for SPI2 slave
};

// DMA Buffers
#define XFER_LEN 8
static uint8_t master_tx[XFER_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
static uint8_t master_rx[XFER_LEN] = {0};
static uint8_t slave_tx[XFER_LEN]  = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x12, 0x34};
static uint8_t slave_rx[XFER_LEN]  = {0};

// DMA configs
static dma_init_t spi1_rx_dma = SPI1_RXDMA_CONT_CONFIG(NULL, 1);
static dma_init_t spi1_tx_dma = SPI1_TXDMA_CONT_CONFIG(NULL, 2);
static dma_init_t spi2_rx_dma = SPI2_RXDMA_CONT_CONFIG(NULL, 1);
static dma_init_t spi2_tx_dma = SPI2_TXDMA_CONT_CONFIG(NULL, 2);

// SPI configs
static SPI_InitConfig_t spi1 = {
    .periph        = SPI1,
    .data_rate     = 1000000,
    .data_len      = 8,
    .mode          = SPI_MODE_MASTER,
    .nss_sw        = true,
    .nss_gpio_port = GPIOA,
    .nss_gpio_pin  = (1 << 4),
    .cpol          = 0,
    .cpha          = 0,
    .rx_dma_cfg    = &spi1_rx_dma,
    .tx_dma_cfg    = &spi1_tx_dma,
};

static SPI_InitConfig_t spi2 = {
    .periph        = SPI2,
    .data_rate     = 1000000,
    .data_len      = 8,
    .mode          = SPI_MODE_SLAVE,
    .nss_sw        = false, // use hardware NSS via PB12
    .nss_gpio_port = GPIOB,
    .nss_gpio_pin  = (1 << 12),
    .cpol          = 0,
    .cpha          = 0,
    .rx_dma_cfg    = &spi2_rx_dma,
    .tx_dma_cfg    = &spi2_tx_dma,
};

int main() {
    if (PHAL_configureClockRates(&clock_config))
        HardFault_Handler();
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
        HardFault_Handler();

    if (!PHAL_SPI_init(&spi1))
        HardFault_Handler();
    if (!PHAL_SPI_init(&spi2))
        HardFault_Handler();

    // DMA two-device test: SPI1 master -> SPI2 slave
    PHAL_SPI_transfer(&spi2, slave_tx, XFER_LEN, slave_rx);
    PHAL_SPI_transfer(&spi1, master_tx, XFER_LEN, master_rx);
    while (PHAL_SPI_busy(&spi1) || PHAL_SPI_busy(&spi2))
        ;

    // Non-DMA loopback test: tie PA7 (MOSI) to PA6 (MISO)
    PHAL_SPI_transfer_noDMA(&spi1, master_tx, XFER_LEN, XFER_LEN, master_rx);
    while (PHAL_SPI_busy(&spi1))
        ;

    return 0;
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_SPI
