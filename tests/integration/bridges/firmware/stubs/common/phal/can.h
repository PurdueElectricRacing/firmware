#ifndef PER_TEST_PHAL_CAN_H
#define PER_TEST_PHAL_CAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t instance;
} FDCAN_GlobalTypeDef;

extern FDCAN_GlobalTypeDef per_test_fdcan2;
extern FDCAN_GlobalTypeDef per_test_fdcan3;
#define FDCAN2 (&per_test_fdcan2)
#define FDCAN3 (&per_test_fdcan3)

typedef struct {
    FDCAN_GlobalTypeDef *Bus;
    bool IDE;
    union {
        uint16_t StdId;
        uint32_t ExtId;
    };
    uint8_t DLC;
    uint8_t Data[8];
} CanMsgTypeDef_t;

static inline void PHAL_FDCAN_setFilters(
    FDCAN_GlobalTypeDef *bus,
    const uint32_t *standard_ids,
    size_t standard_count,
    const uint32_t *extended_ids,
    size_t extended_count
) {
    (void)bus;
    (void)standard_ids;
    (void)standard_count;
    (void)extended_ids;
    (void)extended_count;
}

#endif
