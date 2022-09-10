#ifndef _ORION_H_
#define _ORION_H_

#include "can_parse.h"

#include <stdbool.h>

#define MAX_TEMP (300)

bool orionErrors();
void checkTempsPeriodic();
void orion_chargePeriodic();
void orionInit();

#endif