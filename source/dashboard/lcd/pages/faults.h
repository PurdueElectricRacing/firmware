#ifndef FAULTS_H
#define FAULTS_H

/**
 * @file faults.h
 * @brief Faults page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#define FAULT_STRING "fault"

// Object names for Fault View page
#define FAULT1_BUTTON     "ERROR1"
#define FAULT2_BUTTON     "ERROR2"
#define FAULT3_BUTTON     "ERROR3"
#define FAULT4_BUTTON     "ERROR4"
#define FAULT5_BUTTON     "ERROR5"
#define FAULT6_BUTTON     "ERROR6"
#define FAULT7_BUTTON     "ERROR7"
#define FAULT8_BUTTON     "ERROR8"
#define FAULT1_TXT        "ERROR1"
#define FAULT2_TXT        "ERROR2"
#define FAULT3_TXT        "ERROR3"
#define FAULT4_TXT        "ERROR4"
#define FAULT5_TXT        "ERROR5"
#define FAULT6_TXT        "ERROR6"
#define FAULT7_TXT        "ERROR7"
#define FAULT8_TXT        "ERROR8"
#define FAULT_NONE_STRING "NONE\0"

void faults_update(void);
void faults_move_up(void);
void faults_move_down(void);
void faults_select(void);
void faults_telemetry_update(void);

#endif // FAULTS_H