#ifndef _ORION_H_
#define _ORION_H_

#include "can_parse.h"

#include <stdbool.h>

#define MAX_TEMP (59)

bool orionErrors();
void orionCheckTempsPeriodic();
void orion_chargePeriodic();
void orionInit();

#endif