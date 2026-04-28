/**
 * @file rtc_sync.c
 * @brief Syncrhonization of the RTC peripheral with the GPS UTC time
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 */

#include "rtc_sync.h"
#include "common/phal/rtc.h"
#include "main.h"

static constexpr uint32_t RTC_SYNC_PERIOD_MS = 30 * 1000;
DEFINE_QUEUE(gps_time_queue, timestamped_frame_t, 1);
volatile uint32_t last_RTC_sync_time = 0;

static RTC_timestamp_t RTC_from_gps(timestamped_frame_t gps_time) {
    RTC_timestamp_t RTC_time = {0};

    uint8_t year  = gps_time.payload & 0xFF;
    uint8_t month = (gps_time.payload >> 8) & 0xFF;
    uint8_t day   = (gps_time.payload >> 16) & 0xFF;
    uint8_t hour  = (gps_time.payload >> 24) & 0xFF;
    uint8_t min   = (gps_time.payload >> 32) & 0xFF;
    uint8_t sec   = (gps_time.payload >> 40) & 0xFF;

    RTC_time.date.year_bcd = RTC_CONV_TO_BCD(year);
    RTC_time.date.day_bcd  = RTC_CONV_TO_BCD(day);
    RTC_time.date.weekday  = RTC_WEEKDAY_UNKNOWN;

    switch (month) {
        case 1:  RTC_time.date.month_bcd = RTC_MONTH_JANUARY;   break;
        case 2:  RTC_time.date.month_bcd = RTC_MONTH_FEBRUARY;  break;
        case 3:  RTC_time.date.month_bcd = RTC_MONTH_MARCH;     break;
        case 4:  RTC_time.date.month_bcd = RTC_MONTH_APRIL;     break;
        case 5:  RTC_time.date.month_bcd = RTC_MONTH_MAY;       break;
        case 6:  RTC_time.date.month_bcd = RTC_MONTH_JUNE;      break;
        case 7:  RTC_time.date.month_bcd = RTC_MONTH_JULY;      break;
        case 8:  RTC_time.date.month_bcd = RTC_MONTH_AUGUST;    break;
        case 9:  RTC_time.date.month_bcd = RTC_MONTH_SEPTEMBER; break;
        case 10: RTC_time.date.month_bcd = RTC_MONTH_OCTOBER;   break;
        case 11: RTC_time.date.month_bcd = RTC_MONTH_NOVEMBER;  break;
        case 12: RTC_time.date.month_bcd = RTC_MONTH_DECEMBER;  break;
        default: RTC_time.date.month_bcd = RTC_MONTH_UNKNOWN;   break;
    }

    RTC_time.time.hours_bcd   = RTC_CONV_TO_BCD(hour);
    RTC_time.time.minutes_bcd = RTC_CONV_TO_BCD(min);
    RTC_time.time.seconds_bcd = RTC_CONV_TO_BCD(sec);
    RTC_time.time.time_format = RTC_FORMAT_24_HOUR;

    return RTC_time;
}

void RTC_sync_init(void) {
    INIT_QUEUE(gps_time_queue, timestamped_frame_t, 1);
}

void RTC_sync_thread(void) {
    timestamped_frame_t gps_time;

    if (xQueueReceive(gps_time_queue, &gps_time, portMAX_DELAY) == pdPASS) {
        uint32_t now = xTaskGetTickCount();
        if ((last_RTC_sync_time != 0) && // allow the first sync immediately
            (now - last_RTC_sync_time) < RTC_SYNC_PERIOD_MS) {
            return;
        }

        RTC_timestamp_t gps_rtc_time = RTC_from_gps(gps_time);
        
        if (PHAL_configureRTC(&gps_rtc_time, true)) {
            last_RTC_sync_time = now;
        } else {
            HardFault_Handler();
        }
    }
}