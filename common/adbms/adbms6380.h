#ifndef _ADBMS6380_H_
#define _ADBMS6380_H_

#include <stddef.h>

constexpr size_t ADBMS6380_COMMAND_PKT_SIZE = 4; // 2 for command, 2 for PEC
constexpr size_t ADBMS6380_SINGLE_DATA_PKT_SIZE = 8; // 6 data bytes + 2 PEC


#endif // _ADBMS6380_H_