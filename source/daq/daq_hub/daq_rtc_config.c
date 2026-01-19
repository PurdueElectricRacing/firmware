#include <string.h>
#include <math.h>

#include "common/phal/can.h"
#include "common/phal/rtc.h"
#include "common/phal/gpio.h"
#include "daq_can.h"
#include "daq_hub.h"
#include "main.h"

void rtc_config_cb(CanMsgTypeDef_t *msg) {
   if (daq_hub.rtc_config_state == RTC_SYNC_COMPLETE) return;

   RTC_timestamp_t gps_rtc_time;
   parse_gps_time(msg, &gps_rtc_time);
   
   if (PHAL_configureRTC(&gps_rtc_time, true)) {
      // Successful reintialization
      daq_hub.rtc_config_state = RTC_SYNC_COMPLETE;
   } else {
      HardFault_Handler();
   }
}

static void parse_gps_time(const CanMsgTypeDef_t* msg, RTC_timestamp_t* gps_rtc_time) {
   uint8_t year  = msg->Data[0];
   uint8_t month = msg->Data[1];
   uint8_t day   = msg->Data[2];
   uint8_t hour  = msg->Data[3];
   uint8_t min   = msg->Data[4];
   uint8_t sec   = msg->Data[5];

   gps_rtc_time->date.year_bcd = RTC_CONV_TO_BCD(year);
   gps_rtc_time->date.day_bcd = RTC_CONV_TO_BCD(day);
   gps_rtc_time->date.weekday   = RTC_WEEKDAY_UNKNOWN;

   switch (month) {
        case 1:  gps_rtc_time->date.month_bcd = RTC_MONTH_JANUARY;   break;
        case 2:  gps_rtc_time->date.month_bcd = RTC_MONTH_FEBRUARY;  break;
        case 3:  gps_rtc_time->date.month_bcd = RTC_MONTH_MARCH;     break;
        case 4:  gps_rtc_time->date.month_bcd = RTC_MONTH_APRIL;     break;
        case 5:  gps_rtc_time->date.month_bcd = RTC_MONTH_MAY;       break;
        case 6:  gps_rtc_time->date.month_bcd = RTC_MONTH_JUNE;      break;
        case 7:  gps_rtc_time->date.month_bcd = RTC_MONTH_JULY;      break;
        case 8:  gps_rtc_time->date.month_bcd = RTC_MONTH_AUGUST;    break;
        case 9:  gps_rtc_time->date.month_bcd = RTC_MONTH_SEPTEMBER; break;
        case 10: gps_rtc_time->date.month_bcd = RTC_MONTH_OCTOBER;   break;
        case 11: gps_rtc_time->date.month_bcd = RTC_MONTH_NOVEMBER;  break;
        case 12: gps_rtc_time->date.month_bcd = RTC_MONTH_DECEMBER;  break;
        default: gps_rtc_time->date.month_bcd = RTC_MONTH_UNKNOWN;   break;
    }

    gps_rtc_time->time.hours_bcd   = RTC_CONV_TO_BCD(hour);
    gps_rtc_time->time.minutes_bcd = RTC_CONV_TO_BCD(min);
    gps_rtc_time->time.seconds_bcd = RTC_CONV_TO_BCD(sec);
    gps_rtc_time->time.time_format = RTC_FORMAT_24_HOUR;
}