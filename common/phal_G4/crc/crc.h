/**
 * @file crc.h
 * @brief G4 Hardware CRC32 HAL
 */

#ifndef _PHAL_G4_CRC_H
#define _PHAL_G4_CRC_H

#include "common/phal_G4/phal_G4.h"

void PHAL_CRC32_Reset(void);
void PHAL_CRC32_Init(void);

uint32_t PHAL_CRC32_Calculate(uint32_t* data, uint32_t count);
uint32_t PHAL_CRC32_CalculateSW(uint32_t* data, uint32_t count);

#endif // _PHAL_G4_CRC_H
