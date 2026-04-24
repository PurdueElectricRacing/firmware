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

typedef struct {
    uint16_t throttle;
    uint16_t brake;
} pedal_values_t;

extern pedal_values_t pedal_values;

/* Function Prototypes */
void pedals_periodic(void);

#endif // PEDALS_H
