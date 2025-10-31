#ifndef PEC_H
#define PEC_H

#include <stdint.h>
#include <stdbool.h>

uint16_t adbms_get_pec15(uint8_t len, uint8_t *data);
uint16_t adbms_get_pec10(bool bIsRxCmd, uint8_t nLength, uint8_t *pDataBuf);

#endif // PEC_H