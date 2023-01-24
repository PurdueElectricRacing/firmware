#ifndef _TMU_H
#define _TMU_H_

#include "common/phal_L4/spi/spi.h"
#include <stdint.h>
#include <inttypes.h>

#define TMU_FILTERED_DATA_CMD (0x5U)
#define TMU_PRODID_ADDR (0x0U)

typedef struct {
    SPI_InitConfig_t *spi;
    float tmu_1;
    float tmu_2;
    float tmu_3;
    float tmu_4;
} tmu_handle_t;

void readTemps(tmu_handle_t *tmu);
void initTMU(tmu_handle_t *tmu);


#endif