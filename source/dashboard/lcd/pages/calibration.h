#ifndef CALIBRATION_H
#define CALIBRATION_H

/**
 * @file calibration.h
 * @brief Calibration page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

// Nextion object names
#define CALIBRATION_THROTTLE1  "thr1"
#define CALIBRATION_THROTTLE2  "thr2"
#define CALIBRATION_BRAKE1     "brk1"
#define CALIBRATION_BRAKE2     "brk2"
#define CALIBRATION_BRAKE_PRS1 "brkprs1"
#define CALIBRATION_BRAKE_PRS2 "brkprs2"
#define CALIBRATION_STATUS     "status"

void calibration_telemetry_update();

#endif // CALIBRATION_H