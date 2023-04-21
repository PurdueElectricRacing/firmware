#include "MC_PL0.h"
#include "plettenberg.h"
#include "testbench.h"

// Calibration
#define VOLTAGE_CALIBRATION 0.1
#define SPEED_CALIBRATION 6.63 / (360.0 * 0.2286)
#define SHAFT_SPEED_CALIBRATION 0.104719755
#define SPEED2_CALIBRATION 0.01
#define TORQUE_CALIBRATION 25.0 / 4095.0
#define CURRENT_CALIBRATION 0.1
#define TEMPERATURE_CALIBRATION 1.0

// Clamping
#define ABS_MIN_TORQUE -0.01
#define ABS_MAX_TORQUE 25.0

#define MIN_OMEGA 0.01
#define MAX_OMEGA 1070.0

#define MIN_MOTOR_TEMPERATURE 0.0
#define MAX_MOTOR_TEMPERATURE 100.0

#define MIN_VOLTAGE 50.0
#define MAX_VOLTAGE 340.0

#define MIN_BATTERY_POWER 80000.0
#define MAX_BATTERY_POWER 80000.0

#define MIN_MOTOR_CURRENT 0.0
#define MAX_MOTOR_CURRENT 75.0

void MC_PL_pp(ExtU* rtU, motor_t* motor_left, micro_t* micro);