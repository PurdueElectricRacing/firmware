#ifndef _PHAL_USART_H_
#define _PHAL_USART_H_

// Includes
#define TOTAL_NUM_UART 8

// Active Transfer list indexes (Add to this list if updating TOTAL_NUM_UART)
#define USART1_ACTIVE_IDX 0
#define USART2_ACTIVE_IDX 1
#define USART3_ACTIVE_IDX 2
#define UART4_ACTIVE_IDX  3
#define LPUART1_ACTIVE_IDX 4
#define USART5_ACTIVE_IDX 5
#define USART6_ACTIVE_IDX 6
#define USART7_ACTIVE_IDX 7

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/phal_G4.h"

typedef uint32_t ptr_int;

// Enumerations
// See USART section of Family reference manual for configuration information

typedef enum {
    PT_EVEN = 0b010, //!< Even Parity
    PT_ODD  = 0b100, //!< Odd Parity
    PT_NONE = 0b000  //!< Disable Parity bits
} parity_t;

typedef enum {
    WORD_8, //!< 8-Bit word length
    WORD_9, //!< 9-Bit word length
} word_length_t;

typedef enum {
    SB_ONE      = 0b00, //!< One Stop bit
    SB_TWO      = 0b01, //!< Two Stop bits
    SB_HALF     = 0b10, //!< One half stop bit
    SB_ONE_HALF = 0b11  //!< 1.5 Stop bits
} stop_bits_t;

typedef enum {
    HW_DISABLE, //!< Hardware flow control disable
    CTS,        //!< Enable Clear to send control
    RTS,        //!< Enable Request to send control
    CTS_RTS     //!< Enable both CTS and RTS
} hw_flow_ctl_t;

typedef enum {
    OV_16 = 0, //!< Oversample by 16
    OV_8  = 1  //!< Oversample by 8
} ovsample_t;

typedef enum {
    OB_DISABLE, //!< Disable one bit sampling
    OB_ENABLE   //!< Enable one bit sampling
} obsample_t;

typedef struct
{
    bool overrun;        //!< USART unable to parse data in time
    bool noise_detected; //!< Oversampling detected a possible bit flip due to noise in usart frame
    bool framing_error;  //!< Unable to understand USART frame
    bool parity_error;   //!< USART Parity bit incorrect (Only when parity is enabled)

    // DMA Errors
    bool dma_transfer_error;    //!< DMA transfer error
    bool dma_direct_mode_error; //!< DMA error while attempting to operate in direct mode
    bool dma_fifo_overrun;      //!< DMA FIFO has been overrun
} usart_rx_errors_t;

typedef struct
{
    bool dma_transfer_error;    //!< DMA transfer error
    bool dma_direct_mode_error; //!< DMA error while attempting to operate in direct mode
    bool dma_fifo_overrun;      //!< DMA FIFO has been overrun
} usart_tx_errors_t;

// Structures
typedef struct
{
    // Required parameters
    uint32_t      baud_rate;     //!< Baud rate for communication
    word_length_t word_length;   //!< Word length for tx/rx (8 default)
    stop_bits_t   stop_bits;     //!< Number of stop bits to use (1 default)
    parity_t      parity;        //!< Parity of communication (none default)
    hw_flow_ctl_t hw_flow_ctl;   //!< Special hardware modes (none default)
    ovsample_t    ovsample;      //!< 8x or 16x oversample (16x default)
    obsample_t    obsample;      //!< One bit sampling enable (off default)
    bool          wake_addr;     //!< Wake up when given a specific address
    uint8_t       address;       //!< Address to wake up to when addr_mode is enabled
    uint8_t       usart_active_num; //!< Index of USART in active array (see USARTx_ACTIVE_IDX)

    // DMA configurations
    dma_init_t* tx_dma_cfg; //!< TX configuration
    dma_init_t* rx_dma_cfg; //!< RX configuration
    USART_TypeDef* periph;     //!< USART Peripheral to be used

    // Structs to communicate errors to user
    volatile usart_tx_errors_t tx_errors; //!< Any TX error flags set during transmission
    volatile usart_rx_errors_t rx_errors; //!< Any RX error flags set during reception
} usart_init_t;

// Function Prototypes

/**
 * @brief Initialize a USART Peripheral with desired settings
 *
 * @param handle Handle containing settings for USART peripheral
 * @param fck Clock rate going to USART peripheral (APBx bus)
 * @return true Successfully initialized USART peripheral
 * @return false Failed to initialize USART peripheral
 */
bool PHAL_initUSART(usart_init_t* handle, const uint32_t fck);

/**
 * @brief TX using no DMA (blocks until complete)
 *
 * @param handle The handle for the usart configuration
 * @param data The address of the data to send
 * @param len Number of u8s
 */
void PHAL_usartTxBl(usart_init_t* handle, uint8_t* data, uint32_t len);

/**
 * @brief RX using no DMA (blocks until complete)
 *
 * @param handle The handle for the usart configuration
 * @param data The address of the data to receive
 * @param len Number of u8s expected to receive. Will block if not received.
 */
void PHAL_usartRxBl(usart_init_t* handle, uint8_t* data, uint32_t len);

/**
 * @brief Starts a tx using dma, use PHAL_usartTxDmaComplete to ensure the previous transmission is complete
 *
 * @param handle The handle for the usart configuration
 * @param data The address of the data to send, ensure a cast to (uint16_t *), even if 8 bits
 * @param len Number of units of data, depending on the configured word length
 */
bool PHAL_usartTxDma(usart_init_t* handle, uint16_t* data, uint32_t len);

/**
 * @brief Starts an rx using dma of a specific length
 *
 * @param handle The handle for the usart configuration
 * @param data The address to put the received data, ensure a cast to (uint16_t *), even if 8 bits
 * @param len Number of units of data, depending on the configured word length
 * @param cont Enable Continous RX using the idle line interrupt (only need to call this function once, and HAL will keep recieving messages of the same length)
 */
bool PHAL_usartRxDma(usart_init_t* handle, uint16_t* data, uint32_t len, bool cont);

/**
 * @brief Disables the Continous RX that was previously used
 *
 * @param handle The handle for the usart configuration
 */
bool PHAL_disableContinousRxDMA(usart_init_t* handle);

/**
 * @brief Returns whether USART peripheral is currently transmitting data
 *
 * @param handle Handle of USART peripheral to check
 * @return true USART peripheral is currently sending a message
 * @return false USART peripheal is not currently sending a message
 */
volatile bool PHAL_usartTxBusy(usart_init_t* handle);

/**
 * @brief Returns whether USART peripheral is currently receiving data
 *
 * @param handle Handle of USART peripheral check
 * @return true USART peripheral is currently receiving a message
 * @return false USART peripheal is not currently receiving a message
 */
bool PHAL_usartRxBusy(usart_init_t* handle);

/**
 * @brief Callback function called immediately after reception of a USART RX message
 * Uses USART IDLE line interrupt
 *
 * NOTE: this is executed during an interrupt handler call, so keep code in this function light
 *
 * @param handle Handle of USART peripheral that just recieved a message
 */
extern void usart_recieve_complete_callback(usart_init_t* handle);

// Helper function to handle USART interrupts
static void handleUsartIRQ(USART_TypeDef* periph, uint8_t idx);
// Helper function to handle DMA interrupts
static void handleDMAxComplete(DMA_TypeDef* dma_periph, uint8_t channel, uint8_t dma_type, uint8_t idx);

// DMA Configuration Macros for STM32G4
#define _DEF_USART_RXDMA_CONFIG(rx_addr_, priority_, UXART, dmanum, channelnum, channum_req) \
    { \
        .periph_addr = (uint32_t) & ((UXART)->RDR), .mem_addr = (uint32_t)(rx_addr_), \
        .tx_size = 1, .increment = false, .circular = false, \
        .dir = 0b0, .mem_inc = true, .periph_inc = false, .mem_to_mem = false, \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00, \
        .tx_isr_en = true, .dma_chan_request = channum_req, .channel_idx = channelnum, \
        .periph = DMA##dmanum, \
    }

#define _DEF_USART_TXDMA_CONFIG(tx_addr_, priority_, UXART, dmanum, channelnum, channum_req) \
    { \
        .periph_addr = (uint32_t) & ((UXART)->TDR), .mem_addr = (uint32_t)(tx_addr_), \
        .tx_size = 1, .increment = false, .circular = false, \
        .dir = 0b1, .mem_inc = true, .periph_inc = false, .mem_to_mem = false, \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00, \
        .tx_isr_en = true, .dma_chan_request = channum_req, .channel_idx = channelnum, \
        .periph = DMA##dmanum, \
    }

// Common DMA channel mappings for STM32G4
#define USART1_TXDMA_CONT_CONFIG(a, p) _DEF_USART_TXDMA_CONFIG(a, p, USART1, 1, 7, 1)
#define USART1_RXDMA_CONT_CONFIG(a, p) _DEF_USART_RXDMA_CONFIG(a, p, USART1, 1, 5, 1)

#define USART2_TXDMA_CONT_CONFIG(a, p) _DEF_USART_TXDMA_CONFIG(a, p, USART2, 1, 7, 2)
#define USART2_RXDMA_CONT_CONFIG(a, p) _DEF_USART_RXDMA_CONFIG(a, p, USART2, 1, 6, 2)

#define USART3_TXDMA_CONT_CONFIG(a, p) _DEF_USART_TXDMA_CONFIG(a, p, USART3, 1, 2, 3)
#define USART3_RXDMA_CONT_CONFIG(a, p) _DEF_USART_RXDMA_CONFIG(a, p, USART3, 1, 3, 3)

#define UART4_TXDMA_CONT_CONFIG(a, p)  _DEF_USART_TXDMA_CONFIG(a, p, UART4, 2, 3, 1)
#define UART4_RXDMA_CONT_CONFIG(a, p)  _DEF_USART_RXDMA_CONFIG(a, p, UART4, 2, 5, 1)

#define LPUART1_TXDMA_CONT_CONFIG(a, p) _DEF_USART_TXDMA_CONFIG(a, p, LPUART1, 2, 7, 3)
#define LPUART1_RXDMA_CONT_CONFIG(a, p) _DEF_USART_RXDMA_CONFIG(a, p, LPUART1, 2, 6, 3)

#endif
