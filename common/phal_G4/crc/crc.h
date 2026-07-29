#ifndef __PHAL_G4_CRC_H__
#define __PHAL_G4_CRC_H__

#include "common/phal_G4/phal_G4.h"

void PHAL_CRC_init(void);

uint32_t PHAL_CRC_calculate(const uint32_t *data, uint32_t words);

uint32_t PHAL_CRC_calculate_sw(const uint32_t *data, uint32_t words);

#endif // __PHAL_G4_CRC_H__
