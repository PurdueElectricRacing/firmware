#ifndef TIMESTAMPED_FRAME_H
#define TIMESTAMPED_FRAME_H

#include <stdint.h>

static constexpr uint32_t BUS_ID_MASK = (1u << 31u);
static constexpr uint32_t IS_XID_MASK = (1u << 30u);

typedef struct {
    uint32_t ticks_ms; // ms timestamp of reception
    uint32_t identity; // [1 bit bus ID] [1 bit isExtID] [1 bit reserved] [29 bits CAN ID]
    uint64_t payload;  // message data
} timestamped_frame_t;

static_assert(
    sizeof(timestamped_frame_t) == 16,
    "timestamped_frame_t should be 16 bytes for optimal access patterns"
);


#endif // TIMESTAMPED_FRAME_H