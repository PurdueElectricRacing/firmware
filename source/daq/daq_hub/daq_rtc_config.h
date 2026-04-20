#include "spmc.h"
#include "common/phal/rtc.h"

typedef enum {
    RTC_SYNC_PENDING   = 0,
    RTC_SYNC_COMPLETE  = 1,
} rtc_config_state_t;

void rtc_config_cb(timestamped_frame_t*);