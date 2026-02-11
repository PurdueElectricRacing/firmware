/**
 * @file driveline_config.h
 * @brief "Driveline" configuration definitions
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

// ! IS_FRONT_DRIVELINE and IS_REAR_DRIVELINE is defined by the CMake, do not define it manually here

// Check to prevent both being defined at the same time
#if defined(IS_FRONT_DRIVELINE) && defined(IS_REAR_DRIVELINE)
#error "IS_FRONT_DRIVELINE and IS_REAR_DRIVELINE cannot both be defined"
#endif

// Check to prevent neither being defined
#if !defined(IS_FRONT_DRIVELINE) && !defined(IS_REAR_DRIVELINE)
#error "Either IS_FRONT_DRIVELINE or IS_REAR_DRIVELINE must be defined"
#endif

#ifdef IS_FRONT_DRIVELINE
#define SEND_SHOCKPOTS CAN_SEND_front_shockpots
// Shock Pot Calibration
#define POT_TOTAL_RES 3000
#define POT_MAX_RES   3300
#define POT_MIN_RES   300
static constexpr float POT_VOLT_MAX_L = 11.0f;
static constexpr float POT_VOLT_MIN_L = 4060.0f;
static constexpr float POT_VOLT_MAX_R = 11.0f;
static constexpr float POT_VOLT_MIN_R = 4092.0f;
#define POT_MAX_DIST     75
#define POT_DIST_DROOP_L 56
#define POT_DIST_DROOP_R 55

// others here
#endif

#ifdef IS_REAR_DRIVELINE
#define SEND_SHOCKPOTS CAN_SEND_rear_shockpots
// Shock Pot Calibration
#define POT_TOTAL_RES 3000
#define POT_MAX_RES   3300
#define POT_MIN_RES   300
static constexpr float POT_VOLT_MAX_L = 11.0f;
static constexpr float POT_VOLT_MIN_L = 4060.0f;
static constexpr float POT_VOLT_MAX_R = 11.0f;
static constexpr float POT_VOLT_MIN_R = 4092.0f;
#define POT_MAX_DIST     75
#define POT_DIST_DROOP_L 56
#define POT_DIST_DROOP_R 55

// others here
#endif