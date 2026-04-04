#ifndef INTERP1_H
#define INTERP1_H

#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

double interp1(const double varargin_1[2], const double varargin_2[2],
               double varargin_3);

#ifdef __cplusplus
}
#endif

#endif
