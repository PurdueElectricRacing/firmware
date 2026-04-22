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

// Global Constants
constexpr float PI_F = 3.1415926535f;

// Function modifiers for unit conversion functions
#define UNIT_FUNC_MODIFIERS [[nodiscard, gnu::always_inline]] static inline

// Temperature units
typedef struct { float value; } celsius_t;
typedef struct { float value; } fahrenheit_t;

static_assert(sizeof(celsius_t) == sizeof(float));
static_assert(sizeof(fahrenheit_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
fahrenheit_t fahrenheit_from(celsius_t c) {
    return (fahrenheit_t){ .value = (c.value * 9.0f / 5.0f) + 32.0f };
}

UNIT_FUNC_MODIFIERS
celsius_t celsius_from(fahrenheit_t f) {
    return (celsius_t){ .value = (f.value - 32.0f) * 5.0f / 9.0f };
}

// Distance units
typedef struct { float value; } meters_t;
typedef struct { float value; } centimeters_t;
typedef struct { float value; } millimeters_t;
typedef struct { float value; } inches_t;
typedef struct { float value; } feet_t;
typedef struct { float value; } miles_t;

static_assert(sizeof(meters_t) == sizeof(float));
static_assert(sizeof(centimeters_t) == sizeof(float));
static_assert(sizeof(millimeters_t) == sizeof(float));
static_assert(sizeof(inches_t) == sizeof(float));
static_assert(sizeof(feet_t) == sizeof(float));
static_assert(sizeof(miles_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
meters_t meters_from_centimeters(centimeters_t cm) {
    return (meters_t){ .value = cm.value / 100.0f };
}

UNIT_FUNC_MODIFIERS
meters_t meters_from_millimeters(millimeters_t mm) {
    return (meters_t){ .value = mm.value / 1000.0f };
}

UNIT_FUNC_MODIFIERS
meters_t meters_from_inches(inches_t in) {
    return (meters_t){ .value = in.value * 0.0254f };
}

UNIT_FUNC_MODIFIERS
meters_t meters_from_feet(feet_t ft) {
    return (meters_t){ .value = ft.value * 0.3048f };
}

UNIT_FUNC_MODIFIERS
meters_t meters_from_miles(miles_t mi) {
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
typedef struct { float value; } days_t;

static_assert(sizeof(milliseconds_t) == sizeof(float));
static_assert(sizeof(seconds_t) == sizeof(float));
static_assert(sizeof(minutes_t) == sizeof(float));
static_assert(sizeof(hours_t) == sizeof(float));
static_assert(sizeof(days_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
seconds_t seconds_from_milliseconds(milliseconds_t ms) {
    return (seconds_t){ .value = ms.value / 1000.0f };
}

UNIT_FUNC_MODIFIERS
seconds_t seconds_from_minutes(minutes_t min) {
    return (seconds_t){ .value = min.value * 60.0f };
}

UNIT_FUNC_MODIFIERS
seconds_t seconds_from_hours(hours_t hr) {
    return (seconds_t){ .value = hr.value * 3600.0f };
}

UNIT_FUNC_MODIFIERS
seconds_t seconds_from_days(days_t d) {
    return (seconds_t){ .value = d.value * 86400.0f };
}

#define seconds_from(x) _Generic((x), \
    milliseconds_t: seconds_from_milliseconds, \
    minutes_t: seconds_from_minutes, \
    hours_t: seconds_from_hours, \
    days_t: seconds_from_days \
)(x)

// Angular units
typedef struct { float value; } degrees_t;
typedef struct { float value; } radians_t;

static_assert(sizeof(degrees_t) == sizeof(float));
static_assert(sizeof(radians_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
radians_t radians_from(degrees_t deg) {
    return (radians_t){ .value = deg.value * PI_F / 180.0f };
}

UNIT_FUNC_MODIFIERS
degrees_t degrees_from(radians_t rad) {
    return (degrees_t){ .value = rad.value * 180.0f / PI_F };
}

// Mass units
typedef struct { float value; } grams_t;
typedef struct { float value; } kilograms_t;
typedef struct { float value; } pounds_t;

static_assert(sizeof(grams_t) == sizeof(float));
static_assert(sizeof(kilograms_t) == sizeof(float));
static_assert(sizeof(pounds_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
kilograms_t kilograms_from_grams(grams_t g) {
    return (kilograms_t){ .value = g.value / 1000.0f };
}

UNIT_FUNC_MODIFIERS
kilograms_t kilograms_from_pounds(pounds_t lb) {
    return (kilograms_t){ .value = lb.value * 0.453592f };
}

#define kilograms_from(x) _Generic((x), \
    grams_t: kilograms_from_grams, \
    pounds_t: kilograms_from_pounds \
)(x)

// Pressure units
typedef struct { float value; } pascals_t;
typedef struct { float value; } psi_t;
typedef struct { float value; } bar_t;

static_assert(sizeof(pascals_t) == sizeof(float));
static_assert(sizeof(psi_t) == sizeof(float));
static_assert(sizeof(bar_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
pascals_t pascals_from_psi(psi_t p) {
    return (pascals_t){ .value = p.value * 6894.75729f };
}

UNIT_FUNC_MODIFIERS
pascals_t pascals_from_bar(bar_t b) {
    return (pascals_t){ .value = b.value * 100000.0f };
}

UNIT_FUNC_MODIFIERS
psi_t psi_from_pascals(pascals_t p) {
    return (psi_t){ .value = p.value / 6894.75729f };
}

UNIT_FUNC_MODIFIERS
bar_t bar_from_pascals(pascals_t p) {
    return (bar_t){ .value = p.value / 100000.0f };
}

// Velocity units
typedef struct { float value; } mps_t;
typedef struct { float value; } kph_t;
typedef struct { float value; } mph_t;

static_assert(sizeof(mps_t) == sizeof(float));
static_assert(sizeof(kph_t) == sizeof(float));
static_assert(sizeof(mph_t) == sizeof(float));

UNIT_FUNC_MODIFIERS
mps_t mps_from_kph(kph_t k) {
    return (mps_t){ .value = k.value / 3.6f };
}

UNIT_FUNC_MODIFIERS
mps_t mps_from_mph(mph_t m) {
    return (mps_t){ .value = m.value * 0.44704f };
}

UNIT_FUNC_MODIFIERS
kph_t kph_from_mps(mps_t m) {
    return (kph_t){ .value = m.value * 3.6f };
}

UNIT_FUNC_MODIFIERS
mph_t mph_from_mps(mps_t m) {
    return (mph_t){ .value = m.value / 0.44704f };
}

#endif // UNITS_H
