#ifndef APPS_H_
#define APPS_H_
#include "stdbool.h"

void apps_Init();

void apps_Tick(float throttle_pos, float brake_pos);
bool apps_IsAPPSFaulted();

#endif