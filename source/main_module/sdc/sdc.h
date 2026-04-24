#ifndef SDC_H
#define SDC_H

/**
 * @file sdc.h
 * @brief SDC task implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

static constexpr uint32_t SDC_TASK_PERIOD_MS = 5;

void SDC_task_periodic(void);

#endif // SDC_H