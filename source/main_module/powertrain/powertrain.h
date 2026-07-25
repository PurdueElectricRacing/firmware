#ifndef POWERTRAIN_H
#define POWERTRAIN_H

/**
 * @file powertrain.h
 * @brief Powertrain implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include "common/amk/amk.h"

typedef struct {
    int16_t front_right;
    int16_t front_left;
    int16_t rear_left;
    int16_t rear_right;
} torque_request_t;

typedef struct {
    AMK_t front_right; // INVA
    AMK_t front_left;  // INVB
    AMK_t rear_left;   // INVC
    AMK_t rear_right;  // INVD
    torque_request_t torque_request;
} powertrain_t;

extern powertrain_t g_powertrain;

void powertrain_init(void);
void powertrain_periodic(void);
void powertrain_flush(void);

#endif // POWERTRAIN_H