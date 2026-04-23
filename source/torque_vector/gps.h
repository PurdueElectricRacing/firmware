#ifndef GPS_H
#define GPS_H

#include <stdint.h>

static constexpr uint32_t GPS_THREAD_PERIOD_MS = 100;

void gps_periodic(void);

#endif // GPS_H