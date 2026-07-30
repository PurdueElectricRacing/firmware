#include "common/phal_G4/crc/crc_priv.h"

void CRC_PRIV_enableClock(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    (void) RCC->AHB1ENR;
}

void CRC_PRIV_setConfig(void) {
    CRC->CR = 0;
    CRC->POL = CRC_POLY_CRC32;
    CRC->INIT = CRC_INIT_VALUE;
}

void CRC_PRIV_reset(void) {
    CRC->CR |= CRC_CR_RESET;
    __DSB();
}

void CRC_PRIV_feedWord(uint32_t word) {
    CRC->DR = word;
}

uint32_t CRC_PRIV_readResult(void) {
    return CRC->DR;
}