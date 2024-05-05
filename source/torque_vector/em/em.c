/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em.c
 *
 * Code generated for Simulink model 'em'.
 *
 * Model version                  : 1.51
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Sat May  4 21:48:34 2024
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
#include <math.h>

extern real_T rt_roundd(real_T u);
static uint32_T plook_evenca(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction);
static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[]);

/* Forward declaration for local functions */
static void em_SystemCore_setup(dsp_simulink_MovingAverage_em *obj);
static uint32_T plook_evenca(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction)
{
  uint32_T bpIndex;

  /* Prelookup - Index and Fraction
     Index Search method: 'even'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u <= bp0) {
    bpIndex = 0U;
    *fraction = 0.0;
  } else {
    real_T fbpIndex;
    real_T invSpc;
    invSpc = 1.0 / bpSpace;
    fbpIndex = (u - bp0) * invSpc;
    if (fbpIndex < maxIndex) {
      bpIndex = (uint32_T)fbpIndex;
      *fraction = (u - ((real_T)(uint32_T)fbpIndex * bpSpace + bp0)) * invSpc;
    } else {
      bpIndex = maxIndex;
      *fraction = 0.0;
    }
  }

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

real_T rt_roundd(real_T u)
{
  real_T y;
  if (fabs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = floor(u + 0.5);
    } else if (u > -0.5) {
      y = 0.0;
    } else {
      y = ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

static void em_SystemCore_setup(dsp_simulink_MovingAverage_em *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

/* Model step function */
void em_step(RT_MODEL_em *const rtM_em, ExtU_em *rtU_em, ExtY_em *rtY_em)
{
  DW_em *rtDW_em = rtM_em->dwork;
  h_dsp_internal_SlidingWindow_em *obj;
  real_T csumrev[5];
  real_T fractions[2];
  real_T MovingAverage1_i_0;
  real_T cumRevIndex;
  real_T k_max_idx_0;
  real_T modValueRev;
  real_T z;
  int32_T i;
  int32_T localProduct;
  uint32_T bpIndices[2];
  boolean_T MatrixConcatenate[10];
  if (rtU_em->D_raw[0] > rtP_em.ub_mm[0]) {
    modValueRev = rtP_em.ub_mm[0];
  } else if (rtU_em->D_raw[0] < rtP_em.lb_mm[0]) {
    modValueRev = rtP_em.lb_mm[0];
  } else {
    modValueRev = rtU_em->D_raw[0];
  }

  bpIndices[1U] = plook_evenca(modValueRev, rtP_em.V[0], rtP_em.V[1] - rtP_em.V
    [0], 25U, &cumRevIndex);
  fractions[1U] = cumRevIndex;
  if (rtU_em->D_raw[1] > rtP_em.ub_mm[1]) {
    modValueRev = rtP_em.ub_mm[1];
  } else if (rtU_em->D_raw[1] < rtP_em.lb_mm[1]) {
    modValueRev = rtP_em.lb_mm[1];
  } else {
    modValueRev = rtU_em->D_raw[1];
  }

  z = rtP_em.w[1] - rtP_em.w[0];
  bpIndices[0U] = plook_evenca(modValueRev, rtP_em.w[0], z, 106U, &cumRevIndex);
  fractions[0U] = cumRevIndex;
  k_max_idx_0 = intrp2d_la(bpIndices, fractions, rtP_em.maxK, 107U,
    rtConstP_em.k_max_maxIndex);
  if (rtU_em->D_raw[2] > rtP_em.ub_mm[2]) {
    modValueRev = rtP_em.ub_mm[2];
  } else if (rtU_em->D_raw[2] < rtP_em.lb_mm[2]) {
    modValueRev = rtP_em.lb_mm[2];
  } else {
    modValueRev = rtU_em->D_raw[2];
  }

  bpIndices[0U] = plook_evenca(modValueRev, rtP_em.w[0], z, 106U, &cumRevIndex);
  fractions[0U] = cumRevIndex;
  cumRevIndex = intrp2d_la(bpIndices, fractions, rtP_em.maxK, 107U,
    rtConstP_em.k_max_maxIndex);
  rtY_em->kTVS[0] = k_max_idx_0 * rtU_em->rTV[0];
  rtY_em->kEQUAL[0] = k_max_idx_0 * rtU_em->rEQUAL;
  rtY_em->kTVS[1] = cumRevIndex * rtU_em->rTV[1];
  rtY_em->kEQUAL[1] = cumRevIndex * rtU_em->rEQUAL;
  MatrixConcatenate[4] = (rtP_em.ub_mm[0] + rtP_em.epsilon > rtU_em->D_raw[0]);
  MatrixConcatenate[7] = (rtP_em.lb_mm[0] - rtP_em.epsilon < rtU_em->D_raw[0]);
  MatrixConcatenate[5] = (rtP_em.ub_mm[1] + rtP_em.epsilon > rtU_em->D_raw[1]);
  MatrixConcatenate[8] = (rtP_em.lb_mm[1] - rtP_em.epsilon < rtU_em->D_raw[1]);
  MatrixConcatenate[6] = (rtP_em.ub_mm[2] + rtP_em.epsilon > rtU_em->D_raw[2]);
  MatrixConcatenate[9] = (rtP_em.lb_mm[2] - rtP_em.epsilon < rtU_em->D_raw[2]);
  MatrixConcatenate[0] = rtU_em->F_raw[0];
  MatrixConcatenate[1] = rtU_em->F_raw[1];
  MatrixConcatenate[2] = rtU_em->F_raw[2];
  MatrixConcatenate[3] = rtU_em->F_raw[3];
  localProduct = rtU_em->F_raw[0] ? (int32_T)rtU_em->F_raw[1] : 0;
  for (i = 0; i < 8; i++) {
    localProduct = MatrixConcatenate[i + 2] ? localProduct : 0;
  }

  if (rtDW_em->obj.TunablePropsChanged) {
    rtDW_em->obj.TunablePropsChanged = false;
  }

  obj = rtDW_em->obj.pStatistic;
  if (rtDW_em->obj.pStatistic->isInitialized != 1) {
    rtDW_em->obj.pStatistic->isSetupComplete = false;
    rtDW_em->obj.pStatistic->isInitialized = 1;
    obj->pCumSum = 0.0;
    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 5; i++) {
      obj->pCumSumRev[i] = 0.0;
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  k_max_idx_0 = obj->pCumSum;
  for (i = 0; i < 5; i++) {
    csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;
  MovingAverage1_i_0 = 0.0;
  k_max_idx_0 += (real_T)localProduct;
  if (modValueRev == 0.0) {
    z = csumrev[(int32_T)cumRevIndex - 1] + k_max_idx_0;
  }

  csumrev[(int32_T)cumRevIndex - 1] = localProduct;
  if (cumRevIndex != 5.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    k_max_idx_0 = 0.0;
    csumrev[3] += csumrev[4];
    csumrev[2] += csumrev[3];
    csumrev[1] += csumrev[2];
    csumrev[0] += csumrev[1];
  }

  if (modValueRev == 0.0) {
    MovingAverage1_i_0 = z / 6.0;
  }

  obj->pCumSum = k_max_idx_0;
  for (i = 0; i < 5; i++) {
    obj->pCumSumRev[i] = csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }

  rtY_em->MM_STATE = rt_roundd(MovingAverage1_i_0);
  for (i = 0; i < 10; i++) {
    rtY_em->MM_FLAGS[i] = MatrixConcatenate[i];
  }
}

/* Model initialize function */
void em_initialize(RT_MODEL_em *const rtM_em)
{
  DW_em *rtDW_em = rtM_em->dwork;

  {
    h_dsp_internal_SlidingWindow_em *obj;
    int32_T i;
    rtDW_em->obj.isInitialized = 0;
    rtDW_em->obj.NumChannels = -1;
    rtDW_em->obj.FrameLength = -1;
    rtDW_em->obj.matlabCodegenIsDeleted = false;
    em_SystemCore_setup(&rtDW_em->obj);
    obj = rtDW_em->obj.pStatistic;
    if (obj->isInitialized == 1) {
      obj->pCumSum = 0.0;
      for (i = 0; i < 5; i++) {
        obj->pCumSumRev[i] = 0.0;
      }

      obj->pCumRevIndex = 1.0;
      obj->pModValueRev = 0.0;
    }
  }
}

/* Model terminate function */
void em_terminate(RT_MODEL_em *const rtM_em)
{
  DW_em *rtDW_em = rtM_em->dwork;
  h_dsp_internal_SlidingWindow_em *obj;
  if (!rtDW_em->obj.matlabCodegenIsDeleted) {
    rtDW_em->obj.matlabCodegenIsDeleted = true;
    if ((rtDW_em->obj.isInitialized == 1) && rtDW_em->obj.isSetupComplete) {
      obj = rtDW_em->obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      rtDW_em->obj.NumChannels = -1;
      rtDW_em->obj.FrameLength = -1;
    }
  }
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
