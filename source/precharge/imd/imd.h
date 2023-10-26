/**
* @file imd.h
* @author Michael Gliane (mgliane@purdue.edu)
* @brief
* @version 0.1
* @date 2023-9-30
* 
* @copyright Copyright (c) 2023
*
*/

#ifndef _PHAL_IMD_H
#define _PHAL_IMD_H

#include <stdbool.h>

/**
* @brief                Enabling and configuring the proper GPIO pins (GPIOB pins 5-7)
*                       to read IMD channels
*/
void setup_IMDReading();

/**
* @brief                Enabling and configuring Timer 3 and 4 for reading IMD
*                       frequency.
*/
void PHAL_setupTIMClk();

/**
* @brief                Checks if the IMD's OKhs signal is high, signaling that there
*                       is no faulty resistance
*/
bool checkIMD_signal_OKhs();

/**
* @brief                Checks and returns the frequency of the Mhs
*/
int checkIMD_signal_Mhs();

/**
* @brief                Checks and returns the frequency of the Mls
*/
int checkIMD_signal_Mls();

#endif //_PHAL_IMD_H