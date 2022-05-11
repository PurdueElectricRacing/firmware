#include "common/phal_l4/i2c_alt/i2c_alt.h"

// Static prototypes
static void I2C_IRQHandler(I2C_TypeDef* instance);
static int  gen_start(I2C_TypeDef* instance, uint8_t addr, uint8_t len, bool read);
static void i2c_unmask_irq(USART_TypeDef* instance);
static void i2c_mask_irq(USART_TypeDef* instance);

i2c_handle_t* core;

int PHAL_I2C_init(I2C_TypeDef* instance, i2c_handle_t* handle, uint32_t freq, const uint32_t fck) {
    uint8_t clk_tim;
    uint8_t presc = 0;
    
    // Enable I2C clock
    if (instance == I2C1) {
        RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    } else if (instance == I2C3) {
        RCC->APB1ENR1 |= RCC_APB1ENR1_I2C3EN;
    } else {
        return -E_CONFIG;
    }

    // Disable peripheral
    instance->CR1 &= ~I2C_CR1_PE;

    // Enable DMA
    instance->CR1 |= I2C_CR1_TXDMAEN | I2C_CR1_RXDMAEN | I2C_CR1_TCIE | I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_ERRIE;

    // Set clock timing
    instance->TIMINGR &= 0x0f000000;
    clk_tim = fck / (freq * 2) - 1;

    if (clk_tim > 0xff) {
        presc = 0xff - clk_tim;

        if (presc > 0xff) {
            return -E_CONFIG;
        }
    }

    instance->TIMINGR |= (presc << I2C_TIMINGR_PRESC_Pos) | (clk_tim << I2C_TIMINGR_SCLH_Pos) | (clk_tim << I2C_TIMINGR_SCLL_Pos);

    // Enable peripheral
    instance->CR1 |= I2C_CR1_PE;

    // Configure DMA
    if (handle->tx_dma_cfg && !PHAL_initDMA(handle->tx_dma_cfg)) {
        return false;
    }

    if (handle->rx_dma_cfg && !PHAL_initDMA(handle->rx_dma_cfg)) {
        return false;
    }

    // Set core and unmask interrupts
    core = handle;
    i2c_unmask_irq(instance);

    return 0;
}

/*
    What I'm going for here...

    DMA starts on TX/RX. If DMA hasn't finished from the last pass, return busy.
    Turn on all event interrupts. If there's an error on the bus, stop transfer, store the error code,
    and try to get everything reset for the next go. I think DMA might be the key here. There's no notion
    of checking when the address is actually sent like it is on the F4 variants, so letting DMA decide
    when to pack data might be useful. We can then check in on how far the DMA progress is going with
    calls to read/write with a return of -E_BUSY, and terminate the read/write if it's taking too long.
    So the timeout moves to app code and we aren't in danger of hitting a WWDG reset waiting for something
    to unblock. At least in theory...

    Also, set STOP bit when transfer is complete using DMA interrupt. So a few interrupts going across
    the board.

    It's late and I want some sleep. If only DoorDash could deliver food on time... :/
*/

int PHAL_I2C_write_a(I2C_TypeDef* instance, uint8_t addr, const uint8_t* data, uint8_t len, bool stop, bool force) {
    int ret;
    
    // Check if already busy
    if (core->_tx_busy) {
        if (force) {
            PHAL_stopTxfer(core->tx_dma_cfg);
            PHAL_I2C_stop(instance);
        } else {
            return -E_BUSY;
        }
    }

    // Lock core and set stop
    core->_tx_busy = true;
    core->stop = stop;

    // Configure DMA for TX
    PHAL_DMA_setTxferLength(core->tx_dma_cfg, len);
    PHAL_DMA_setMemAddress(core->tx_dma_cfg, (uint32_t) data);

    // Attempt to generate start
    ret = gen_start(instance, addr, len, true);

    if (ret < 0) {
        core->_tx_busy = false;

        return ret;
    }

    // Start write
    PHAL_startTxfer(core->tx_dma_cfg);

    return 0;
}

int PHAL_I2C_read_a(I2C_TypeDef* instance, uint8_t addr, uint8_t* data, uint8_t len, bool stop, bool force) {
    int ret;

    // Check if already busy
    if (core->_rx_busy) {
        if (force) {
            PHAL_stopTxfer(core->rx_dma_cfg);
            PHAL_I2C_stop(instance);
        } else {
            return -E_BUSY;
        }
    }

    // Lock core and set stop
    core->_rx_busy = true;
    core->stop = stop;

    // Configure DMA for RX
    PHAL_DMA_setTxferLength(core->rx_dma_cfg, len);
    PHAL_DMA_setMemAddress(core->rx_dma_cfg, (uint32_t) data);

    // Attempt to generate start
    ret = gen_start(instance, addr, len, false);

    if (ret < 0) {
        core->_rx_busy = false;

        return ret;
    }

    // Start read
    PHAL_startTxfer(core->rx_dma_cfg);

    return 0;
}

int PHAL_I2C_stop(I2C_TypeDef* instance) {
    instance->CR2 |= I2C_CR2_STOP;
    core->_tx_busy = core->_rx_busy = false;

    return 0;
}

int PHAL_I2C_check_state(void) {
    return (core->_tx_busy || core->_rx_busy) ? -E_BUSY : 0;
}

void DMA1_Channel6_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF6)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF6;
    }
    if (DMA1->ISR & DMA_ISR_TCIF6) 
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF6;
    }
    if (DMA1->ISR & DMA_ISR_GIF6)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF6;
    }
}

void DMA1_Channel7_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF7)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF7;
    }
    if (DMA1->ISR & DMA_ISR_TCIF7) 
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF7;
    }
    if (DMA1->ISR & DMA_ISR_GIF7)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF7;
    }
}

void I2C1_EV_IRQHandler() {
    I2C_IRQHandler(I2C1);
}

void I2C3_EV_IRQHandler() {
    I2C_IRQHandler(I2C3);
}

static void I2C_IRQHandler(I2C_TypeDef* instance) {
    // Check if bus is busy but we don't think we should be busy
    if (instance->ISR & I2C_ISR_BUSY && (!core->_tx_busy && !core->_rx_busy)) {
        core->error[core->err_idx++] = E_BAD_BUSY;
        core->err_idx = (core->err_idx == MAX_ERR) ? 0 : core->err_idx;
    }

    // Check if the bus is not busy and unlock
    if (!(instance->ISR & I2C_ISR_BUSY)) {
        core->_tx_busy = core->_rx_busy = false;
    }

    // Check for arbitration loss
    if (instance->ISR & I2C_ISR_ARLO) {
        core->error[core->err_idx++] = E_ARLO;
        core->err_idx = (core->err_idx == MAX_ERR) ? 0 : core->err_idx;
    }

    // Check for bad start/stop bits
    if (instance->ISR & I2C_ISR_BERR) {
        core->error[core->err_idx++] = E_BERR;
        core->err_idx = (core->err_idx == MAX_ERR) ? 0 : core->err_idx;
    }

    // Check for slave device NAK
    if (instance->ISR & I2C_ISR_NACKF) {
        core->error[core->err_idx++] = E_NACK;
        core->err_idx = (core->err_idx == MAX_ERR) ? 0 : core->err_idx;
    }

    // Send a stop if the last byte was just transmitted
    if (instance->ISR & I2C_ISR_TC) {
        if (core->stop) {
            PHAL_I2C_stop(instance);
        }

        core->_tx_busy = core->_rx_busy = false;
    }

    // Clear ISR so we don't trigger again from NVIC
    instance->ICR = 0b11111100111000;
}

static int gen_start(I2C_TypeDef* instance, uint8_t addr, uint8_t len, bool read) {
    // Clear from last read/write
    instance->CR2 &= 0x07ffffff;

    // Set NBYTES, read/write, and address, then start
    instance->CR2 |= (len << I2C_CR2_NBYTES_Pos) | (read << I2C_CR2_RD_WRN_Pos) | (addr << 1);
    instance->CR2 |= I2C_CR2_START;

    return 0;
}

static void i2c_unmask_irq(USART_TypeDef* instance) {
    switch ((ptr_int) instance)
    {
        case I2C1_BASE:
            NVIC->ISER[0] |= 1U << I2C1_EV_IRQn;
            NVIC->ISER[0] |= 1U << DMA1_Channel6_IRQn;
            NVIC->ISER[0] |= 1U << DMA1_Channel7_IRQn;
            break;

        case I2C3_BASE:
            NVIC->ISER[2] |= 1U << (I2C3_EV_IRQn - 64);
            NVIC->ISER[0] |= 1U << DMA1_Channel2_IRQn;
            NVIC->ISER[0] |= 1U << DMA1_Channel3_IRQn;
            break;
    }
}

static void i2c_mask_irq(USART_TypeDef* instance) {
    switch ((ptr_int) instance)
    {
        case I2C1_BASE:
            NVIC->ISER[0] &= ~(1U << I2C1_EV_IRQn);
            NVIC->ISER[0] &= ~(1U << DMA1_Channel6_IRQn);
            NVIC->ISER[0] &= ~(1U << DMA1_Channel7_IRQn);
            break;

        case I2C3_BASE:
            NVIC->ISER[2] &= ~(1U << (I2C3_EV_IRQn - 64));
            NVIC->ISER[0] &= ~(1U << DMA1_Channel2_IRQn);
            NVIC->ISER[0] &= ~(1U << DMA1_Channel3_IRQn);
            break;
    }
}
