#ifndef _ORION_H_
#define _ORION_H_

#include "can_parse.h"

#include <stdbool.h>

#define MAX_TEMP (550)
#define MAX_VOLT (319) // 4.2 * 76

extern uint8_t charge_request_user;
extern uint16_t user_charge_current_request;
extern uint16_t user_charge_voltage_request;

bool orionErrors();
void orionCheckTempsPeriodic();
void orionChargePeriodic();
void orionInit();

#endif