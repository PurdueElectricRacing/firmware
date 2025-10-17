#ifndef __DAQ_RTC_CONFIG_H__
#define __DAQ_RTC_CONFIG_H__
#include <stdint.h>

typedef enum { 
    RTC_CONFIG_STATE_IDLE     = 0,
    RTC_CONFIG_STATE_PENDING  = 1,
    RTC_CONFIG_STATE_COMPLETE = 2,
} rtc_config_state_t;

void rtc_check_periodic (void);
void rtc_config_shutdown (void);

#endif