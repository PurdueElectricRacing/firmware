/**
 * @file pec.h
 * @brief Calculation of PEC (CRC15, CRC10) for ADBMS communication.
 *
 * @author Analog Devices, Millan Kumar (kumar798@purdue.edu), Irving Wang (irvingw@purdue.edu)
 */

#ifndef _ADBMS_PEC_H_
#define _ADBMS_PEC_H_

#include <stdbool.h>
#include <stdint.h>

uint16_t adbms_pec_get_pec15(uint8_t len, const uint8_t *data);
uint16_t adbms_pec_get_pec10(bool bIsRxCmd, uint8_t nLength, const uint8_t *pDataBuf);

#endif // _ADBMS_PEC_H_