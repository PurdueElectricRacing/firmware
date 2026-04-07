#ifndef VCU_STEP_H
#define VCU_STEP_H

#include "rtwtypes.h"
#include "vcu_step_types.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void vcu_step(const pVCU_struct *p, const xVCU_struct *x,
                     yVCU_struct *y);

#ifdef __cplusplus
}
#endif

#endif
