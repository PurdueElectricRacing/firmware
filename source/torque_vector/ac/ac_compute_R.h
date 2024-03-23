#ifndef AC_COMPUTE_R_H
#define AC_COMPUTE_R_H

#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void ac_compute_R(const float ax[151], const float ay[151],
                         const float az[151], float R[9]);

#ifdef __cplusplus
}
#endif

#endif
