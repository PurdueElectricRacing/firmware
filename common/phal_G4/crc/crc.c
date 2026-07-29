#include "common/phal_G4/crc/crc.h"

void PHAL_CRC_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
}

uint32_t PHAL_CRC_calculate(const uint32_t *data, uint32_t words) {
    CRC->CR = CRC_CR_RESET;
    __DSB();

    for (uint32_t i = 0; i < words; i++) {
        CRC->DR = data[i];
    }

    return CRC->DR;
}

static const uint32_t crc32b_LUT[16] = {
    0x00000000,
    0x04C11DB7,
    0x09823B6E,
    0x0D4326D9,
    0x130476DC,
    0x17C56B6B,
    0x1A864DB2,
    0x1E475005,
    0x2608EDB8,
    0x22C9F00F,
    0x2F8AD6D6,
    0x2B4BCB61,
    0x350C9B64,
    0x31CD86D3,
    0x3C8EA00A,
    0x384FBDBD,
};

static inline uint32_t crc_step(uint32_t crc, uint8_t data) {
    crc ^= data;

    for (int i = 0; i < 8; i++) {
        crc = (crc << 4) ^ crc32_mpeg2_lut[crc >> 28];
    }
    return crc;
}

uint32_t PHAL_CRC_calculate_sw(const uint32_t *data, uint32_t words) {
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < words; i++) {
        crc = crc_step(crc, data[i]);
    }
    return crc;
}