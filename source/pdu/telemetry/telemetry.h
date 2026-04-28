#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

static constexpr uint32_t TELEMETRY_10HZ_PERIOD_MS = 100;
void telemetry_10hz(void);

#endif // TELEMETRY_H
