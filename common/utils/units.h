#ifndef UNITS_H
#define UNITS_H

/**
 * @file units.h
 * @brief SI and Imperial unit defs and conversion functions.
 *
 * Provides simple type-safe wrappers around physical unit conversions.
 * All types under the hood are structs containing a single float.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

// Electrical units
typedef struct { float value; } volts_t;
typedef struct { float value; } amps_t;
typedef struct { float value; } ohms_t;
typedef struct { float value; } watts_t;

static inline amps_t amps_from(volts_t voltage, ohms_t resistance) {
    return (amps_t){ .value = voltage.value / resistance.value };
}

static inline volts_t volts_from(amps_t current, ohms_t resistance) {
    return (volts_t){ .value = current.value * resistance.value };
}

static inline ohms_t ohms_from(amps_t current, volts_t voltage) {
    return (ohms_t){ .value = voltage.value / current.value };
}

static inline watts_t watts_from(volts_t voltage, amps_t current) {
    return (watts_t){ .value = voltage.value * current.value };
}

// Temperature units
typedef struct { float value; } celsius_t;
typedef struct { float value; } fahrenheit_t;

static inline fahrenheit_t fahrenheit_from(celsius_t c) {
    return (fahrenheit_t){ .value = (c.value * 9.0f / 5.0f) + 32.0f };
}

static inline celsius_t celsius_from(fahrenheit_t f) {
    return (celsius_t){ .value = (f.value - 32.0f) * 5.0f / 9.0f };
}

// Distance units
typedef struct { float value; } meters_t;
typedef struct { float value; } centimeters_t;
typedef struct { float value; } millimeters_t;
typedef struct { float value; } inches_t;
typedef struct { float value; } feet_t;
typedef struct { float value; } miles_t;

static inline meters_t meters_from_centimeters(centimeters_t cm) {
    return (meters_t){ .value = cm.value / 100.0f };
}

static inline meters_t meters_from_millimeters(millimeters_t mm) {
    return (meters_t){ .value = mm.value / 1000.0f };
}

static inline meters_t meters_from_inches(inches_t in) {
    return (meters_t){ .value = in.value * 0.0254f };
}

static inline meters_t meters_from_feet(feet_t ft) {
    return (meters_t){ .value = ft.value * 0.3048f };
}

static inline meters_t meters_from_miles(miles_t mi) {
    return (meters_t){ .value = mi.value * 1609.34f };
}

#define meters_from(x) _Generic((x), \
    centimeters_t: meters_from_centimeters, \
    millimeters_t: meters_from_millimeters, \
    inches_t: meters_from_inches, \
    feet_t: meters_from_feet, \
    miles_t: meters_from_miles \
)(x)

// Time units
typedef struct { float value; } milliseconds_t;
typedef struct { float value; } seconds_t;
typedef struct { float value; } minutes_t;
typedef struct { float value; } hours_t;

static inline seconds_t seconds_from_milliseconds(milliseconds_t ms) {
    return (seconds_t){ .value = ms.value / 1000.0f };
}

static inline seconds_t seconds_from_minutes(minutes_t min) {
    return (seconds_t){ .value = min.value * 60.0f };
}

static inline seconds_t seconds_from_hours(hours_t hr) {
    return (seconds_t){ .value = hr.value * 3600.0f };
}

#define seconds_from(x) _Generic((x), \
    milliseconds_t: seconds_from_milliseconds, \
    minutes_t: seconds_from_minutes, \
    hours_t: seconds_from_hours \
)(x)

#endif // UNITS_H