#include "cooling.h"

// error checks
// if pump on, ensure flow
// over temp

// Drivetrain
// Therm 1, 2
// Pump Ctrl <- N/A
// Pump flow adjust
// Flow rate PWM
// Fan ctrl

// Battery
// therm in
// therm out
// Pump Ctrl <- N/A
// Pump flow adjust
// Flow rate PWM
// Fan ctrl

// Thermistors just for logging
// Get temps from BMS and motor controllers
// Fans and pump on at same time together
// Battery - on if temp high, after TSMS/precharge, 
//           stay on until GLVMS turned off
// essentially on after car starts, or if hot
// DT - on if motor temp high

bool initCooling()
{

}

void coolingPeriodic()
{


}