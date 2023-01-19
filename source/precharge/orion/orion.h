#ifndef _ORION_H_
#define _ORION_H_

#include "can_parse.h"

#include <stdbool.h>

#define MAX_TEMP (550)
#define MAX_VOLT (336) // 4.2 * 80

bool orionErrors();
void orionCheckTempsPeriodic();
void orionChargePeriodic();
void orionInit();

#endif