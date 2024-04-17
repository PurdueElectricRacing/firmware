/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em.c
 *
 * Code generated for Simulink model 'em'.
 *
 * Model version                  : 1.35
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Tue Apr 16 17:06:49 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "em.h"
#include "rtwtypes.h"

static uint32_T plook_evencag(real_T u, real_T bp0, real_T bpSpace, real_T
  *fraction);
static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[]);

/*===========*
 * Constants *
 *===========*/
#define RT_PI                          3.14159265358979323846
#define RT_PIF                         3.1415927F
#define RT_LN_10                       2.30258509299404568402
#define RT_LN_10F                      2.3025851F
#define RT_LOG10E                      0.43429448190325182765
#define RT_LOG10EF                     0.43429449F
#define RT_E                           2.7182818284590452354
#define RT_EF                          2.7182817F

/*
 * UNUSED_PARAMETER(x)
 *   Used to specify that a function parameter (argument) is required but not
 *   accessed by the function body.
 */
#ifndef UNUSED_PARAMETER
#if defined(__LCC__)
#define UNUSED_PARAMETER(x)                                      /* do nothing */
#else

/*
 * This is the semi-ANSI standard way of indicating that an
 * unused function parameter is required.
 */
#define UNUSED_PARAMETER(x)            (void) (x)
#endif
#endif

static uint32_T plook_evencag(real_T u, real_T bp0, real_T bpSpace, real_T
  *fraction)
{
  real_T invSpc;
  uint32_T bpIndex;

  /* Prelookup - Index and Fraction
     Index Search method: 'even'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'on'
   */
  invSpc = 1.0 / bpSpace;
  bpIndex = (uint32_T)((u - bp0) * invSpc);
  *fraction = (u - ((real_T)(uint32_T)((u - bp0) * invSpc) * bpSpace + bp0)) *
    invSpc;
  return bpIndex;
}

static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[])
{
  real_T y;
  real_T yL_0d0;
  uint32_T offset_1d;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'wrapping'
   */
  offset_1d = bpIndex[1U] * stride + bpIndex[0U];
  if (bpIndex[0U] == maxIndex[0U]) {
    y = table[offset_1d];
  } else {
    yL_0d0 = table[offset_1d];
    y = (table[offset_1d + 1U] - yL_0d0) * frac[0U] + yL_0d0;
  }

  if (bpIndex[1U] == maxIndex[1U]) {
  } else {
    offset_1d += stride;
    if (bpIndex[0U] == maxIndex[0U]) {
      yL_0d0 = table[offset_1d];
    } else {
      yL_0d0 = table[offset_1d];
      yL_0d0 += (table[offset_1d + 1U] - yL_0d0) * frac[0U];
    }

    y += (yL_0d0 - y) * frac[1U];
  }

  return y;
}

/* Model step function */
void em_step(RT_MODEL_em *const rtM_em, ExtU_em *rtU_em, ExtY_em *rtY_em)
{
  real_T fractions[2];
  real_T fractions_0[2];
  real_T bpIndices_tmp;
  real_T dk_idx_0;
  real_T dk_idx_1;
  real_T k_min_idx_0;
  real_T k_min_idx_1;
  uint32_T bpIndices[2];
  uint32_T bpIndices_0[2];
  dk_idx_0 = rtP_em.V[1] - rtP_em.V[0];
  bpIndices[1U] = plook_evencag(rtU_em->V, rtP_em.V[0], dk_idx_0, &dk_idx_1);
  fractions[1U] = dk_idx_1;
  bpIndices_tmp = rtP_em.w[1] - rtP_em.w[0];
  bpIndices[0U] = plook_evencag(rtU_em->w[0], rtP_em.w[0], bpIndices_tmp,
    &dk_idx_1);
  fractions[0U] = dk_idx_1;
  k_min_idx_0 = intrp2d_la(bpIndices, fractions, rtConstP_em.k_min_tableData,
    107U, rtConstP_em.pooled1);
  bpIndices[0U] = plook_evencag(rtU_em->w[1], rtP_em.w[0], bpIndices_tmp,
    &dk_idx_1);
  fractions[0U] = dk_idx_1;
  k_min_idx_1 = intrp2d_la(bpIndices, fractions, rtConstP_em.k_min_tableData,
    107U, rtConstP_em.pooled1);
  bpIndices_0[1U] = plook_evencag(rtU_em->V, rtP_em.V[0], dk_idx_0, &dk_idx_1);
  fractions_0[1U] = dk_idx_1;
  bpIndices_0[0U] = plook_evencag(rtU_em->w[0], rtP_em.w[0], bpIndices_tmp,
    &dk_idx_1);
  fractions_0[0U] = dk_idx_1;
  dk_idx_0 = intrp2d_la(bpIndices_0, fractions_0, rtConstP_em.dk_tableData, 107U,
                        rtConstP_em.pooled1);
  bpIndices_0[0U] = plook_evencag(rtU_em->w[1], rtP_em.w[0], bpIndices_tmp,
    &dk_idx_1);
  fractions_0[0U] = dk_idx_1;
  dk_idx_1 = intrp2d_la(bpIndices_0, fractions_0, rtConstP_em.dk_tableData, 107U,
                        rtConstP_em.pooled1);
  rtY_em->kTVS[0] = dk_idx_0 * rtU_em->rTVS[0] + k_min_idx_0;
  rtY_em->kTVS[1] = dk_idx_1 * rtU_em->rTVS[1] + k_min_idx_1;
  rtY_em->kEQUAL[0] = dk_idx_0 * rtU_em->rEQUAL[0] + k_min_idx_0;
  rtY_em->kEQUAL[1] = dk_idx_1 * rtU_em->rEQUAL[1] + k_min_idx_1;
  UNUSED_PARAMETER(rtM_em);
}

/* Model initialize function */
void em_initialize(RT_MODEL_em *const rtM_em)
{
  /* (no initialization code required) */
  UNUSED_PARAMETER(rtM_em);
}

/* Model terminate function */
void em_terminate(RT_MODEL_em *const rtM_em)
{
  /* (no terminate code required) */
  UNUSED_PARAMETER(rtM_em);
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
