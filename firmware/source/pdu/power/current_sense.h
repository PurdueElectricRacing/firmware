/**
 * @file current_sense.h
 * @brief current sensing
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef CURRENT_SENSE_H
#define CURRENT_SENSE_H

#include <stdint.h>

static constexpr uint32_t CURRENT_SENSE_TASK_PERIOD_MS = 4;
void current_sense_periodic(void);

#endif // CURRENT_SENSE_H