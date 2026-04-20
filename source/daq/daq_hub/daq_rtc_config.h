#ifndef DAQ_RTC_CONFIG_H
#define DAQ_RTC_CONFIG_H

#include "timestamped_frame.h"
#include "common/phal/rtc.h"

typedef enum {
    RTC_SYNC_PENDING   = 0,
    RTC_SYNC_COMPLETE  = 1,
} rtc_config_state_t;

void rtc_config_cb(timestamped_frame_t*);

#endif // DAQ_RTC_CONFIG_H