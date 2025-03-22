
#include <stdbool.h>
#include "common/phal_F4_F7/spi/spi.h"
#include "common/phal_F4_F7/gpio/gpio.h"

#ifndef _m95p32_H_
#define _m95p32_H_

#define M95_WREN (0x06)     /* Write enable */
#define M95_WRDI (0x04)     /* Write disable */   
#define M95_RDSR (0x05)     /* Read status register */
#define M95_WRSR (0x01)     /* Write status register */
#define M95_READ (0x03)     /* Read data single output */
#define M95_FREAD (0x0B)    /* Fast read single output with one dummy byte */
#define M95_PGWR (0x02)     /* Page write (erase and program)*/
#define M95_PGPR (0x0A)     /* Page program */
#define M95_PGER (0xDB)     /* Page erase (512 bytes) */
#define M95_SCER (0x20)     /* Sector erase (4 Kbytes) */
#define M95_BKER (0xD8)     /* Block erase (64 Kbytes) */
#define M95_CHER (0xC7)     /* Chip erase (ONLY POSSIBLE 100 TIMES) */
#define M95_RDID (0x83)     /* Read identification (EE) */
#define M95_FRDID (0x8B)    /* Fast read identification (EE) */
#define M95_WRID (0x82)     /* Write identification page (EE) */
#define M95_DPD (0xB9)      /* Deep power-down enter */
#define M95_RDPD (0xAB)     /* Deep power-down release */
#define M95_JEDID (0x9F)    /* JEDIEC identification (SF) */
#define M95_RDCR (0x15)     /* Read configuration and safety register */
#define M95_RDVR (0x85)     /* Read volatile register */
#define M95_WRVR (0x81)     /* Write volatile register */
#define M95_CLRSF (0x50)    /* Clear safety sticky flags */
#define M95_RDSFDP (0x5A)   /* Read SFDP */
#define M95_RSTEN (0x66)    /* Enable reset */
#define M95_RESET (0x99)    /* Software reset */

typedef struct {
    bool is_initialized;
    SPI_InitConfig_t *spi;

} M95_t;

bool M95_init(M95_t* M95);
uint8_t M95_getSafetyRegister(M95_t* M95);
void M95_readAddress(M95_t* M95, uint8_t command, uint32_t addr, uint8_t* rx_data, uint32_t rx_len);
void M95_readBytes(M95_t* M95, uint8_t command, uint8_t* rx_data, uint32_t rx_len);
static void PHAL_SPI_transfer_noDMA_M95(SPI_InitConfig_t *spi, const uint8_t *tx_data, uint32_t txlen, uint32_t rxlen, uint8_t *rx_data);
static bool M95_checkManufacturerCode(M95_t* M95);
static inline void M95_selectDevice(M95_t* M95);
static inline void M95_deselectDevice(M95_t* M95);
bool M95_reset(M95_t* M95);

#endif // _m95p32_H_