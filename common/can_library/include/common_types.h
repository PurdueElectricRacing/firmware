#include <stdint.h>

typedef struct {
    uint8_t bus;
    bool is_extended_id;
    uint32_t unmasked_id;
    uint64_t data_BE;
} can_msg_t;

