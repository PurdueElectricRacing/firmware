#ifndef __DAQ_RTC_CONFIG_H__
#define __DAQ_RTC_CONFIG_H__
#include <stdint.h>

typedef enum { 
    RTC_CONFIG_STATE_IDLE     = 0,
    RTC_CONFIG_STATE_PENDING  = 1,
    RTC_CONFIG_STATE_COMPLETE = 2,
} rtc_config_state_t;

typedef struct {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    } gps_time_t ;

void rtc_check_periodic (void);
void rtc_config_shutdown (void);

#endif