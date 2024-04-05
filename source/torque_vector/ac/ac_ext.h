#ifndef _AC_EXT_H_
#define _AC_EXT_H_
#define NUM_ELEM_ACC_CALIBRATION 151

typedef struct  {
    float ax[151];
    float ay[151];
    float az[151];
} vec_accumulator;
#endif