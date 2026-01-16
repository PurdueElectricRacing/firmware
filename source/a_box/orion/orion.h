#ifndef _ORION_H_
#define _ORION_H_

#include <stdbool.h>

#include "A_BOX.h"

#define MAX_TEMP (550)
#define MAX_VOLT (598) // Max Pack voltage per ESF is 598 volts.

#define DEFAULT_CHARGE_VOLTAGE_REQUEST 590
#define DEFAULT_CHARGE_CURRENT_REQUEST 2

extern uint8_t charge_request_user;
extern uint16_t user_charge_current_request;
extern uint16_t user_charge_voltage_request;

bool orionErrors();
void orionCheckTempsPeriodic();
void orionChargePeriodic();
void orionInit();

#endif
