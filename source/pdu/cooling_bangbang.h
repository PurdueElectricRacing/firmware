#ifndef PDU_COOLING_BANGBANG_H
#define PDU_COOLING_BANGBANG_H

#include <stdbool.h>

#include "state.h"

#ifndef PDU_COOLING_ENABLE_BANGBANG
#define PDU_COOLING_ENABLE_BANGBANG 0
#endif

bool cooling_bangbang_is_enabled(void);
void cooling_bangbang_init(void);
void cooling_bangbang_update(pdu_cooling_command_t *cooling_command);

#endif // PDU_COOLING_BANGBANG_H
