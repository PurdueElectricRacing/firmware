#ifndef _AFE_H_
#define _AFE_H_

// Includes
#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include "common/phal_L4/spi/spi.h"
#include "common_defs.h"
#include "bms.h"
#include "string.h"

// Generic Defines
// SPI
#define LTC6811_ADDR_ONE	0x1
#define LTC6811_ADDR_TWO	0x2
#define LTC6811_REG_SIZE    6

// Voltage Flags
#define LTC6811_MAX_PCKV 	50.4 // Max Pack voltage cutoff
#define LTC6811_MIN_PCKV 	30   // Min pack voltage cutoff
#define CELL_MAX_V          4.2
#define CELL_MIN_V          2.7
#define CELL_0_OFFSET       3000
#define TEMP_CONV           (1.0 / 75.0)
#define OW_THRESH           4000

// Balance limits
#define SOC_DRIFT_LIM       2.0f

// ADCV
#define	NORMAL_MODE			0x2
#define REG_A				0x4
#define REG_B				0x6
#define REG_C				0x8
#define REG_D				0xA
#define REG_E				0x9
#define REG_F				0xB

// RDCV
#define RDCVA               0x0004  /** Read the Cell Voltage Register A (CVAR) */
#define RDCVB               0x0006  /** Read the Cell Voltage Register B (CVBR) */
#define RDCVC               0x0008  /** Read the Cell Voltage Register C (CBCR) */
#define RDCVD               0x000A  /** Read the Cell Voltage Register D (CBDR) */

// Timings
#define TYP_T_WAKE          200     /**< [us] */
#define MAX_T_WAKE          400     /**< [us] */

#define MIN_T_SLEEP         1800000 /**< [us] */
#define TYP_T_SLEEP         2000000 /**< [us] */
#define MAX_T_SLEEP         2200000 /**< [us] */

#define MIN_T_REFUP         2700    /**< [us] */
#define TYP_T_REFUP         3500    /**< [us] */
#define MAX_T_REFUP         4400    /**< [us] */

#define MAX_T_READY         10      /**< [us] */

#define MIN_T_IDLE          4300    /**< [us] */
#define TYP_T_IDLE          5500    /**< [us] */
#define MAX_T_IDLE          6700    /**< [us] */

#define T_CONV_CELL_ALL_7K                  2300    /**< [us] */
#define T_CONV_CELL_ONE_7K                  405     /**< [us] */
#define T_CONV_GPIO_ALL_PLUS_REFERENCE_7K   2300    /** [us] */
#define T_CONV_GPIO_ONE_7K                  405     /** [us] */
#define T_CONV_STATUS_ALL_7K                1600    /** [us] */
#define T_CONV_STATUS_ONE_7K                405     /** [us] */

// Commands
#define WRCFGA      0x0001  /** Write to Configuration Register (CFGR) */
#define RDCFGA      0x0002  /** Read the Configuration Register (CFGR) */
#define RDCVA       0x0004  /** Read the Cell Voltage Register A (CVAR) */
#define RDCVB       0x0006  /** Read the Cell Voltage Register B (CVBR) */
#define RDCVC       0x0008  /** Read the Cell Voltage Register C (CBCR) */
#define RDCVD       0x000A  /** Read the Cell Voltage Register D (CBDR) */
#define RDAUXA      0x000C  /** Read the Auxiliary Register Group A (AVAR) */
#define RDAUXB      0x000E  /** Read the Auxiliary Register Group B (AVBR) */
#define RDSTATA     0x0010  /** Read the Status Register Group A (STAR) */
#define RDSTATB     0x0012  /** Read the Status Register Group B (STBR) */
#define WRSCTRL     0x0014  /** Write to the S Control Register Group (SCTRL) */
#define WRPWM       0x0020  /** Write to the PWM Register Group (PWMR) */
#define RDSCTRL     0x0016  /** Read the S Control Register Group (SCTRL) */
#define RDPWM       0x0022  /** Read the PWM Register Group (PWMR) */
#define STSCTRL     0x0019  /** Start S Control pulsing and poll status */
#define CLRSCTRL    0x0018  /** Clear S Control register */
#define CLRCELL     0x0711  /** Clear Cell Voltage Register Groups */
#define CLRAUX      0x0712  /** Clear Auxiliary Register Groups */
#define CLRSTAT     0x0713  /** Clear Status Register Groups */
#define PLADC       0x0714  /** Poll ADC Conversion Status */
#define DIAGN       0x0715  /** Diagnose MUX and Poll Status */
#define WRCOMM      0x0721  /** Write COMM Register Group */
#define RDCOMM      0x0722  /** Read COMM Register Group */
#define STCOMM      0x0723  /** Start I2C /SPI Communication */

// Macros
#define ADCV(MD,DCP,CH)         (0x260 | (MD<<7) | (DCP<<4) | (CH))
#define ADOW(MD,PUP,DCP,CH)     (0x228 | (MD<<7) | (PUP<<6) | (DCP<<4) | (CH))
#define CVST(MD,ST)             (0x207 | (MD<<7) | (ST<<5))
#define ADOL(MD,DCP)            (0x201 | (MD<<7) | (DCP<<4))
#define ADAX(MD,CHG)            (0x460 | (MD<<7) | (CHG))
#define ADAXD(MD,CHG)           (0x400 | (MD<<7) | (CHG))
#define AXST(MD,ST)             (0x407 | (MD<<7) | (ST<<5))
#define ADSTAT(MD,CHST)         (0x468 | (MD<<7) | (CHST))
#define ADSTATD(MD,CHST)        (0x408 | (MD<<7) | (CHST))
#define STATS(MD,ST)            (0x40F | (MD<<7) | (ST<<5))
#define ADCVAX(MD,DCP)          (0x46F | (MD<<7) | (DCP<<4))
#define VUV(voltage_low)        ((voltage_low * 10000 / 16) - 1)
#define VOV(voltage_high)       (voltage_high * 10000 / 16)
#define ADCVSC(MD,DCP)          (0x467 | (MD<<7) | (DCP<<4))
#define byte_combine(msb, lsb)  ((msb << 8) | lsb)

// Enumerations
enum {
	ALL_CELLS,
	CELLS_1_7,
	CELLS_2_8,
	CELLS_3_9,
	CELLS_4_10,
	CELLS_5_11,
	CELLS_6_12
};

enum {
	DISCHARGE_NOT_PERMITTED,
	DISCHARGE_PERMITTED
};

enum {
    S_MIN = 0b0000,
    S_MAX = 0b1111
};

typedef enum {
    BAL_OFF,
    SETTLE,
    MEAS,
    BAL,
    DIAG,
    OW_PU0,
    OW_PU1,
    OW_PD0,
    OW_PD1,
    OW_CALC,
    HALT
} afe_state_t;

// Prototypes
void checkConn(void);
void afeTask(void);
void afeWakeup(void);
void broadcastPoll(uint16_t command);
void broadcastWrite(uint16_t command, uint16_t size, uint8_t* data);
int broadcastRead(uint16_t command, uint16_t size, uint8_t* data);

// PEC Table
static const uint16_t crc15Table[256] = {0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e,
0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990,
0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3,
0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
0x2544, 0x2be,  0xc727, 0xcc15, 0x98c,  0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93,
0xe5a1, 0x2038, 0x7c2,  0xc25b, 0xc969, 0xcf0,  0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d,
0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9,
0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11, 0x57c,  0xc0e5, 0xcbd7,
0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a,
0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9, 0xf84,
0xca1d, 0xc12f, 0x4b6,  0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e,
0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0, 0xd3a,  0xc8a3, 0xc391, 0x608,  0xd5f5, 0x106c, 0x1b5e,
0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb,
0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e,
0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x846,  0xcddf, 0xc6ed, 0x374,
0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8,  0xcf61, 0xc453, 0x1ca,  0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054,
0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1,
0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095};

// Read Commands
static const uint16_t readCmd[4] = {RDCVA, RDCVB, RDCVC, RDCVD};

#endif