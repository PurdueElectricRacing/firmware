/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: tv.c
 *
 * Code generated for Simulink model 'tv'.
 *
 * Model version                  : 1.45
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Sat May  4 21:24:23 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "tv.h"
#include "rtwtypes.h"
#include <math.h>
#include <string.h>

extern real_T rt_roundd(real_T u);
static uint32_T plook_evenca(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction);
static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[]);
static void MedianFilter_Init(DW_MedianFilter_tv *localDW);
static void MedianFilter(real_T rtu_0, DW_MedianFilter_tv *localDW);
static void MedianFilter_Term(DW_MedianFilter_tv *localDW);

/* Forward declaration for local functions */
static void MedianFilterCG_resetImpl(c_dsp_internal_MedianFilterC_tv *obj);
static void MedianFilterCG_trickleDownMin(c_dsp_internal_MedianFilterC_tv *obj,
  real_T i);
static void MedianFilterCG_trickleDownMax(c_dsp_internal_MedianFilterC_tv *obj,
  real_T i);

/* Forward declaration for local functions */
static void SystemCore_setup(dsp_simulink_MovingAverage_tv *obj);
static void SystemCore_setup_j(dsp_simulink_MovingAverage_j_tv *obj);
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

static void MedianFilterCG_resetImpl(c_dsp_internal_MedianFilterC_tv *obj)
{
  real_T cnt2;
  int32_T cnt1;
  int32_T i;
  memset(&obj->pBuf[0], 0, 150U * sizeof(real_T));
  memset(&obj->pPos[0], 0, 150U * sizeof(real_T));
  memset(&obj->pHeap[0], 0, 150U * sizeof(real_T));
  obj->pWinLen = 150.0;
  obj->pIdx = obj->pWinLen;
  obj->pMidHeap = ceil((obj->pWinLen + 1.0) / 2.0);
  obj->pMinHeapLength = trunc((obj->pWinLen - 1.0) / 2.0);
  obj->pMaxHeapLength = trunc(obj->pWinLen / 2.0);
  cnt1 = 1;
  cnt2 = obj->pWinLen;
  for (i = 0; i < 150; i++) {
    if (fmod(150.0 - (real_T)i, 2.0) == 0.0) {
      obj->pPos[149 - i] = cnt1;
      cnt1++;
    } else {
      obj->pPos[149 - i] = cnt2;
      cnt2--;
    }

    obj->pHeap[(int32_T)obj->pPos[149 - i] - 1] = 150.0 - (real_T)i;
  }
}

static void MedianFilterCG_trickleDownMin(c_dsp_internal_MedianFilterC_tv *obj,
  real_T i)
{
  boolean_T exitg1;
  exitg1 = false;
  while ((!exitg1) && (i <= obj->pMinHeapLength)) {
    real_T ind1;
    real_T ind2;
    real_T tmp;
    real_T tmp_0;
    if ((i > 1.0) && (i < obj->pMinHeapLength) && (obj->pBuf[(int32_T)obj->
         pHeap[(int32_T)((i + 1.0) + obj->pMidHeap) - 1] - 1] < obj->pBuf
         [(int32_T)obj->pHeap[(int32_T)(i + obj->pMidHeap) - 1] - 1])) {
      i++;
    }

    ind1 = i + obj->pMidHeap;
    ind2 = trunc(i / 2.0) + obj->pMidHeap;
    tmp = obj->pHeap[(int32_T)ind1 - 1];
    tmp_0 = obj->pHeap[(int32_T)ind2 - 1];
    if (obj->pBuf[(int32_T)tmp - 1] >= obj->pBuf[(int32_T)tmp_0 - 1]) {
      exitg1 = true;
    } else {
      obj->pHeap[(int32_T)ind1 - 1] = tmp_0;
      obj->pHeap[(int32_T)ind2 - 1] = tmp;
      obj->pPos[(int32_T)obj->pHeap[(int32_T)ind1 - 1] - 1] = ind1;
      obj->pPos[(int32_T)obj->pHeap[(int32_T)ind2 - 1] - 1] = ind2;
      i *= 2.0;
    }
  }
}

static void MedianFilterCG_trickleDownMax(c_dsp_internal_MedianFilterC_tv *obj,
  real_T i)
{
  boolean_T exitg1;
  exitg1 = false;
  while ((!exitg1) && (i >= -obj->pMaxHeapLength)) {
    real_T ind1;
    real_T ind2;
    real_T tmp;
    real_T tmp_0;
    if ((i < -1.0) && (i > -obj->pMaxHeapLength) && (obj->pBuf[(int32_T)
         obj->pHeap[(int32_T)(i + obj->pMidHeap) - 1] - 1] < obj->pBuf[(int32_T)
         obj->pHeap[(int32_T)((i - 1.0) + obj->pMidHeap) - 1] - 1])) {
      i--;
    }

    ind1 = trunc(i / 2.0) + obj->pMidHeap;
    ind2 = i + obj->pMidHeap;
    tmp = obj->pHeap[(int32_T)ind1 - 1];
    tmp_0 = obj->pHeap[(int32_T)ind2 - 1];
    if (obj->pBuf[(int32_T)tmp - 1] >= obj->pBuf[(int32_T)tmp_0 - 1]) {
      exitg1 = true;
    } else {
      obj->pHeap[(int32_T)ind1 - 1] = tmp_0;
      obj->pHeap[(int32_T)ind2 - 1] = tmp;
      obj->pPos[(int32_T)obj->pHeap[(int32_T)ind1 - 1] - 1] = ind1;
      obj->pPos[(int32_T)obj->pHeap[(int32_T)ind2 - 1] - 1] = ind2;
      i *= 2.0;
    }
  }
}

/* System initialize for atomic system: */
static void MedianFilter_Init(DW_MedianFilter_tv *localDW)
{
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  localDW->obj.isInitialized = 1;
  localDW->obj.NumChannels = 1;
  localDW->obj.pMID.isInitialized = 0;
  localDW->obj.isSetupComplete = true;
}

/* Output and update for atomic system: */
static void MedianFilter(real_T rtu_0, DW_MedianFilter_tv *localDW)
{
  c_dsp_internal_MedianFilterC_tv *obj;
  real_T flag_tmp;
  real_T p;
  real_T temp;
  real_T vprev;
  int32_T vprev_tmp;
  boolean_T exitg1;
  boolean_T flag;
  obj = &localDW->obj.pMID;
  if (localDW->obj.pMID.isInitialized != 1) {
    localDW->obj.pMID.isInitialized = 1;
    localDW->obj.pMID.isSetupComplete = true;
    MedianFilterCG_resetImpl(&localDW->obj.pMID);
  }

  vprev_tmp = (int32_T)localDW->obj.pMID.pIdx - 1;
  vprev = localDW->obj.pMID.pBuf[vprev_tmp];
  localDW->obj.pMID.pBuf[vprev_tmp] = rtu_0;
  p = localDW->obj.pMID.pPos[(int32_T)localDW->obj.pMID.pIdx - 1];
  localDW->obj.pMID.pIdx++;
  if (localDW->obj.pMID.pWinLen + 1.0 == localDW->obj.pMID.pIdx) {
    localDW->obj.pMID.pIdx = 1.0;
  }

  if (p > localDW->obj.pMID.pMidHeap) {
    if (vprev < rtu_0) {
      MedianFilterCG_trickleDownMin(&localDW->obj.pMID, (p -
        localDW->obj.pMID.pMidHeap) * 2.0);
    } else {
      vprev = p - localDW->obj.pMID.pMidHeap;
      exitg1 = false;
      while ((!exitg1) && (vprev > 0.0)) {
        flag_tmp = trunc(vprev / 2.0);
        flag = (obj->pBuf[(int32_T)obj->pHeap[(int32_T)(vprev + obj->pMidHeap) -
                1] - 1] < obj->pBuf[(int32_T)obj->pHeap[(int32_T)(flag_tmp +
                 obj->pMidHeap) - 1] - 1]);
        if (!flag) {
          exitg1 = true;
        } else {
          p = vprev + obj->pMidHeap;
          flag_tmp += obj->pMidHeap;
          temp = obj->pHeap[(int32_T)p - 1];
          obj->pHeap[(int32_T)p - 1] = obj->pHeap[(int32_T)flag_tmp - 1];
          obj->pHeap[(int32_T)flag_tmp - 1] = temp;
          obj->pPos[(int32_T)obj->pHeap[(int32_T)p - 1] - 1] = p;
          obj->pPos[(int32_T)obj->pHeap[(int32_T)flag_tmp - 1] - 1] = flag_tmp;
          vprev = trunc(vprev / 2.0);
        }
      }

      if (vprev == 0.0) {
        MedianFilterCG_trickleDownMax(&localDW->obj.pMID, -1.0);
      }
    }
  } else if (p < localDW->obj.pMID.pMidHeap) {
    if (rtu_0 < vprev) {
      MedianFilterCG_trickleDownMax(&localDW->obj.pMID, (p -
        localDW->obj.pMID.pMidHeap) * 2.0);
    } else {
      vprev = p - localDW->obj.pMID.pMidHeap;
      exitg1 = false;
      while ((!exitg1) && (vprev < 0.0)) {
        flag_tmp = trunc(vprev / 2.0);
        flag = (obj->pBuf[(int32_T)obj->pHeap[(int32_T)(flag_tmp + obj->pMidHeap)
                - 1] - 1] < obj->pBuf[(int32_T)obj->pHeap[(int32_T)(vprev +
                 obj->pMidHeap) - 1] - 1]);
        if (!flag) {
          exitg1 = true;
        } else {
          p = flag_tmp + obj->pMidHeap;
          flag_tmp = vprev + obj->pMidHeap;
          temp = obj->pHeap[(int32_T)p - 1];
          obj->pHeap[(int32_T)p - 1] = obj->pHeap[(int32_T)flag_tmp - 1];
          obj->pHeap[(int32_T)flag_tmp - 1] = temp;
          obj->pPos[(int32_T)obj->pHeap[(int32_T)p - 1] - 1] = p;
          obj->pPos[(int32_T)obj->pHeap[(int32_T)flag_tmp - 1] - 1] = flag_tmp;
          vprev = trunc(vprev / 2.0);
        }
      }

      if (vprev == 0.0) {
        MedianFilterCG_trickleDownMin(&localDW->obj.pMID, 1.0);
      }
    }
  } else {
    if (localDW->obj.pMID.pMaxHeapLength != 0.0) {
      MedianFilterCG_trickleDownMax(&localDW->obj.pMID, -1.0);
    }

    if (localDW->obj.pMID.pMinHeapLength > 0.0) {
      MedianFilterCG_trickleDownMin(&localDW->obj.pMID, 1.0);
    }
  }

  vprev = localDW->obj.pMID.pBuf[(int32_T)localDW->obj.pMID.pHeap[(int32_T)
    localDW->obj.pMID.pMidHeap - 1] - 1];
  localDW->MedianFilter_c = (localDW->obj.pMID.pBuf[(int32_T)
    localDW->obj.pMID.pHeap[(int32_T)(localDW->obj.pMID.pMidHeap - 1.0) - 1] - 1]
    + vprev) / 2.0;
}

/* Termination for atomic system: */
static void MedianFilter_Term(DW_MedianFilter_tv *localDW)
{
  if (!localDW->obj.matlabCodegenIsDeleted) {
    localDW->obj.matlabCodegenIsDeleted = true;
    if ((localDW->obj.isInitialized == 1) && localDW->obj.isSetupComplete) {
      localDW->obj.NumChannels = -1;
      if (localDW->obj.pMID.isInitialized == 1) {
        localDW->obj.pMID.isInitialized = 2;
      }
    }
  }
}

// real_T rt_roundd(real_T u)
// {
//   real_T y;
//   if (fabs(u) < 4.503599627370496E+15) {
//     if (u >= 0.5) {
//       y = floor(u + 0.5);
//     } else if (u > -0.5) {
//       y = 0.0;
//     } else {
//       y = ceil(u - 0.5);
//     }
//   } else {
//     y = u;
//   }

//   return y;
// }

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
  real_T csumrev[19];
  real_T Saturation[16];
  real_T csumrev_0[5];
  real_T Product_p[3];
  real_T tmp[3];
  real_T fractions[2];
  real_T Product_h;
  real_T R;
  real_T Saturation_0;
  real_T Saturation_1;
  real_T csum;
  real_T cumRevIndex;
  real_T modValueRev;
  real_T tmp_0;
  real_T u0;
  real_T z;
  int32_T i;
  int32_T localProduct;
  uint32_T bpIndices[2];
  boolean_T MatrixConcatenate[39];
  boolean_T LessThan[16];
  for (i = 0; i < 16; i++) {
    u0 = rtU_tv->D_raw[i];
    cumRevIndex = rtConstP_tv.Saturation_LowerSat[i];
    csum = rtP_tv.ub[i];
    if (u0 > csum) {
      Saturation[i] = csum;
    } else if (u0 < cumRevIndex) {
      Saturation[i] = cumRevIndex;
    } else {
      Saturation[i] = u0;
    }
  }

  rtY_tv->rEQUAL = Saturation[0];
  memcpy(&rtY_tv->sig_trunc[0], &Saturation[0], 12U * sizeof(real_T));
  rtY_tv->sig_trunc[12] = Saturation[13];
  rtY_tv->sig_trunc[13] = Saturation[14];
  rtY_tv->sig_trunc[14] = Saturation[15];
  MedianFilter(Saturation[11], &rtDW_tv->MedianFilter1);
  MedianFilter(Saturation[10], &rtDW_tv->MedianFilter_g);
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
  u0 = 0.0;
  csum += Saturation[9];
  if (modValueRev == 0.0) {
    z = csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  csumrev[(int32_T)cumRevIndex - 1] = Saturation[9];
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
    u0 = z / 20.0;
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

  cumRevIndex = Saturation[6];
  csum = Saturation[13];
  modValueRev = Saturation[7];
  z = Saturation[14];
  Saturation_0 = Saturation[15];
  Saturation_1 = Saturation[8];
  for (i = 0; i < 3; i++) {
    R = rtU_tv->R[i];
    Product_h = R * cumRevIndex;
    tmp_0 = R * csum;
    R = rtU_tv->R[i + 3];
    Product_h += R * modValueRev;
    tmp_0 += R * z;
    R = rtU_tv->R[i + 6];
    tmp[i] = R * Saturation_0 + tmp_0;
    Product_p[i] = R * Saturation_1 + Product_h;
  }

  rtY_tv->sig_filt[0] = Saturation[0];
  rtY_tv->sig_filt[1] = -Saturation[1];
  rtY_tv->sig_filt[2] = Saturation[2];
  rtY_tv->sig_filt[3] = Saturation[3];
  rtY_tv->sig_filt[4] = Saturation[4];
  rtY_tv->sig_filt[5] = Saturation[5];
  rtY_tv->sig_filt[6] = Product_p[0];
  rtY_tv->sig_filt[7] = Product_p[1];
  rtY_tv->sig_filt[8] = Product_p[2];
  rtY_tv->sig_filt[9] = u0;
  rtY_tv->sig_filt[10] = rtDW_tv->MedianFilter_g.MedianFilter_c;
  rtY_tv->sig_filt[11] = rtDW_tv->MedianFilter1.MedianFilter_c;
  rtY_tv->sig_filt[12] = tmp[0];
  rtY_tv->sig_filt[13] = tmp[1];
  rtY_tv->sig_filt[14] = tmp[2];
  MedianFilter(Saturation[12], &rtDW_tv->MedianFilter2);
  cumRevIndex = fmin(fmin(fmin((rtDW_tv->MedianFilter1.MedianFilter_c +
    rtP_tv.mT_bias) * rtP_tv.mT_gain + 1.0,
    (rtDW_tv->MedianFilter_g.MedianFilter_c + rtP_tv.mcT_bias) * rtP_tv.mcT_gain
    + 1.0), (rtDW_tv->MedianFilter2.MedianFilter_c + rtP_tv.bT_bias) *
    rtP_tv.bT_gain + 1.0), (u0 + rtP_tv.bI_bias) * rtP_tv.bI_gain + 1.0);
  if (cumRevIndex > 1.0) {
    cumRevIndex = 1.0;
  } else if (cumRevIndex < 0.0) {
    cumRevIndex = 0.0;
  }

  if (Saturation[0] > cumRevIndex) {
    csum = cumRevIndex;
  } else if (Saturation[0] < 0.0) {
    csum = 0.0;
  } else {
    csum = Saturation[0];
  }

  z = rtP_tv.r_power_sat * csum;
  if (-Saturation[1] > 130.0) {
    modValueRev = 130.0;
  } else if (-Saturation[1] < -130.0) {
    modValueRev = -130.0;
  } else {
    modValueRev = -Saturation[1];
  }

  bpIndices[0U] = plook_evenca(Saturation[5], rtP_tv.v[0], rtP_tv.v[1] -
    rtP_tv.v[0], 50U, &u0);
  fractions[0U] = u0;
  bpIndices[1U] = plook_evenca(fabs(modValueRev) > rtU_tv->dphi ? modValueRev :
    0.0, rtP_tv.s[0], rtP_tv.s[1] - rtP_tv.s[0], 52U, &u0);
  fractions[1U] = u0;
  modValueRev = (rtU_tv->TVS_I * intrp2d_la(bpIndices, fractions,
    rtP_tv.yaw_table, 51U, rtConstP_tv.uDLookupTable_maxIndex) - Product_p[2]) *
    rtU_tv->TVS_P * rtP_tv.half_track[1];
  if (modValueRev <= z) {
    z = -z;
    if (modValueRev >= z) {
      z = modValueRev;
    }
  }

  if (z > 0.0) {
    rtY_tv->rTVS[0] = csum;
    rtY_tv->rTVS[1] = csum - z;
  } else {
    rtY_tv->rTVS[0] = csum - fabs(z);
    rtY_tv->rTVS[1] = csum;
  }

  rtY_tv->max_K = cumRevIndex;
  for (i = 0; i < 16; i++) {
    LessThan[i] = (rtP_tv.ub[i] + rtP_tv.epsilon > rtU_tv->D_raw[i]);
  }

  for (i = 0; i < 10; i++) {
    MatrixConcatenate[i + 13] = LessThan[i];
  }

  MatrixConcatenate[23] = LessThan[13];
  MatrixConcatenate[24] = LessThan[14];
  MatrixConcatenate[25] = LessThan[15];
  for (i = 0; i < 16; i++) {
    LessThan[i] = (rtConstP_tv.Constant5_lb[i] - rtP_tv.epsilon < rtU_tv->
                   D_raw[i]);
  }

  for (i = 0; i < 10; i++) {
    MatrixConcatenate[i + 26] = LessThan[i];
  }

  MatrixConcatenate[36] = LessThan[13];
  MatrixConcatenate[37] = LessThan[14];
  MatrixConcatenate[38] = LessThan[15];
  for (i = 0; i < 13; i++) {
    MatrixConcatenate[i] = rtU_tv->F_raw[i];
  }

  localProduct = MatrixConcatenate[0] ? (int32_T)MatrixConcatenate[1] : 0;
  for (i = 0; i < 37; i++) {
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
  u0 = 0.0;
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
    u0 = z / 6.0;
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

  rtY_tv->TVS_STATE = rt_roundd(u0);
  for (i = 0; i < 39; i++) {
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
    MedianFilter_Init(&rtDW_tv->MedianFilter1);
    MedianFilter_Init(&rtDW_tv->MedianFilter_g);
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

    MedianFilter_Init(&rtDW_tv->MedianFilter2);
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

/* Model terminate function */
void tv_terminate(RT_MODEL_tv *const rtM_tv)
{
  DW_tv *rtDW_tv = rtM_tv->dwork;
  h_dsp_internal_SlidingWind_j_tv *obj_0;
  h_dsp_internal_SlidingWindow_tv *obj;
  MedianFilter_Term(&rtDW_tv->MedianFilter1);
  MedianFilter_Term(&rtDW_tv->MedianFilter_g);
  if (!rtDW_tv->obj.matlabCodegenIsDeleted) {
    rtDW_tv->obj.matlabCodegenIsDeleted = true;
    if ((rtDW_tv->obj.isInitialized == 1) && rtDW_tv->obj.isSetupComplete) {
      obj = rtDW_tv->obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      rtDW_tv->obj.NumChannels = -1;
      rtDW_tv->obj.FrameLength = -1;
    }
  }

  MedianFilter_Term(&rtDW_tv->MedianFilter2);
  if (!rtDW_tv->obj_m.matlabCodegenIsDeleted) {
    rtDW_tv->obj_m.matlabCodegenIsDeleted = true;
    if ((rtDW_tv->obj_m.isInitialized == 1) && rtDW_tv->obj_m.isSetupComplete) {
      obj_0 = rtDW_tv->obj_m.pStatistic;
      if (obj_0->isInitialized == 1) {
        obj_0->isInitialized = 2;
      }

      rtDW_tv->obj_m.NumChannels = -1;
      rtDW_tv->obj_m.FrameLength = -1;
    }
  }
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
