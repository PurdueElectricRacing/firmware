#ifndef _FLOW_RATE_H
#define _FLOW_RATE_H

#include <stdbool.h>
#include "stm32f407xx.h"
#include "main.h"

#define MAX_TIME_BETWEEN_CC_MS 15

bool flowRateInit();
uint32_t getFlowRate1();
uint32_t getFlowRate2();

#endif // _FLOW_RATE_H
