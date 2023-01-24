#ifndef _TMU_H
#define _TMU_H_

#include "common/phal_L4/spi/spi.h"
#include <stdint.h>
#include <inttypes.h>

#define TMU_FILTERED_DATA_CMD (0x5U)
#define TMU_PRODID_ADDR (0x0U)

#define TMU_ADDR_SIZE 0xFFFU
#define TMU_VREF 1.8F

typedef struct {
    SPI_InitConfig_t *spi;
    float tmu_1;
    float tmu_2;
    float tmu_3;
    float tmu_4;
} tmu_handle_t;

void readTemps(tmu_handle_t *tmu);
bool initTMU(tmu_handle_t *tmu);


#endif