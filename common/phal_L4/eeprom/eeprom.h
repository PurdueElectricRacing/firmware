
// #ifndef _PHAL_EEPROM_H_
// #define _PHAL_EEPROM_H_
#if FALSE

#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L471xx
#include "stm32l432xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include "common/common_defs/common_defs.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/i2c/i2c.h"
#include "stdbool.h"
#include "stddef.h"

// Defines
#define MAX_PAGES     63
#define MICRO_PG_SIZE 32
#define MACRO_PG_SIZE 1024
#define CHIP_SIZE     (MACRO_PG_SIZE * 64)
#define NAME_LEN      4
#define MEM_ADDR      0xa0
#define ID_ADDR       0xb0
#define MODE_W        0U
#define MODE_R        1U
#define MEM_FG_TIME   10U

#define E_SUCCESS    0
#define E_NO_MEM     1
#define E_PG_BOUNDS  2
#define E_V_MISMATCH 3
#define E_META       4
#define E_NO_INIT    5
#define E_I2C        6
#define E_M_MISMATCH 7
#define E_NO_NAME    8

#define S_INIT    1
#define S_VERSION 2
#define S_FNAME   252
#define S_ADDR    126
#define S_LEN     126
#define S_BCMP    63

#define M_INIT    0
#define M_VERSION (M_INIT + S_INIT)
#define M_FNAME   (M_VERSION + S_VERSION)
#define M_ADDR    (M_FNAME + S_FNAME)
#define M_LEN     (M_ADDR + S_ADDR)
#define M_BCMP    (M_LEN + S_LEN)

/*
 * Memory Map:
 *
 * +-----------------------------------------+
 * +               METADATA                  +
 * +           Mem init [1 byte]             +
 * +    Version Number (X.X.X) [2 bytes]     +
 * + 63 page names (4 bytes per) [252 bytes] +
 * + 63 page addrs (2 bytes per) [126 bytes] +
 * +  63 page lens (2 bytes per) [126 bytes] +
 * +   Backwards comp. enabled (63 bytes)    +
 * +        Page guard [454 bytes]           +
 * +-----------------------------------------+
 * +                PAGE 0                   +
 * +-----------------------------------------+
 * +                PAGE 1                   +
 * +-----------------------------------------+
 * +                   .                     +
 * +                   .                     +
 * +                   .                     +
 * +-----------------------------------------+
 * +                PAGE 63                  +
 * +-----------------------------------------+
 *
 */

// Structures
struct phys_mem {
    bool init; // Memory initialized
    uint16_t version; // Version number -> X.X.X
    char filename[MAX_PAGES * NAME_LEN]; // 4 character page-name
    uint16_t pg_bound[MAX_PAGES]; // Page bound offset
    uint16_t mem_size[MAX_PAGES]; // Size of file in bytes (actual)
    bool bcmp[MAX_PAGES]; // Backwards compatibility enabled
} __attribute__((__packed__));

struct eeprom {
    struct phys_mem phys; // Physical
    bool init_physical; // Memory on chip initialized
    bool zero_req; // Request chip reset
    uint32_t pg_loaded[2]; // Page loaded
    uint32_t req_flush[2]; // Page requires flushing
    uint8_t* pg_addr[MAX_PAGES]; // Page address
    size_t pg_size[MAX_PAGES]; // Size of file in bytes (desired)
    GPIO_TypeDef* wc_gpio_port; // WC port
    uint32_t wc_gpio_pin; // WC pin

    bool write_pending; // Background wants a write
    uint8_t* source_loc; // Source location for write
    uint16_t dest_loc; // Destination location for write
    uint8_t update_len; // Foreground write length
};

// Prototypes
int initMem(GPIO_TypeDef* wc_gpio_port, uint32_t wc_gpio_pin, uint16_t version, bool force_init);
int checkVersion(uint16_t version);
int mapMem(uint8_t* addr, uint16_t len, uint8_t* fname, bool bcmp);
void memBg(void);
void memFg(void);

#endif