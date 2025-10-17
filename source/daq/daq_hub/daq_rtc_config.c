#include "daq_rtc_config.h"

#include "common/phal/gpio.h"
#include "common/phal/rtc.h"
#include "daq_hub.h"
#include "main.h"
#include "math.h"

static void parse_gps_time(const timestamped_frame_t* frame, gps_time_t* out);
static uint8_t get_weekday(uint8_t d, uint8_t m, uint16_t Y);
static void GPS_time_to_BCD_RTC(RTC_timestamp_t *gps_rtc_time, gps_time_t gps_time);

void rtc_check_periodic (void) {
    timestamped_frame_t* buf;
    RTC_timestamp_t gps_rtc_time;
    gps_time_t gps_time;
    uint32_t consecutive_items;
    
    switch(dh.rtc_config_state) {
        case RTC_CONFIG_STATE_IDLE:
            bActivateTail(&b_rx_can, RX_TAIL_CAN_RX);
            dh.rtc_config_state = RTC_CONFIG_STATE_PENDING;
            // Letting the statement fall through is probably fine here?
        case RTC_CONFIG_STATE_PENDING:
            if (bGetItemCount(&b_rx_can, RX_TAIL_CAN_RX)) {
                if ((bGetTailForRead(&b_rx_can, RX_TAIL_CAN_RX, (void**)&buf, &consecutive_items) == 0)) {
                    if (buf->msg_id == ID_GPS_TIME) {
                        parse_gps_time(buf, &gps_time);
                        GPS_time_to_BCD_RTC(&gps_rtc_time, gps_time);
                        if (!PHAL_configureRTC(&gps_rtc_time, true)) {
                            // Successful reintialization 
                            dh.rtc_config_state = RTC_CONFIG_STATE_COMPLETE;
                        }
                    }
                    else {
                        bCommitRead(&b_rx_can, RX_TAIL_CAN_RX, consecutive_items);
                    }
                } else {
                    dh.can1_rx_overflow++; // currently also incremented by the tcp eth tail people which seems incorrect??
                }
            }
            break;
        case RTC_CONFIG_STATE_COMPLETE:
            osThreadTerminate(osThreadGetId())
            break;        
    }
}

static void GPS_time_to_BCD_RTC(RTC_timestamp_t *gps_rtc_time, gps_time_t gps_time)
{
    gps_rtc_time->date.year_bcd = RTC_CONV_TO_BCD(gps_time.year);
    gps_rtc_time->date.month_bcd = RTC_CONV_TO_BCD(gps_time.month);
    gps_rtc_time->date.day_bcd = RTC_CONV_TO_BCD(gps_time.day);

    gps_rtc_time->time.hours_bcd = RTC_CONV_TO_BCD(gps_time.hour);
    gps_rtc_time->time.minutes_bcd = RTC_CONV_TO_BCD(gps_time.minute);
    gps_rtc_time->time.seconds_bcd = RTC_CONV_TO_BCD(gps_time.second);

    gps_rtc_time->date.weekday = RTC_CONV_TO_BCD(get_weekday(gps_time.day, gps_time.month, gps_time.year));
    gps_rtc_time->time.time_format = RTC_FORMAT_24_HOUR;
}

static void parse_gps_time(const timestamped_frame_t* frame, gps_time_t* out) {
    if (frame->msg_id != 2285897728) return; // Check correct message

    out->year        = frame->data[0];
    out->month       = frame->data[1];
    out->day         = frame->data[2];
    out->hour        = frame->data[3];
    out->minute      = frame->data[4];
    out->second      = frame->data[5];
}

static uint8_t get_weekday (uint8_t d, uint8_t m, uint16_t Y) {
    /* Calculation for the weekday which in theory works */
    uint16_t F;
    uint8_t weekday;

    if (m < 3)
    {
      m += 12;
      Y -= 1;
    }

    F = d + floor(13.00 * (m + 1.00) / 5.00) + Y + floor(Y / 4.00) - floor(Y / 100.00) + floor(Y / 400.00);
    weekday = ((F + 5) % 7) + 1;
    return weekday;
}

void rtc_config_shutdown (void) {
    bDeactivateTail(&b_rx_can, RX_TAIL_CAN_RX);
}
