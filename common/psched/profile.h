#ifndef _PROFILE_H_
#define _PROFILE_H_

// Uncomment depending on MCU type
#if !defined(L4) || !defined(F4)
    #define L4
    // #define F4
#endif

// Includes
#if defined(L4)
    #include "stm32l4xx.h"
    #include "system_stm32l4xx.h"
#elif defined(F4)
    #include "system_stm32f4xx.h"
#endif

#include "stdbool.h"

#endif