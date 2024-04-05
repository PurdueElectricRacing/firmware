/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: tv.c
 *
 * Code generated for Simulink model 'tv'.
 *
 * Model version                  : 1.26
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Thu Mar 28 16:15:49 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "tv.h"
#include <math.h>
#include "rtwtypes.h"

extern real_T rt_roundd(real_T u);
static uint32_T plook_evencag(real_T u, real_T bp0, real_T bpSpace, real_T
  *fraction);
static real_T intrp1d_la(uint32_T bpIndex, real_T frac, const real_T table[],
  uint32_T maxIndex);
static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[]);

/* Forward declaration for local functions */
static void SystemCore_setup(dsp_simulink_MovingAverage_tv *obj);
static void SystemCore_setup_j(dsp_simulink_MovingAverage_j_tv *obj);
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

static real_T intrp1d_la(uint32_T bpIndex, real_T frac, const real_T table[],
  uint32_T maxIndex)
{
  real_T y;

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'wrapping'
   */
  if (bpIndex == maxIndex) {
    y = table[bpIndex];
  } else {
    real_T yL_0d0;
    yL_0d0 = table[bpIndex];
    y = (table[bpIndex + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
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

static void SystemCore_setup(dsp_simulink_MovingAverage_tv *obj)
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

static void SystemCore_setup_j(dsp_simulink_MovingAverage_j_tv *obj)
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
void tv_step(RT_MODEL_tv *const rtM_tv, ExtU_tv *rtU_tv, ExtY_tv *rtY_tv)
{
  DW_tv *rtDW_tv = rtM_tv->dwork;
  h_dsp_internal_SlidingWind_j_tv *obj_0;
  h_dsp_internal_SlidingWindow_tv *obj;
  real_T Saturation[19];
  real_T csumrev[19];
  real_T csumrev_0[5];
  real_T Product_p[3];
  real_T tmp[3];
  real_T fractions[2];
  real_T BatterytoMaxPowerLevel;
  real_T Gain4;
  real_T csum;
  real_T cumRevIndex;
  real_T modValueRev;
  real_T z;
  int32_T i;
  int32_T localProduct;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  boolean_T MatrixConcatenate[51];
  for (i = 0; i < 19; i++) {
    BatterytoMaxPowerLevel = rtU_tv->D_raw[i];
    cumRevIndex = rtConstP_tv.pooled1[i];
    csum = rtP_tv.ub[i];
    if (BatterytoMaxPowerLevel > csum) {
      BatterytoMaxPowerLevel = csum;
      Saturation[i] = csum;
    } else if (BatterytoMaxPowerLevel < cumRevIndex) {
      BatterytoMaxPowerLevel = cumRevIndex;
      Saturation[i] = cumRevIndex;
    } else {
      Saturation[i] = BatterytoMaxPowerLevel;
    }

    rtY_tv->sig_trunc[i] = BatterytoMaxPowerLevel;
  }

  rtY_tv->w[0] = Saturation[3];
  rtY_tv->w[1] = Saturation[4];
  rtY_tv->V = Saturation[2];
  if (rtDW_tv->obj.TunablePropsChanged) {
    rtDW_tv->obj.TunablePropsChanged = false;
  }

  obj = rtDW_tv->obj.pStatistic;
  if (rtDW_tv->obj.pStatistic->isInitialized != 1) {
    rtDW_tv->obj.pStatistic->isSetupComplete = false;
    rtDW_tv->obj.pStatistic->isInitialized = 1;
    obj->pCumSum = 0.0;
    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 19; i++) {
      obj->pCumSumRev[i] = 0.0;
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  csum = obj->pCumSum;
  for (i = 0; i < 19; i++) {
    csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;
  BatterytoMaxPowerLevel = 0.0;
  csum += Saturation[11];
  if (modValueRev == 0.0) {
    z = csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  csumrev[(int32_T)cumRevIndex - 1] = Saturation[11];
  if (cumRevIndex != 19.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    for (i = 17; i >= 0; i--) {
      csumrev[i] += csumrev[i + 1];
    }
  }

  if (modValueRev == 0.0) {
    BatterytoMaxPowerLevel = z / 20.0;
  }

  obj->pCumSum = csum;
  for (i = 0; i < 19; i++) {
    obj->pCumSumRev[i] = csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = 0.0;
  for (i = 0; i < 3; i++) {
    csum = Saturation[i + 5];
    cumRevIndex += csum * csum;
    csum = rtU_tv->R[i];
    modValueRev = csum * Saturation[8];
    z = csum * Saturation[16];
    csum = rtU_tv->R[i + 3];
    modValueRev += csum * Saturation[9];
    z += csum * Saturation[17];
    csum = rtU_tv->R[i + 6];
    tmp[i] = csum * Saturation[18] + z;
    Product_p[i] = csum * Saturation[10] + modValueRev;
  }

  modValueRev = sqrt(cumRevIndex);
  rtY_tv->sig_filt[0] = Saturation[0];
  rtY_tv->sig_filt[1] = Saturation[0];
  rtY_tv->sig_filt[2] = Saturation[1];
  rtY_tv->sig_filt[3] = Saturation[2];
  rtY_tv->sig_filt[4] = Saturation[3];
  rtY_tv->sig_filt[5] = Saturation[4];
  rtY_tv->sig_filt[6] = modValueRev;
  rtY_tv->sig_filt[7] = Product_p[0];
  rtY_tv->sig_filt[8] = Product_p[1];
  rtY_tv->sig_filt[9] = Product_p[2];
  rtY_tv->sig_filt[10] = BatterytoMaxPowerLevel;
  rtY_tv->sig_filt[11] = Saturation[12];
  rtY_tv->sig_filt[12] = Saturation[13];
  rtY_tv->sig_filt[13] = Saturation[14];
  rtY_tv->sig_filt[14] = Saturation[15];
  rtY_tv->sig_filt[15] = tmp[0];
  rtY_tv->sig_filt[16] = tmp[1];
  rtY_tv->sig_filt[17] = tmp[2];
  rtY_tv->rEQUAL[0] = Saturation[0];
  rtY_tv->rEQUAL[1] = Saturation[0];
  bpIdx = plook_evencag(fmax(Saturation[14], Saturation[15]), rtP_tv.Tmo[0],
                        rtP_tv.Tmo[1] - rtP_tv.Tmo[0], &z);
  cumRevIndex = intrp1d_la(bpIdx, z, rtP_tv.k_TL, 1U);
  bpIdx = plook_evencag(fmax(Saturation[12], Saturation[13]), rtP_tv.Tmc[0],
                        rtP_tv.Tmc[1] - rtP_tv.Tmc[0], &z);
  csum = intrp1d_la(bpIdx, z, rtP_tv.k_TL, 1U);
  bpIdx = plook_evencag(BatterytoMaxPowerLevel - rtP_tv.I_FUSE, rtP_tv.dIb[0],
                        rtP_tv.dIb[1] - rtP_tv.dIb[0], &z);
  BatterytoMaxPowerLevel = intrp1d_la(bpIdx, z, rtP_tv.k_TL, 1U);
  Gain4 = rtP_tv.r_power_sat / rtP_tv.PLb * fmin(fmin(fmin(Saturation[0] +
    Saturation[0], cumRevIndex), csum), BatterytoMaxPowerLevel);
  bpIndices[0U] = plook_evencag(modValueRev, rtP_tv.v[0], rtP_tv.v[1] -
    rtP_tv.v[0], &z);
  fractions[0U] = z;
  bpIndices[1U] = plook_evencag(fabs(Saturation[1]) > rtU_tv->dphi ? Saturation
    [1] : 0.0, rtP_tv.s[0], rtP_tv.s[1] - rtP_tv.s[0], &z);
  fractions[1U] = z;
  modValueRev = (rtU_tv->TVS_I * intrp2d_la(bpIndices, fractions,
    rtP_tv.yaw_table, 51U, rtConstP_tv.uDLookupTable_maxIndex) - Product_p[2]) *
    rtU_tv->TVS_P * rtP_tv.half_track[1];
  if (modValueRev <= Gain4) {
    Gain4 = -Gain4;
    if (modValueRev >= Gain4) {
      Gain4 = modValueRev;
    }
  }

  if (Gain4 > 0.0) {
    rtY_tv->rTVS[0] = Saturation[0];
    rtY_tv->rTVS[1] = Saturation[0] - Gain4;
  } else {
    rtY_tv->rTVS[0] = Saturation[0] - fabs(Gain4);
    rtY_tv->rTVS[1] = Saturation[0];
  }

  rtY_tv->max_K = 1.0 / rtP_tv.PLb * fmin(fmin(cumRevIndex, csum),
    BatterytoMaxPowerLevel);
  for (i = 0; i < 19; i++) {
    BatterytoMaxPowerLevel = rtU_tv->D_raw[i];
    MatrixConcatenate[i + 32] = (rtConstP_tv.pooled1[i] <=
      BatterytoMaxPowerLevel);
    MatrixConcatenate[i + 13] = (rtP_tv.ub[i] >= BatterytoMaxPowerLevel);
  }

  for (i = 0; i < 13; i++) {
    MatrixConcatenate[i] = rtU_tv->F_raw[i];
  }

  localProduct = MatrixConcatenate[0] ? (int32_T)MatrixConcatenate[1] : 0;
  for (i = 0; i < 49; i++) {
    localProduct = MatrixConcatenate[i + 2] ? localProduct : 0;
  }

  if (rtDW_tv->obj_m.TunablePropsChanged) {
    rtDW_tv->obj_m.TunablePropsChanged = false;
  }

  obj_0 = rtDW_tv->obj_m.pStatistic;
  if (rtDW_tv->obj_m.pStatistic->isInitialized != 1) {
    rtDW_tv->obj_m.pStatistic->isSetupComplete = false;
    rtDW_tv->obj_m.pStatistic->isInitialized = 1;
    obj_0->pCumSum = 0.0;
    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
    obj_0->isSetupComplete = true;
    obj_0->pCumSum = 0.0;
    for (i = 0; i < 5; i++) {
      obj_0->pCumSumRev[i] = 0.0;
      obj_0->pCumSumRev[i] = 0.0;
    }

    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
  }

  cumRevIndex = obj_0->pCumRevIndex;
  csum = obj_0->pCumSum;
  for (i = 0; i < 5; i++) {
    csumrev_0[i] = obj_0->pCumSumRev[i];
  }

  modValueRev = obj_0->pModValueRev;
  z = 0.0;
  Gain4 = 0.0;
  csum += (real_T)localProduct;
  if (modValueRev == 0.0) {
    z = csumrev_0[(int32_T)cumRevIndex - 1] + csum;
  }

  csumrev_0[(int32_T)cumRevIndex - 1] = localProduct;
  if (cumRevIndex != 5.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    csumrev_0[3] += csumrev_0[4];
    csumrev_0[2] += csumrev_0[3];
    csumrev_0[1] += csumrev_0[2];
    csumrev_0[0] += csumrev_0[1];
  }

  if (modValueRev == 0.0) {
    Gain4 = z / 6.0;
  }

  obj_0->pCumSum = csum;
  for (i = 0; i < 5; i++) {
    obj_0->pCumSumRev[i] = csumrev_0[i];
  }

  obj_0->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj_0->pModValueRev = modValueRev - 1.0;
  } else {
    obj_0->pModValueRev = 0.0;
  }

  rtY_tv->TVS_STATE = rt_roundd(Gain4);
  for (i = 0; i < 51; i++) {
    rtY_tv->F_TVS[i] = MatrixConcatenate[i];
  }
}

/* Model initialize function */
void tv_initialize(RT_MODEL_tv *const rtM_tv)
{
  DW_tv *rtDW_tv = rtM_tv->dwork;

  {
    h_dsp_internal_SlidingWind_j_tv *obj_0;
    h_dsp_internal_SlidingWindow_tv *obj;
    int32_T i;
    rtDW_tv->obj.isInitialized = 0;
    rtDW_tv->obj.NumChannels = -1;
    rtDW_tv->obj.FrameLength = -1;
    rtDW_tv->obj.matlabCodegenIsDeleted = false;
    SystemCore_setup(&rtDW_tv->obj);
    obj = rtDW_tv->obj.pStatistic;
    if (obj->isInitialized == 1) {
      obj->pCumSum = 0.0;
      for (i = 0; i < 19; i++) {
        obj->pCumSumRev[i] = 0.0;
      }

      obj->pCumRevIndex = 1.0;
      obj->pModValueRev = 0.0;
    }

    rtDW_tv->obj_m.isInitialized = 0;
    rtDW_tv->obj_m.NumChannels = -1;
    rtDW_tv->obj_m.FrameLength = -1;
    rtDW_tv->obj_m.matlabCodegenIsDeleted = false;
    SystemCore_setup_j(&rtDW_tv->obj_m);
    obj_0 = rtDW_tv->obj_m.pStatistic;
    if (obj_0->isInitialized == 1) {
      obj_0->pCumSum = 0.0;
      for (i = 0; i < 5; i++) {
        obj_0->pCumSumRev[i] = 0.0;
      }

      obj_0->pCumRevIndex = 1.0;
      obj_0->pModValueRev = 0.0;
    }
  }
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
