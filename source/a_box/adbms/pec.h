#ifndef _ADBMS_PEC_H_
#define _ADBMS_PEC_H_

#include <stdint.h>
#include <stdbool.h>

uint16_t adbms_pec_get_pec15(uint8_t len, uint8_t *data);
uint16_t adbms_pec_get_pec10(bool bIsRxCmd, uint8_t nLength, uint8_t *pDataBuf);

#endif // _ADBMS_PEC_H_