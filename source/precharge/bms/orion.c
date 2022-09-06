#include "orion.h"

#include <stdbool.h>
#include "can_parse.h"
#include "common_defs.h"

bool check_errors() {
    int err = 0;
    uint16_t relay_state = can_data.orion_info.relay_state;
    err = (((relay_state & 0x1000) == 0x1000) ? 0 : 1);
    return err;
}