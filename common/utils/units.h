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

// Centralized Value Access Macro
#define UNIT_VAL(x) ((x).value)

// Electrical units
typedef struct { float value; } volts_t;
typedef struct { float value; } amps_t;
typedef struct { float value; } ohms_t;
typedef struct { float value; } watts_t;

static_assert(sizeof(volts_t) == sizeof(float));
static_assert(sizeof(amps_t) == sizeof(float));
static_assert(sizeof(ohms_t) == sizeof(float));
static_assert(sizeof(watts_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline amps_t amps_from(volts_t voltage, ohms_t resistance) {
    return (amps_t){ .value = voltage.value / resistance.value };
}

[[nodiscard, gnu::always_inline]]
static inline volts_t volts_from(amps_t current, ohms_t resistance) {
    return (volts_t){ .value = current.value * resistance.value };
}

[[nodiscard, gnu::always_inline]]
static inline ohms_t ohms_from(volts_t voltage, amps_t current) {
    return (ohms_t){ .value = voltage.value / current.value };
}

[[nodiscard, gnu::always_inline]]
static inline watts_t watts_from(volts_t voltage, amps_t current) {
    return (watts_t){ .value = voltage.value * current.value };
}

// Temperature units
typedef struct { float value; } celsius_t;
typedef struct { float value; } fahrenheit_t;

static_assert(sizeof(celsius_t) == sizeof(float));
static_assert(sizeof(fahrenheit_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline fahrenheit_t fahrenheit_from(celsius_t c) {
    return (fahrenheit_t){ .value = (c.value * 9.0f / 5.0f) + 32.0f };
}

[[nodiscard, gnu::always_inline]]
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

static_assert(sizeof(meters_t) == sizeof(float));
static_assert(sizeof(centimeters_t) == sizeof(float));
static_assert(sizeof(millimeters_t) == sizeof(float));
static_assert(sizeof(inches_t) == sizeof(float));
static_assert(sizeof(feet_t) == sizeof(float));
static_assert(sizeof(miles_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline meters_t meters_from_centimeters(centimeters_t cm) {
    return (meters_t){ .value = cm.value / 100.0f };
}

[[nodiscard, gnu::always_inline]]
static inline meters_t meters_from_millimeters(millimeters_t mm) {
    return (meters_t){ .value = mm.value / 1000.0f };
}

[[nodiscard, gnu::always_inline]]
static inline meters_t meters_from_inches(inches_t in) {
    return (meters_t){ .value = in.value * 0.0254f };
}

[[nodiscard, gnu::always_inline]]
static inline meters_t meters_from_feet(feet_t ft) {
    return (meters_t){ .value = ft.value * 0.3048f };
}

[[nodiscard, gnu::always_inline]]
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
typedef struct { float value; } days_t;

static_assert(sizeof(milliseconds_t) == sizeof(float));
static_assert(sizeof(seconds_t) == sizeof(float));
static_assert(sizeof(minutes_t) == sizeof(float));
static_assert(sizeof(hours_t) == sizeof(float));
static_assert(sizeof(days_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline seconds_t seconds_from_milliseconds(milliseconds_t ms) {
    return (seconds_t){ .value = ms.value / 1000.0f };
}

[[nodiscard, gnu::always_inline]]
static inline seconds_t seconds_from_minutes(minutes_t min) {
    return (seconds_t){ .value = min.value * 60.0f };
}

[[nodiscard, gnu::always_inline]]
static inline seconds_t seconds_from_hours(hours_t hr) {
    return (seconds_t){ .value = hr.value * 3600.0f };
}

[[nodiscard, gnu::always_inline]]
static inline seconds_t seconds_from_days(days_t d) {
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

[[nodiscard, gnu::always_inline]]
static inline radians_t radians_from(degrees_t deg) {
    return (radians_t){ .value = deg.value * PI_F / 180.0f };
}

[[nodiscard, gnu::always_inline]]
static inline degrees_t degrees_from(radians_t rad) {
    return (degrees_t){ .value = rad.value * 180.0f / PI_F };
}

// Mass units
typedef struct { float value; } grams_t;
typedef struct { float value; } kilograms_t;
typedef struct { float value; } pounds_t;

static_assert(sizeof(grams_t) == sizeof(float));
static_assert(sizeof(kilograms_t) == sizeof(float));
static_assert(sizeof(pounds_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline kilograms_t kilograms_from_grams(grams_t g) {
    return (kilograms_t){ .value = g.value / 1000.0f };
}

[[nodiscard, gnu::always_inline]]
static inline kilograms_t kilograms_from_pounds(pounds_t lb) {
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

[[nodiscard, gnu::always_inline]]
static inline pascals_t pascals_from_psi(psi_t p) {
    return (pascals_t){ .value = p.value * 6894.75729f };
}

[[nodiscard, gnu::always_inline]]
static inline pascals_t pascals_from_bar(bar_t b) {
    return (pascals_t){ .value = b.value * 100000.0f };
}

[[nodiscard, gnu::always_inline]]
static inline psi_t psi_from_pascals(pascals_t p) {
    return (psi_t){ .value = p.value / 6894.75729f };
}

[[nodiscard, gnu::always_inline]]
static inline bar_t bar_from_pascals(pascals_t p) {
    return (bar_t){ .value = p.value / 100000.0f };
}

// Velocity units
typedef struct { float value; } mps_t;
typedef struct { float value; } kph_t;
typedef struct { float value; } mph_t;

static_assert(sizeof(mps_t) == sizeof(float));
static_assert(sizeof(kph_t) == sizeof(float));
static_assert(sizeof(mph_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline mps_t mps_from_kph(kph_t k) {
    return (mps_t){ .value = k.value / 3.6f };
}

[[nodiscard, gnu::always_inline]]
static inline mps_t mps_from_mph(mph_t m) {
    return (mps_t){ .value = m.value * 0.44704f };
}

[[nodiscard, gnu::always_inline]]
static inline kph_t kph_from_mps(mps_t m) {
    return (kph_t){ .value = m.value * 3.6f };
}

[[nodiscard, gnu::always_inline]]
static inline mph_t mph_from_mps(mps_t m) {
    return (mph_t){ .value = m.value / 0.44704f };
}

// Acceleration units
typedef struct { float value; } mps2_t;
typedef struct { float value; } g_force_t;

static_assert(sizeof(mps2_t) == sizeof(float));
static_assert(sizeof(g_force_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline mps2_t mps2_from_g(g_force_t g) {
    return (mps2_t){ .value = g.value * 9.80665f };
}

[[nodiscard, gnu::always_inline]]
static inline g_force_t g_from_mps2(mps2_t m) {
    return (g_force_t){ .value = m.value / 9.80665f };
}

// Torque units
typedef struct { float value; } nm_t;
typedef struct { float value; } lbft_t;

static_assert(sizeof(nm_t) == sizeof(float));
static_assert(sizeof(lbft_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline nm_t nm_from_lbft(lbft_t l) {
    return (nm_t){ .value = l.value * 1.355818f };
}

[[nodiscard, gnu::always_inline]]
static inline lbft_t lbft_from_nm(nm_t n) {
    return (lbft_t){ .value = n.value / 1.355818f };
}

// Energy units
typedef struct { float value; } joules_t;
typedef struct { float value; } wh_t;

static_assert(sizeof(joules_t) == sizeof(float));
static_assert(sizeof(wh_t) == sizeof(float));

[[nodiscard, gnu::always_inline]]
static inline joules_t joules_from_wh(wh_t w) {
    return (joules_t){ .value = w.value * 3600.0f };
}

[[nodiscard, gnu::always_inline]]
static inline wh_t wh_from_joules(joules_t j) {
    return (wh_t){ .value = j.value / 3600.0f };
}

#endif // UNITS_H
