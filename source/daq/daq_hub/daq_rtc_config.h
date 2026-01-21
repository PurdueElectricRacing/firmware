#include "common/phal/can.h"
#include "common/phal/rtc.h"

void rtc_config_cb(CanMsgTypeDef_t*);
static void parse_gps_time(const CanMsgTypeDef_t* msg, RTC_timestamp_t* gps_rtc_time);