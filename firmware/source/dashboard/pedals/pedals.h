#ifndef PEDALS_H
#define PEDALS_H

/**
 * @file pedals.c
 * @brief Pedal processing logic
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#include <stdint.h>
#include "can_library/generated/DASHBOARD.h"

extern volatile pedals_data_t pedal_values;

void pedals_periodic(void);

#endif // PEDALS_H
