#ifndef __PHAL_G4_CRC_PRIV_H__
#define __PHAL_G4_CRC_PRIV_H__

#include "common/phal_G4/phal_G4.h"

#define CRC_POLY_CRC32 (0x04C11DB7)
#define CRC_INIT_VALUE (0xFFFFFFFF)

void CRC_PRIV_enableClock(void);
void CRC_PRIV_setConfig(void);
void CRC_PRIV_reset(void);
void CRC_PRIV_feedWord(uint32_t word);
uint32_t CRC_PRIV_readResult(void);

#endif // __PHAL_G4_CRC_PRIV_H__