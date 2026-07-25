#ifndef TORQUE_CONTROLLER_H
#define TORQUE_CONTROLLER_H

/**
 * @file torque_controller.h
 * @brief Vehicle torque request calculation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "powertrain.h"

torque_request_t torque_controller_get_request(void);

#endif // TORQUE_CONTROLLER_H
