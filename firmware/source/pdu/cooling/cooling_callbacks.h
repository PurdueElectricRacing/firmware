/**
 * @file cooling_callbacks.h
 * @brief callback functions for bang-bang control
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */
#ifndef COOLING_CALLBACKS_H
#define COOLING_CALLBACKS_H

void powertrain_pumps_on(void);
void powertrain_pumps_off(void);
void hx_fan_on(void);
void hx_fan_off(void);
void battery_fans_on(void);
void battery_fans_off(void);

#endif // COOLING_CALLBACKS_H