#ifndef __PHAL_G4_CRC_PRIV_H__
#define __PHAL_G4_CRC_PRIV_H__

#include "common/phal_G4/phal_G4.h"

#define CRC_POLY_CRC32 (0x04C11DB7)
#define CRC_INIT_VALUE (0xFFFFFFFF)

static inline void crc_setConfig(void) {
    CRC->CR = 0;
    CRC->POL = CRC_POLY_CRC32;
    CRC->INIT = CRC_INIT_VALUE;
}

static inline void crc_reset(void) {
    CRC->CR |= CRC_CR_RESET;
    __DSB();
}

static inline void crc_feedWord(uint32_t word) {
    CRC->DR = word;
}

static inline uint32_t crc_readResult(void) {
    return CRC->DR;
}

#endif // __PHAL_G4_CRC_PRIV_H__