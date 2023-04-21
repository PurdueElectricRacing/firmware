/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: MC_PL0.c
 *
 * Code generated for Simulink model 'MC_PL0'.
 *
 * Model version                  : 1.267
 * Simulink Coder version         : 9.7 (R2022a) 13-Nov-2021
 * C/C++ source code generated on : Sat Dec  3 12:02:05 2022
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "MC_PL0.h"
#include <math.h>
#include "rtwtypes.h"
#include <stddef.h>
#define NumBitsPerChar                 8U

static real_T look1_binlc(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
static uint32_T plook_evenca(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction);
static real_T intrp2d_la(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride, const uint32_T maxIndex[]);
static uint32_T plook_evenc(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction);
static real_T intrp2d_l(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride);
extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static void rt_InitInfAndNaN(size_t realSize);
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
typedef struct {
  struct {
    uint32_T wordH;
    uint32_T wordL;
  } words;
} BigEndianIEEEDouble;

typedef struct {
  struct {
    uint32_T wordL;
    uint32_T wordH;
  } words;
} LittleEndianIEEEDouble;

typedef struct {
  union {
    real32_T wordLreal;
    uint32_T wordLuint;
  } wordL;
} IEEESingle;

real_T rtInf;
real_T rtMinusInf;
real_T rtNaN;
real32_T rtInfF;
real32_T rtMinusInfF;
real32_T rtNaNF;
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);

/*
 * Initialize the rtInf, rtMinusInf, and rtNaN needed by the
 * generated code. NaN is initialized as non-signaling. Assumes IEEE.
 */
static void rt_InitInfAndNaN(size_t realSize)
{
  (void) (realSize);
  rtNaN = rtGetNaN();
  rtNaNF = rtGetNaNF();
  rtInf = rtGetInf();
  rtInfF = rtGetInfF();
  rtMinusInf = rtGetMinusInf();
  rtMinusInfF = rtGetMinusInfF();
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
  return (boolean_T)((value==rtInf || value==rtMinusInf) ? 1U : 0U);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
  return (boolean_T)(((value)==rtInfF || (value)==rtMinusInfF) ? 1U : 0U);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
  boolean_T result = (boolean_T) 0;
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  if (bitsPerReal == 32U) {
    result = rtIsNaNF((real32_T)value);
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.fltVal = value;
    result = (boolean_T)((tmpVal.bitVal.words.wordH & 0x7FF00000) == 0x7FF00000 &&
                         ( (tmpVal.bitVal.words.wordH & 0x000FFFFF) != 0 ||
                          (tmpVal.bitVal.words.wordL != 0) ));
  }

  return result;
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
  IEEESingle tmp;
  tmp.wordL.wordLreal = value;
  return (boolean_T)( (tmp.wordL.wordLuint & 0x7F800000) == 0x7F800000 &&
                     (tmp.wordL.wordLuint & 0x007FFFFF) != 0 );
}

/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T inf = 0.0;
  if (bitsPerReal == 32U) {
    inf = rtGetInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0x7FF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    inf = tmpVal.fltVal;
  }

  return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetInfF(void)
{
  IEEESingle infF;
  infF.wordL.wordLuint = 0x7F800000U;
  return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetMinusInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T minf = 0.0;
  if (bitsPerReal == 32U) {
    minf = rtGetMinusInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    minf = tmpVal.fltVal;
  }

  return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetMinusInfF(void)
{
  IEEESingle minfF;
  minfF.wordL.wordLuint = 0xFF800000U;
  return minfF.wordL.wordLreal;
}

/*
 * Initialize rtNaN needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetNaN(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T nan = 0.0;
  if (bitsPerReal == 32U) {
    nan = rtGetNaNF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF80000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    nan = tmpVal.fltVal;
  }

  return nan;
}

/*
 * Initialize rtNaNF needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetNaNF(void)
{
  IEEESingle nanF = { { 0.0F } };

  nanF.wordL.wordLuint = 0xFFC00000U;
  return nanF.wordL.wordLreal;
}

static real_T look1_binlc(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex)
{
  real_T frac;
  real_T yL_0d0;
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex]) {
    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = 1.0;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

static uint32_T plook_evenca(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction)
{
  real_T fbpIndex;
  real_T invSpc;
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

static uint32_T plook_evenc(real_T u, real_T bp0, real_T bpSpace, uint32_T
  maxIndex, real_T *fraction)
{
  real_T fbpIndex;
  real_T invSpc;
  uint32_T bpIndex;

  /* Prelookup - Index and Fraction
     Index Search method: 'even'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u <= bp0) {
    bpIndex = 0U;
    *fraction = 0.0;
  } else {
    invSpc = 1.0 / bpSpace;
    fbpIndex = (u - bp0) * invSpc;
    if (fbpIndex < maxIndex) {
      bpIndex = (uint32_T)fbpIndex;
      *fraction = (u - ((real_T)(uint32_T)fbpIndex * bpSpace + bp0)) * invSpc;
    } else {
      bpIndex = maxIndex - 1U;
      *fraction = 1.0;
    }
  }

  return bpIndex;
}

static real_T intrp2d_l(const uint32_T bpIndex[], const real_T frac[], const
  real_T table[], const uint32_T stride)
{
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T offset_1d;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  offset_1d = bpIndex[1U] * stride + bpIndex[0U];
  yL_0d0 = table[offset_1d];
  yL_0d0 += (table[offset_1d + 1U] - yL_0d0) * frac[0U];
  offset_1d += stride;
  yL_0d1 = table[offset_1d];
  return (((table[offset_1d + 1U] - yL_0d1) * frac[0U] + yL_0d1) - yL_0d0) *
    frac[1U] + yL_0d0;
}

/* Model step function */
void MC_PL0_step(RT_MODEL *const rtM, ExtU *rtU, ExtY *rtY)
{
  DW *rtDW = rtM->dwork;
  real_T rtb_Divide_l_idx_0;
  real_T rtb_Divide_l_idx_1;
  real_T rtb_Divide_l_idx_2;
  real_T rtb_RateLimiter_m_idx_2;
  real_T rtb_Switch_c_idx_0;
  real_T rtb_Switch_c_idx_1;
  real_T rtb_Switch_c_idx_2;
  int32_T i;
  int32_T rtb_MatrixMultiply_tmp;

  /* Sum: '<S4>/Sum1' incorporates:
   *  Inport: '<Root>/Tx'
   */
  rtDW->Sum1 = ((rtU->Tx[0] + rtU->Tx[1]) + rtU->Tx[2]) + rtU->Tx[3];

  /* Switch: '<S4>/Switch' incorporates:
   *  Constant: '<S4>/Constant2'
   *  Inport: '<Root>/Tx'
   *  Product: '<S4>/Divide'
   *  Switch: '<S9>/Switch'
   */
  if (rtDW->Sum1 != 0.0) {
    rtDW->Switch[0] = rtU->Tx[0] / rtDW->Sum1;
    rtDW->Switch[1] = rtU->Tx[1] / rtDW->Sum1;
    rtDW->Switch[2] = rtU->Tx[2] / rtDW->Sum1;
    rtDW->Switch[3] = rtU->Tx[3] / rtDW->Sum1;
  } else {
    rtDW->Switch[0] = 0.25;
    rtDW->Switch[1] = 0.25;
    rtDW->Switch[2] = 0.25;
    rtDW->Switch[3] = 0.25;
  }

  /* End of Switch: '<S4>/Switch' */
  for (i = 0; i < 4; i++) {
    /* Product: '<S4>/MatrixMultiply' incorporates:
     *  Inport: '<Root>/power_limits'
     *  Switch: '<S9>/Switch'
     */
    rtDW->Sum1 = rtDW->Switch[i];
    rtb_MatrixMultiply_tmp = i << 1;
    rtDW->MatrixMultiply[rtb_MatrixMultiply_tmp] = rtU->power_limits[0] *
      rtDW->Sum1;
    rtDW->MatrixMultiply[rtb_MatrixMultiply_tmp + 1] = rtU->power_limits[1] *
      rtDW->Sum1;

    /* Sum: '<S1>/Sum' incorporates:
     *  Constant: '<S1>/Constant1'
     *  Inport: '<Root>/Wx'
     *  Product: '<S1>/Product2'
     *  Product: '<S1>/Product3'
     *  UnitDelay: '<S1>/Unit Delay1'
     */
    rtDW->Sum[i] = rtU->Wx[i] * 0.3 + 0.7 * rtDW->UnitDelay1_DSTATE[i];

    /* Abs: '<S4>/Abs1' incorporates:
     *  Inport: '<Root>/Tx'
     *  Switch: '<S9>/Switch'
     */
    rtDW->Switch[i] = fabs(rtU->Tx[i]);
  }

  /* Lookup_n-D: '<S4>/2-D Lookup Table' incorporates:
   *  Sum: '<S1>/Sum'
   *  Switch: '<S9>/Switch'
   */
  rtDW->bpIndices[0U] = plook_evenca(rtDW->Switch[0], 0.0, 1.6666666666666665,
    15U, &rtDW->Sum1);
  rtDW->fractions[0U] = rtDW->Sum1;
  rtDW->bpIndices[1U] = plook_evenca(rtDW->Sum[0], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions[1U] = rtDW->Sum1;
  rtDW->Switch[0] = intrp2d_la(rtDW->bpIndices, rtDW->fractions,
    rtConstP.uDLookupTable_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices[0U] = plook_evenca(rtDW->Switch[1], 0.0, 1.6666666666666665,
    15U, &rtDW->Sum1);
  rtDW->fractions[0U] = rtDW->Sum1;
  rtDW->bpIndices[1U] = plook_evenca(rtDW->Sum[1], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions[1U] = rtDW->Sum1;
  rtDW->Switch[1] = intrp2d_la(rtDW->bpIndices, rtDW->fractions,
    rtConstP.uDLookupTable_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices[0U] = plook_evenca(rtDW->Switch[2], 0.0, 1.6666666666666665,
    15U, &rtDW->Sum1);
  rtDW->fractions[0U] = rtDW->Sum1;
  rtDW->bpIndices[1U] = plook_evenca(rtDW->Sum[2], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions[1U] = rtDW->Sum1;
  rtDW->Switch[2] = intrp2d_la(rtDW->bpIndices, rtDW->fractions,
    rtConstP.uDLookupTable_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices[0U] = plook_evenca(rtDW->Switch[3], 0.0, 1.6666666666666665,
    15U, &rtDW->Sum1);
  rtDW->fractions[0U] = rtDW->Sum1;
  rtDW->bpIndices[1U] = plook_evenca(rtDW->Sum[3], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions[1U] = rtDW->Sum1;

  /* Signum: '<S4>/Sign1' incorporates:
   *  Inport: '<Root>/Tx'
   *  Product: '<S4>/Product2'
   *  Switch: '<S9>/Switch'
   */
  if (rtIsNaN(rtU->Tx[0])) {
    rtDW->Sum1 = rtU->Tx[0];
  } else if (rtU->Tx[0] < 0.0) {
    rtDW->Sum1 = -1.0;
  } else {
    rtDW->Sum1 = (rtU->Tx[0] > 0.0);
  }

  /* Product: '<S4>/Product2' incorporates:
   *  Switch: '<S9>/Switch'
   */
  rtDW->Sum1 *= rtDW->Switch[0];

  /* Switch: '<S7>/Switch2' incorporates:
   *  Gain: '<S4>/Gain'
   *  Product: '<S4>/MatrixMultiply'
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  RelationalOperator: '<S7>/UpperRelop'
   *  Selector: '<S4>/Selector'
   *  Selector: '<S4>/Selector1'
   *  Switch: '<S7>/Switch'
   */
  if (rtDW->Sum1 > rtDW->MatrixMultiply[0]) {
    rtDW->rtb_Switch2_k = rtDW->MatrixMultiply[0];
  } else if (rtDW->Sum1 < -rtDW->MatrixMultiply[1]) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Gain: '<S4>/Gain'
     *  Selector: '<S4>/Selector1'
     */
    rtDW->rtb_Switch2_k = -rtDW->MatrixMultiply[1];
  } else {
    rtDW->rtb_Switch2_k = rtDW->Sum1;
  }

  /* Product: '<S2>/Divide1' incorporates:
   *  Inport: '<Root>/Motor_I'
   *  Inport: '<Root>/Vbatt'
   *  Product: '<S2>/Product1'
   *  Sum: '<S1>/Sum'
   *  Sum: '<S2>/Subtract'
   */
  rtDW->rtb_Switch_c_b = (rtDW->rtb_Switch2_k - rtU->Motor_V[0] * rtU->Motor_I[0])
    * (1.0 / rtDW->Sum[0]);

  /* Saturate: '<S2>/Saturation3' incorporates:
   *  DeadZone: '<S2>/Dead Zone'
   */
  if (rtDW->rtb_Switch_c_b > 0.01) {
    rtDW->rtb_Switch_c_b -= 0.01;
  } else if (rtDW->rtb_Switch_c_b >= -0.01) {
    rtDW->rtb_Switch_c_b = 0.0;
  } else {
    rtDW->rtb_Switch_c_b -= -0.01;
  }

  if (rtDW->rtb_Switch_c_b > 25.0) {
    rtDW->rtb_Switch_c_b = 25.0;
  } else if (rtDW->rtb_Switch_c_b < -25.0) {
    rtDW->rtb_Switch_c_b = -25.0;
  }

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S2>/Gain1'
   */
  rtDW->DiscreteTimeIntegrator = 200.0 * rtDW->rtb_Switch_c_b * 0.015 +
    rtDW->DiscreteTimeIntegrator_DSTATE[0];

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  if (rtDW->DiscreteTimeIntegrator >= 4095.0) {
    rtDW->DiscreteTimeIntegrator = 4095.0;
  } else if (rtDW->DiscreteTimeIntegrator <= 0.0) {
    rtDW->DiscreteTimeIntegrator = 0.0;
  }

  /* Product: '<S5>/Divide1' incorporates:
   *  Product: '<S2>/Divide1'
   *  Product: '<S5>/Product3'
   *  Sum: '<S1>/Sum'
   *  UnitDelay: '<S5>/Unit Delay'
   */
  rtDW->rtb_RateLimiter_m_c = rtDW->rtb_Switch2_k * rtDW->UnitDelay_DSTATE[0] /
    rtDW->Sum[0];

  /* Lookup_n-D: '<S5>/1-D Lookup Table' incorporates:
   *  Product: '<S2>/Divide1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->Sum1 = look1_binlc(rtDW->Sum[0], rtConstP.uDLookupTable_bp01Data,
    rtConstP.uDLookupTable_tableData_g, 67U);

  /* Switch: '<S8>/Switch2' incorporates:
   *  Inport: '<Root>/Tx'
   *  RelationalOperator: '<S8>/LowerRelop1'
   */
  if (!(rtU->Tx[0] > rtDW->Sum1)) {
    /* Switch: '<S8>/Switch' incorporates:
     *  Gain: '<S5>/Gain3'
     *  RelationalOperator: '<S8>/UpperRelop'
     */
    if (rtU->Tx[0] < -rtDW->Sum1) {
      rtDW->Sum1 = -rtDW->Sum1;
    } else {
      rtDW->Sum1 = rtU->Tx[0];
    }
  }

  /* Switch: '<S9>/Switch2' incorporates:
   *  RelationalOperator: '<S9>/LowerRelop1'
   */
  if (!(rtDW->Sum1 > rtDW->rtb_RateLimiter_m_c)) {
    rtDW->rtb_RateLimiter_m_c = rtDW->Sum1;
  }

  /* DeadZone: '<S5>/Dead Zone' */
  if (rtDW->rtb_RateLimiter_m_c > 0.02) {
    rtDW->rtb_RateLimiter_m_c -= 0.02;
  } else if (rtDW->rtb_RateLimiter_m_c >= -0.02) {
    rtDW->rtb_RateLimiter_m_c = 0.0;
  } else {
    rtDW->rtb_RateLimiter_m_c -= -0.02;
  }

  /* RateLimiter: '<S5>/Rate Limiter' */
  rtDW->Sum1 = rtDW->rtb_RateLimiter_m_c - rtDW->PrevY[0];
  if (rtDW->Sum1 > 1.875) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[0] + 1.875;
  } else if (rtDW->Sum1 < -4.5) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[0] + -4.5;
  }

  rtDW->PrevY[0] = rtDW->rtb_RateLimiter_m_c;

  /* Switch: '<S7>/Switch2' */
  rtDW->rtb_Switch2_idx_0 = rtDW->rtb_Switch2_k;

  /* Product: '<S2>/Divide1' incorporates:
   *  Switch: '<S1>/Switch'
   */
  rtb_Switch_c_idx_0 = rtDW->rtb_Switch_c_b;

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_idx_0 = rtDW->DiscreteTimeIntegrator;

  /* Product: '<S5>/Divide1' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   */
  rtDW->rtb_RateLimiter_m_idx_0 = rtDW->rtb_RateLimiter_m_c;

  /* Gain: '<S5>/Gain3' incorporates:
   *  Abs: '<S3>/Abs2'
   *  Product: '<S3>/Divide'
   */
  rtb_Divide_l_idx_0 = fabs(rtDW->rtb_RateLimiter_m_c);

  /* Signum: '<S4>/Sign1' incorporates:
   *  Inport: '<Root>/Tx'
   *  Product: '<S4>/Product2'
   *  Switch: '<S9>/Switch'
   */
  if (rtIsNaN(rtU->Tx[1])) {
    rtDW->Sum1 = rtU->Tx[1];
  } else if (rtU->Tx[1] < 0.0) {
    rtDW->Sum1 = -1.0;
  } else {
    rtDW->Sum1 = (rtU->Tx[1] > 0.0);
  }

  /* Product: '<S4>/Product2' incorporates:
   *  Switch: '<S9>/Switch'
   */
  rtDW->Sum1 *= rtDW->Switch[1];

  /* Switch: '<S7>/Switch2' incorporates:
   *  Gain: '<S4>/Gain'
   *  Product: '<S4>/MatrixMultiply'
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  RelationalOperator: '<S7>/UpperRelop'
   *  Selector: '<S4>/Selector'
   *  Selector: '<S4>/Selector1'
   *  Switch: '<S7>/Switch'
   */
  if (rtDW->Sum1 > rtDW->MatrixMultiply[2]) {
    rtDW->rtb_Switch2_k = rtDW->MatrixMultiply[2];
  } else if (rtDW->Sum1 < -rtDW->MatrixMultiply[3]) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Gain: '<S4>/Gain'
     *  Selector: '<S4>/Selector1'
     */
    rtDW->rtb_Switch2_k = -rtDW->MatrixMultiply[3];
  } else {
    rtDW->rtb_Switch2_k = rtDW->Sum1;
  }

  /* Product: '<S2>/Divide1' incorporates:
   *  Inport: '<Root>/Motor_I'
   *  Inport: '<Root>/Vbatt'
   *  Product: '<S2>/Product1'
   *  Sum: '<S1>/Sum'
   *  Sum: '<S2>/Subtract'
   */
  rtDW->rtb_Switch_c_b = (rtDW->rtb_Switch2_k - rtU->Motor_V[1] * rtU->Motor_I[1])
    * (1.0 / rtDW->Sum[1]);

  /* Saturate: '<S2>/Saturation3' incorporates:
   *  DeadZone: '<S2>/Dead Zone'
   */
  if (rtDW->rtb_Switch_c_b > 0.01) {
    rtDW->rtb_Switch_c_b -= 0.01;
  } else if (rtDW->rtb_Switch_c_b >= -0.01) {
    rtDW->rtb_Switch_c_b = 0.0;
  } else {
    rtDW->rtb_Switch_c_b -= -0.01;
  }

  if (rtDW->rtb_Switch_c_b > 25.0) {
    rtDW->rtb_Switch_c_b = 25.0;
  } else if (rtDW->rtb_Switch_c_b < -25.0) {
    rtDW->rtb_Switch_c_b = -25.0;
  }

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S2>/Gain1'
   */
  rtDW->DiscreteTimeIntegrator = 200.0 * rtDW->rtb_Switch_c_b * 0.015 +
    rtDW->DiscreteTimeIntegrator_DSTATE[1];

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  if (rtDW->DiscreteTimeIntegrator >= 4095.0) {
    rtDW->DiscreteTimeIntegrator = 4095.0;
  } else if (rtDW->DiscreteTimeIntegrator <= 0.0) {
    rtDW->DiscreteTimeIntegrator = 0.0;
  }

  /* Product: '<S5>/Divide1' incorporates:
   *  Product: '<S2>/Divide1'
   *  Product: '<S5>/Product3'
   *  Sum: '<S1>/Sum'
   *  UnitDelay: '<S5>/Unit Delay'
   */
  rtDW->rtb_RateLimiter_m_c = rtDW->rtb_Switch2_k * rtDW->UnitDelay_DSTATE[1] /
    rtDW->Sum[1];

  /* Lookup_n-D: '<S5>/1-D Lookup Table' incorporates:
   *  Product: '<S2>/Divide1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->Sum1 = look1_binlc(rtDW->Sum[1], rtConstP.uDLookupTable_bp01Data,
    rtConstP.uDLookupTable_tableData_g, 67U);

  /* Switch: '<S8>/Switch2' incorporates:
   *  Inport: '<Root>/Tx'
   *  RelationalOperator: '<S8>/LowerRelop1'
   */
  if (!(rtU->Tx[1] > rtDW->Sum1)) {
    /* Switch: '<S8>/Switch' incorporates:
     *  Gain: '<S5>/Gain3'
     *  RelationalOperator: '<S8>/UpperRelop'
     */
    if (rtU->Tx[1] < -rtDW->Sum1) {
      rtDW->Sum1 = -rtDW->Sum1;
    } else {
      rtDW->Sum1 = rtU->Tx[1];
    }
  }

  /* Switch: '<S9>/Switch2' incorporates:
   *  RelationalOperator: '<S9>/LowerRelop1'
   */
  if (!(rtDW->Sum1 > rtDW->rtb_RateLimiter_m_c)) {
    rtDW->rtb_RateLimiter_m_c = rtDW->Sum1;
  }

  /* DeadZone: '<S5>/Dead Zone' */
  if (rtDW->rtb_RateLimiter_m_c > 0.02) {
    rtDW->rtb_RateLimiter_m_c -= 0.02;
  } else if (rtDW->rtb_RateLimiter_m_c >= -0.02) {
    rtDW->rtb_RateLimiter_m_c = 0.0;
  } else {
    rtDW->rtb_RateLimiter_m_c -= -0.02;
  }

  /* RateLimiter: '<S5>/Rate Limiter' */
  rtDW->Sum1 = rtDW->rtb_RateLimiter_m_c - rtDW->PrevY[1];
  if (rtDW->Sum1 > 1.875) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[1] + 1.875;
  } else if (rtDW->Sum1 < -4.5) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[1] + -4.5;
  }

  rtDW->PrevY[1] = rtDW->rtb_RateLimiter_m_c;

  /* Switch: '<S7>/Switch2' */
  rtDW->rtb_Switch2_idx_1 = rtDW->rtb_Switch2_k;

  /* Product: '<S2>/Divide1' incorporates:
   *  Switch: '<S1>/Switch'
   */
  rtb_Switch_c_idx_1 = rtDW->rtb_Switch_c_b;

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_idx_1 = rtDW->DiscreteTimeIntegrator;

  /* Product: '<S5>/Divide1' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   */
  rtDW->rtb_RateLimiter_m_idx_1 = rtDW->rtb_RateLimiter_m_c;

  /* Gain: '<S5>/Gain3' incorporates:
   *  Abs: '<S3>/Abs2'
   *  Product: '<S3>/Divide'
   */
  rtb_Divide_l_idx_1 = fabs(rtDW->rtb_RateLimiter_m_c);

  /* Signum: '<S4>/Sign1' incorporates:
   *  Inport: '<Root>/Tx'
   *  Product: '<S4>/Product2'
   *  Switch: '<S9>/Switch'
   */
  if (rtIsNaN(rtU->Tx[2])) {
    rtDW->Sum1 = rtU->Tx[2];
  } else if (rtU->Tx[2] < 0.0) {
    rtDW->Sum1 = -1.0;
  } else {
    rtDW->Sum1 = (rtU->Tx[2] > 0.0);
  }

  /* Product: '<S4>/Product2' incorporates:
   *  Switch: '<S9>/Switch'
   */
  rtDW->Sum1 *= rtDW->Switch[2];

  /* Switch: '<S7>/Switch2' incorporates:
   *  Gain: '<S4>/Gain'
   *  Product: '<S4>/MatrixMultiply'
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  RelationalOperator: '<S7>/UpperRelop'
   *  Selector: '<S4>/Selector'
   *  Selector: '<S4>/Selector1'
   *  Switch: '<S7>/Switch'
   */
  if (rtDW->Sum1 > rtDW->MatrixMultiply[4]) {
    rtDW->rtb_Switch2_k = rtDW->MatrixMultiply[4];
  } else if (rtDW->Sum1 < -rtDW->MatrixMultiply[5]) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Gain: '<S4>/Gain'
     *  Selector: '<S4>/Selector1'
     */
    rtDW->rtb_Switch2_k = -rtDW->MatrixMultiply[5];
  } else {
    rtDW->rtb_Switch2_k = rtDW->Sum1;
  }

  /* Product: '<S2>/Divide1' incorporates:
   *  Inport: '<Root>/Motor_I'
   *  Inport: '<Root>/Vbatt'
   *  Product: '<S2>/Product1'
   *  Sum: '<S1>/Sum'
   *  Sum: '<S2>/Subtract'
   */
  rtDW->rtb_Switch_c_b = (rtDW->rtb_Switch2_k - rtU->Motor_V[2] * rtU->Motor_I[2])
    * (1.0 / rtDW->Sum[2]);

  /* Saturate: '<S2>/Saturation3' incorporates:
   *  DeadZone: '<S2>/Dead Zone'
   */
  if (rtDW->rtb_Switch_c_b > 0.01) {
    rtDW->rtb_Switch_c_b -= 0.01;
  } else if (rtDW->rtb_Switch_c_b >= -0.01) {
    rtDW->rtb_Switch_c_b = 0.0;
  } else {
    rtDW->rtb_Switch_c_b -= -0.01;
  }

  if (rtDW->rtb_Switch_c_b > 25.0) {
    rtDW->rtb_Switch_c_b = 25.0;
  } else if (rtDW->rtb_Switch_c_b < -25.0) {
    rtDW->rtb_Switch_c_b = -25.0;
  }

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S2>/Gain1'
   */
  rtDW->DiscreteTimeIntegrator = 200.0 * rtDW->rtb_Switch_c_b * 0.015 +
    rtDW->DiscreteTimeIntegrator_DSTATE[2];

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  if (rtDW->DiscreteTimeIntegrator >= 4095.0) {
    rtDW->DiscreteTimeIntegrator = 4095.0;
  } else if (rtDW->DiscreteTimeIntegrator <= 0.0) {
    rtDW->DiscreteTimeIntegrator = 0.0;
  }

  /* Product: '<S5>/Divide1' incorporates:
   *  Product: '<S2>/Divide1'
   *  Product: '<S5>/Product3'
   *  Sum: '<S1>/Sum'
   *  UnitDelay: '<S5>/Unit Delay'
   */
  rtDW->rtb_RateLimiter_m_c = rtDW->rtb_Switch2_k * rtDW->UnitDelay_DSTATE[2] /
    rtDW->Sum[2];

  /* Lookup_n-D: '<S5>/1-D Lookup Table' incorporates:
   *  Product: '<S2>/Divide1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->Sum1 = look1_binlc(rtDW->Sum[2], rtConstP.uDLookupTable_bp01Data,
    rtConstP.uDLookupTable_tableData_g, 67U);

  /* Switch: '<S8>/Switch2' incorporates:
   *  Inport: '<Root>/Tx'
   *  RelationalOperator: '<S8>/LowerRelop1'
   */
  if (!(rtU->Tx[2] > rtDW->Sum1)) {
    /* Switch: '<S8>/Switch' incorporates:
     *  Gain: '<S5>/Gain3'
     *  RelationalOperator: '<S8>/UpperRelop'
     */
    if (rtU->Tx[2] < -rtDW->Sum1) {
      rtDW->Sum1 = -rtDW->Sum1;
    } else {
      rtDW->Sum1 = rtU->Tx[2];
    }
  }

  /* Switch: '<S9>/Switch2' incorporates:
   *  RelationalOperator: '<S9>/LowerRelop1'
   */
  if (!(rtDW->Sum1 > rtDW->rtb_RateLimiter_m_c)) {
    rtDW->rtb_RateLimiter_m_c = rtDW->Sum1;
  }

  /* DeadZone: '<S5>/Dead Zone' */
  if (rtDW->rtb_RateLimiter_m_c > 0.02) {
    rtDW->rtb_RateLimiter_m_c -= 0.02;
  } else if (rtDW->rtb_RateLimiter_m_c >= -0.02) {
    rtDW->rtb_RateLimiter_m_c = 0.0;
  } else {
    rtDW->rtb_RateLimiter_m_c -= -0.02;
  }

  /* RateLimiter: '<S5>/Rate Limiter' */
  rtDW->Sum1 = rtDW->rtb_RateLimiter_m_c - rtDW->PrevY[2];
  if (rtDW->Sum1 > 1.875) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[2] + 1.875;
  } else if (rtDW->Sum1 < -4.5) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[2] + -4.5;
  }

  rtDW->PrevY[2] = rtDW->rtb_RateLimiter_m_c;

  /* Switch: '<S7>/Switch2' */
  rtDW->rtb_Switch2_idx_2 = rtDW->rtb_Switch2_k;

  /* Product: '<S2>/Divide1' incorporates:
   *  Switch: '<S1>/Switch'
   */
  rtb_Switch_c_idx_2 = rtDW->rtb_Switch_c_b;

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_idx_2 = rtDW->DiscreteTimeIntegrator;

  /* Product: '<S5>/Divide1' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   */
  rtb_RateLimiter_m_idx_2 = rtDW->rtb_RateLimiter_m_c;

  /* Gain: '<S5>/Gain3' incorporates:
   *  Abs: '<S3>/Abs2'
   *  Product: '<S3>/Divide'
   */
  rtb_Divide_l_idx_2 = fabs(rtDW->rtb_RateLimiter_m_c);

  /* Signum: '<S4>/Sign1' incorporates:
   *  Inport: '<Root>/Tx'
   *  Product: '<S4>/Product2'
   *  Switch: '<S9>/Switch'
   */
  if (rtIsNaN(rtU->Tx[3])) {
    rtDW->Sum1 = rtU->Tx[3];
  } else if (rtU->Tx[3] < 0.0) {
    rtDW->Sum1 = -1.0;
  } else {
    rtDW->Sum1 = (rtU->Tx[3] > 0.0);
  }

  /* Product: '<S4>/Product2' incorporates:
   *  Lookup_n-D: '<S4>/2-D Lookup Table'
   */
  rtDW->Sum1 *= intrp2d_la(rtDW->bpIndices, rtDW->fractions,
    rtConstP.uDLookupTable_tableData, 16U, rtConstP.pooled6);

  /* Switch: '<S7>/Switch2' incorporates:
   *  Gain: '<S4>/Gain'
   *  Product: '<S4>/MatrixMultiply'
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  RelationalOperator: '<S7>/UpperRelop'
   *  Selector: '<S4>/Selector'
   *  Selector: '<S4>/Selector1'
   *  Switch: '<S7>/Switch'
   */
  if (rtDW->Sum1 > rtDW->MatrixMultiply[6]) {
    rtDW->rtb_Switch2_k = rtDW->MatrixMultiply[6];
  } else if (rtDW->Sum1 < -rtDW->MatrixMultiply[7]) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Gain: '<S4>/Gain'
     *  Selector: '<S4>/Selector1'
     */
    rtDW->rtb_Switch2_k = -rtDW->MatrixMultiply[7];
  } else {
    rtDW->rtb_Switch2_k = rtDW->Sum1;
  }

  /* Product: '<S2>/Divide1' incorporates:
   *  Inport: '<Root>/Motor_I'
   *  Inport: '<Root>/Vbatt'
   *  Product: '<S2>/Product1'
   *  Sum: '<S1>/Sum'
   *  Sum: '<S2>/Subtract'
   */
  rtDW->rtb_Switch_c_b = (rtDW->rtb_Switch2_k - rtU->Motor_V[3] * rtU->Motor_I[3])
    * (1.0 / rtDW->Sum[3]);

  /* Saturate: '<S2>/Saturation3' incorporates:
   *  DeadZone: '<S2>/Dead Zone'
   */
  if (rtDW->rtb_Switch_c_b > 0.01) {
    rtDW->rtb_Switch_c_b -= 0.01;
  } else if (rtDW->rtb_Switch_c_b >= -0.01) {
    rtDW->rtb_Switch_c_b = 0.0;
  } else {
    rtDW->rtb_Switch_c_b -= -0.01;
  }

  if (rtDW->rtb_Switch_c_b > 25.0) {
    rtDW->rtb_Switch_c_b = 25.0;
  } else if (rtDW->rtb_Switch_c_b < -25.0) {
    rtDW->rtb_Switch_c_b = -25.0;
  }

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S2>/Gain1'
   */
  rtDW->DiscreteTimeIntegrator = 200.0 * rtDW->rtb_Switch_c_b * 0.015 +
    rtDW->DiscreteTimeIntegrator_DSTATE[3];

  /* DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  if (rtDW->DiscreteTimeIntegrator >= 4095.0) {
    rtDW->DiscreteTimeIntegrator = 4095.0;
  } else if (rtDW->DiscreteTimeIntegrator <= 0.0) {
    rtDW->DiscreteTimeIntegrator = 0.0;
  }

  /* Product: '<S5>/Divide1' incorporates:
   *  Product: '<S2>/Divide1'
   *  Product: '<S5>/Product3'
   *  Sum: '<S1>/Sum'
   *  UnitDelay: '<S5>/Unit Delay'
   */
  rtDW->rtb_RateLimiter_m_c = rtDW->rtb_Switch2_k * rtDW->UnitDelay_DSTATE[3] /
    rtDW->Sum[3];

  /* Lookup_n-D: '<S5>/1-D Lookup Table' incorporates:
   *  Product: '<S2>/Divide1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->Sum1 = look1_binlc(rtDW->Sum[3], rtConstP.uDLookupTable_bp01Data,
    rtConstP.uDLookupTable_tableData_g, 67U);

  /* Switch: '<S8>/Switch2' incorporates:
   *  Inport: '<Root>/Tx'
   *  RelationalOperator: '<S8>/LowerRelop1'
   */
  if (!(rtU->Tx[3] > rtDW->Sum1)) {
    /* Switch: '<S8>/Switch' incorporates:
     *  Gain: '<S5>/Gain3'
     *  RelationalOperator: '<S8>/UpperRelop'
     */
    if (rtU->Tx[3] < -rtDW->Sum1) {
      rtDW->Sum1 = -rtDW->Sum1;
    } else {
      rtDW->Sum1 = rtU->Tx[3];
    }
  }

  /* Switch: '<S9>/Switch2' incorporates:
   *  RelationalOperator: '<S9>/LowerRelop1'
   */
  if (!(rtDW->Sum1 > rtDW->rtb_RateLimiter_m_c)) {
    rtDW->rtb_RateLimiter_m_c = rtDW->Sum1;
  }

  /* DeadZone: '<S5>/Dead Zone' */
  if (rtDW->rtb_RateLimiter_m_c > 0.02) {
    rtDW->rtb_RateLimiter_m_c -= 0.02;
  } else if (rtDW->rtb_RateLimiter_m_c >= -0.02) {
    rtDW->rtb_RateLimiter_m_c = 0.0;
  } else {
    rtDW->rtb_RateLimiter_m_c -= -0.02;
  }

  /* RateLimiter: '<S5>/Rate Limiter' */
  rtDW->Sum1 = rtDW->rtb_RateLimiter_m_c - rtDW->PrevY[3];
  if (rtDW->Sum1 > 1.875) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[3] + 1.875;
  } else if (rtDW->Sum1 < -4.5) {
    rtDW->rtb_RateLimiter_m_c = rtDW->PrevY[3] + -4.5;
  }

  rtDW->PrevY[3] = rtDW->rtb_RateLimiter_m_c;

  /* Lookup_n-D: '<S3>/2-D Lookup Table1' incorporates:
   *  Abs: '<S3>/Abs2'
   *  Product: '<S3>/Divide'
   *  Sum: '<S1>/Sum'
   */
  rtDW->bpIndices_p[0U] = plook_evenca(rtb_Divide_l_idx_0, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_m[0U] = rtDW->Sum1;
  rtDW->bpIndices_p[1U] = plook_evenca(rtDW->Sum[0], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_m[1U] = rtDW->Sum1;
  rtb_Divide_l_idx_0 = intrp2d_la(rtDW->bpIndices_p, rtDW->fractions_m,
    rtConstP.uDLookupTable1_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices_p[0U] = plook_evenca(rtb_Divide_l_idx_1, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_m[0U] = rtDW->Sum1;
  rtDW->bpIndices_p[1U] = plook_evenca(rtDW->Sum[1], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_m[1U] = rtDW->Sum1;
  rtb_Divide_l_idx_1 = intrp2d_la(rtDW->bpIndices_p, rtDW->fractions_m,
    rtConstP.uDLookupTable1_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices_p[0U] = plook_evenca(rtb_Divide_l_idx_2, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_m[0U] = rtDW->Sum1;
  rtDW->bpIndices_p[1U] = plook_evenca(rtDW->Sum[2], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_m[1U] = rtDW->Sum1;
  rtb_Divide_l_idx_2 = intrp2d_la(rtDW->bpIndices_p, rtDW->fractions_m,
    rtConstP.uDLookupTable1_tableData, 16U, rtConstP.pooled6);
  rtDW->bpIndices_p[0U] = plook_evenca(fabs(rtDW->rtb_RateLimiter_m_c), 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_m[0U] = rtDW->Sum1;
  rtDW->bpIndices_p[1U] = plook_evenca(rtDW->Sum[3], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_m[1U] = rtDW->Sum1;

  /* Switch: '<S1>/Switch' incorporates:
   *  Gain: '<S3>/Gain1'
   *  Product: '<S3>/Product1'
   *  Sum: '<S1>/Sum'
   */
  if (rtDW->Sum[0] > rtP.feebackThreshhold) {
    /* Saturate: '<S2>/Saturation2' incorporates:
     *  DiscreteIntegrator: '<S2>/Discrete-Time Integrator'
     *  Gain: '<S2>/Gain'
     *  Sum: '<S2>/Sum1'
     *  Switch: '<S1>/Switch'
     */
    rtb_Switch_c_idx_0 = 5.0 * rtb_Switch_c_idx_0 +
      rtDW->DiscreteTimeIntegrator_idx_0;
    if (rtb_Switch_c_idx_0 > 4095.0) {
      rtb_Switch_c_idx_0 = 4095.0;
    } else if (rtb_Switch_c_idx_0 < 0.0) {
      rtb_Switch_c_idx_0 = 0.0;
    }
  } else {
    /* MinMax: '<S3>/Min' incorporates:
     *  Inport: '<Root>/Vbatt'
     *  Product: '<S3>/Divide'
     */
    rtb_Switch_c_idx_0 = rtb_Divide_l_idx_0 / rtU->Motor_V[0];

    /* Signum: '<S3>/Sign2' incorporates:
     *  RateLimiter: '<S5>/Rate Limiter'
     */
    if (rtIsNaN(rtDW->rtb_RateLimiter_m_idx_0)) {
      rtDW->Sum1 = rtDW->rtb_RateLimiter_m_idx_0;
    } else if (rtDW->rtb_RateLimiter_m_idx_0 < 0.0) {
      rtDW->Sum1 = -1.0;
    } else {
      rtDW->Sum1 = (rtDW->rtb_RateLimiter_m_idx_0 > 0.0);
    }

    /* MinMax: '<S3>/Min' */
    if (!(rtb_Switch_c_idx_0 <= 1.0)) {
      rtb_Switch_c_idx_0 = 1.0;
    }

    rtb_Switch_c_idx_0 = rtDW->Sum1 * rtb_Switch_c_idx_0 * 4095.0;
  }

  /* RateLimiter: '<S1>/Rate Limiter' */
  rtDW->Sum1 = rtb_Switch_c_idx_0 - rtDW->PrevY_l[0];
  if (rtDW->Sum1 > 37.5) {
    rtDW->Sum1 = rtDW->PrevY_l[0] + 37.5;
  } else if (rtDW->Sum1 < -90.0) {
    rtDW->Sum1 = rtDW->PrevY_l[0] + -90.0;
  } else {
    rtDW->Sum1 = rtb_Switch_c_idx_0;
  }

  rtDW->PrevY_l[0] = rtDW->Sum1;

  /* Outport: '<Root>/k' */
  rtY->k[0] = rtDW->Sum1;

  /* Outport: '<Root>/T' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Signum: '<S3>/Sign2'
   */
  rtY->T[0] = rtDW->rtb_RateLimiter_m_idx_0;

  /* Switch: '<S1>/Switch' incorporates:
   *  Gain: '<S3>/Gain1'
   *  Product: '<S3>/Product1'
   *  Sum: '<S1>/Sum'
   */
  if (rtDW->Sum[1] > rtP.feebackThreshhold) {
    /* Saturate: '<S2>/Saturation2' incorporates:
     *  DiscreteIntegrator: '<S2>/Discrete-Time Integrator'
     *  Gain: '<S2>/Gain'
     *  Sum: '<S2>/Sum1'
     *  Switch: '<S1>/Switch'
     */
    rtb_Switch_c_idx_0 = 5.0 * rtb_Switch_c_idx_1 +
      rtDW->DiscreteTimeIntegrator_idx_1;
    if (rtb_Switch_c_idx_0 > 4095.0) {
      rtb_Switch_c_idx_0 = 4095.0;
    } else if (rtb_Switch_c_idx_0 < 0.0) {
      rtb_Switch_c_idx_0 = 0.0;
    }
  } else {
    /* MinMax: '<S3>/Min' incorporates:
     *  Inport: '<Root>/Vbatt'
     *  Product: '<S3>/Divide'
     */
    rtb_Switch_c_idx_0 = rtb_Divide_l_idx_1 / rtU->Motor_V[1];

    /* Signum: '<S3>/Sign2' incorporates:
     *  RateLimiter: '<S5>/Rate Limiter'
     */
    if (rtIsNaN(rtDW->rtb_RateLimiter_m_idx_1)) {
      rtDW->Sum1 = rtDW->rtb_RateLimiter_m_idx_1;
    } else if (rtDW->rtb_RateLimiter_m_idx_1 < 0.0) {
      rtDW->Sum1 = -1.0;
    } else {
      rtDW->Sum1 = (rtDW->rtb_RateLimiter_m_idx_1 > 0.0);
    }

    /* MinMax: '<S3>/Min' */
    if (!(rtb_Switch_c_idx_0 <= 1.0)) {
      rtb_Switch_c_idx_0 = 1.0;
    }

    rtb_Switch_c_idx_0 = rtDW->Sum1 * rtb_Switch_c_idx_0 * 4095.0;
  }

  /* RateLimiter: '<S1>/Rate Limiter' */
  rtDW->Sum1 = rtb_Switch_c_idx_0 - rtDW->PrevY_l[1];
  if (rtDW->Sum1 > 37.5) {
    rtDW->Sum1 = rtDW->PrevY_l[1] + 37.5;
  } else if (rtDW->Sum1 < -90.0) {
    rtDW->Sum1 = rtDW->PrevY_l[1] + -90.0;
  } else {
    rtDW->Sum1 = rtb_Switch_c_idx_0;
  }

  rtDW->PrevY_l[1] = rtDW->Sum1;

  /* Outport: '<Root>/k' */
  rtY->k[1] = rtDW->Sum1;

  /* Outport: '<Root>/T' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Signum: '<S3>/Sign2'
   */
  rtY->T[1] = rtDW->rtb_RateLimiter_m_idx_1;

  /* Switch: '<S1>/Switch' incorporates:
   *  Gain: '<S3>/Gain1'
   *  Product: '<S3>/Product1'
   *  Sum: '<S1>/Sum'
   */
  if (rtDW->Sum[2] > rtP.feebackThreshhold) {
    /* Saturate: '<S2>/Saturation2' incorporates:
     *  DiscreteIntegrator: '<S2>/Discrete-Time Integrator'
     *  Gain: '<S2>/Gain'
     *  Sum: '<S2>/Sum1'
     *  Switch: '<S1>/Switch'
     */
    rtb_Switch_c_idx_0 = 5.0 * rtb_Switch_c_idx_2 +
      rtDW->DiscreteTimeIntegrator_idx_2;
    if (rtb_Switch_c_idx_0 > 4095.0) {
      rtb_Switch_c_idx_0 = 4095.0;
    } else if (rtb_Switch_c_idx_0 < 0.0) {
      rtb_Switch_c_idx_0 = 0.0;
    }
  } else {
    /* MinMax: '<S3>/Min' incorporates:
     *  Inport: '<Root>/Vbatt'
     *  Product: '<S3>/Divide'
     */
    rtb_Switch_c_idx_0 = rtb_Divide_l_idx_2 / rtU->Motor_V[2];

    /* Signum: '<S3>/Sign2' incorporates:
     *  RateLimiter: '<S5>/Rate Limiter'
     */
    if (rtIsNaN(rtb_RateLimiter_m_idx_2)) {
      rtDW->Sum1 = rtb_RateLimiter_m_idx_2;
    } else if (rtb_RateLimiter_m_idx_2 < 0.0) {
      rtDW->Sum1 = -1.0;
    } else {
      rtDW->Sum1 = (rtb_RateLimiter_m_idx_2 > 0.0);
    }

    /* MinMax: '<S3>/Min' */
    if (!(rtb_Switch_c_idx_0 <= 1.0)) {
      rtb_Switch_c_idx_0 = 1.0;
    }

    rtb_Switch_c_idx_0 = rtDW->Sum1 * rtb_Switch_c_idx_0 * 4095.0;
  }

  /* RateLimiter: '<S1>/Rate Limiter' */
  rtDW->Sum1 = rtb_Switch_c_idx_0 - rtDW->PrevY_l[2];
  if (rtDW->Sum1 > 37.5) {
    rtDW->Sum1 = rtDW->PrevY_l[2] + 37.5;
  } else if (rtDW->Sum1 < -90.0) {
    rtDW->Sum1 = rtDW->PrevY_l[2] + -90.0;
  } else {
    rtDW->Sum1 = rtb_Switch_c_idx_0;
  }

  rtDW->PrevY_l[2] = rtDW->Sum1;

  /* Outport: '<Root>/k' */
  rtY->k[2] = rtDW->Sum1;

  /* Outport: '<Root>/T' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Signum: '<S3>/Sign2'
   */
  rtY->T[2] = rtb_RateLimiter_m_idx_2;

  /* Switch: '<S1>/Switch' incorporates:
   *  Gain: '<S3>/Gain1'
   *  Product: '<S3>/Product1'
   *  Sum: '<S1>/Sum'
   */
  if (rtDW->Sum[3] > rtP.feebackThreshhold) {
    /* Saturate: '<S2>/Saturation2' incorporates:
     *  Gain: '<S2>/Gain'
     *  Sum: '<S2>/Sum1'
     */
    rtb_Switch_c_idx_0 = 5.0 * rtDW->rtb_Switch_c_b +
      rtDW->DiscreteTimeIntegrator;
    if (rtb_Switch_c_idx_0 > 4095.0) {
      rtb_Switch_c_idx_0 = 4095.0;
    } else if (rtb_Switch_c_idx_0 < 0.0) {
      rtb_Switch_c_idx_0 = 0.0;
    }
  } else {
    /* MinMax: '<S3>/Min' incorporates:
     *  Inport: '<Root>/Vbatt'
     *  Lookup_n-D: '<S3>/2-D Lookup Table1'
     *  Product: '<S3>/Divide'
     */
    rtb_Switch_c_idx_0 = intrp2d_la(rtDW->bpIndices_p, rtDW->fractions_m,
      rtConstP.uDLookupTable1_tableData, 16U, rtConstP.pooled6) / rtU->Motor_V[3];

    /* Signum: '<S3>/Sign2' */
    if (rtIsNaN(rtDW->rtb_RateLimiter_m_c)) {
      rtDW->Sum1 = rtDW->rtb_RateLimiter_m_c;
    } else if (rtDW->rtb_RateLimiter_m_c < 0.0) {
      rtDW->Sum1 = -1.0;
    } else {
      rtDW->Sum1 = (rtDW->rtb_RateLimiter_m_c > 0.0);
    }

    /* MinMax: '<S3>/Min' */
    if (!(rtb_Switch_c_idx_0 <= 1.0)) {
      rtb_Switch_c_idx_0 = 1.0;
    }

    rtb_Switch_c_idx_0 = rtDW->Sum1 * rtb_Switch_c_idx_0 * 4095.0;
  }

  /* RateLimiter: '<S1>/Rate Limiter' */
  rtDW->Sum1 = rtb_Switch_c_idx_0 - rtDW->PrevY_l[3];
  if (rtDW->Sum1 > 37.5) {
    rtDW->Sum1 = rtDW->PrevY_l[3] + 37.5;
  } else if (rtDW->Sum1 < -90.0) {
    rtDW->Sum1 = rtDW->PrevY_l[3] + -90.0;
  } else {
    rtDW->Sum1 = rtb_Switch_c_idx_0;
  }

  rtDW->PrevY_l[3] = rtDW->Sum1;

  /* Outport: '<Root>/k' */
  rtY->k[3] = rtDW->Sum1;

  /* Outport: '<Root>/T' */
  rtY->T[3] = rtDW->rtb_RateLimiter_m_c;

  /* Lookup_n-D: '<S5>/2-D Lookup Table' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Sum: '<S1>/Sum'
   */
  rtDW->bpIndices_c[0U] = plook_evenc(rtDW->rtb_RateLimiter_m_idx_0, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_c[0U] = rtDW->Sum1;
  rtDW->bpIndices_c[1U] = plook_evenc(rtDW->Sum[0], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_c[1U] = rtDW->Sum1;

  /* Update for UnitDelay: '<S5>/Unit Delay' incorporates:
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   */
  rtDW->UnitDelay_DSTATE[0] = intrp2d_l(rtDW->bpIndices_c, rtDW->fractions_c,
    rtConstP.uDLookupTable_tableData_b, 16U);

  /* Lookup_n-D: '<S5>/2-D Lookup Table' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Sum: '<S1>/Sum'
   */
  rtDW->bpIndices_c[0U] = plook_evenc(rtDW->rtb_RateLimiter_m_idx_1, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_c[0U] = rtDW->Sum1;
  rtDW->bpIndices_c[1U] = plook_evenc(rtDW->Sum[1], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_c[1U] = rtDW->Sum1;

  /* Update for UnitDelay: '<S5>/Unit Delay' incorporates:
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   */
  rtDW->UnitDelay_DSTATE[1] = intrp2d_l(rtDW->bpIndices_c, rtDW->fractions_c,
    rtConstP.uDLookupTable_tableData_b, 16U);

  /* Lookup_n-D: '<S5>/2-D Lookup Table' incorporates:
   *  RateLimiter: '<S5>/Rate Limiter'
   *  Sum: '<S1>/Sum'
   */
  rtDW->bpIndices_c[0U] = plook_evenc(rtb_RateLimiter_m_idx_2, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_c[0U] = rtDW->Sum1;
  rtDW->bpIndices_c[1U] = plook_evenc(rtDW->Sum[2], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_c[1U] = rtDW->Sum1;

  /* Update for UnitDelay: '<S5>/Unit Delay' incorporates:
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   */
  rtDW->UnitDelay_DSTATE[2] = intrp2d_l(rtDW->bpIndices_c, rtDW->fractions_c,
    rtConstP.uDLookupTable_tableData_b, 16U);

  /* Lookup_n-D: '<S5>/2-D Lookup Table' incorporates:
   *  Sum: '<S1>/Sum'
   */
  rtDW->bpIndices_c[0U] = plook_evenc(rtDW->rtb_RateLimiter_m_c, 0.0,
    1.6666666666666665, 15U, &rtDW->Sum1);
  rtDW->fractions_c[0U] = rtDW->Sum1;
  rtDW->bpIndices_c[1U] = plook_evenc(rtDW->Sum[3], 0.0, 10.4719755, 106U,
    &rtDW->Sum1);
  rtDW->fractions_c[1U] = rtDW->Sum1;

  /* Outport: '<Root>/P' incorporates:
   *  Switch: '<S7>/Switch2'
   */
  rtY->P_g[0] = rtDW->rtb_Switch2_idx_0;

  /* Outport: '<Root>/r' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Outport: '<Root>/P'
   *  Product: '<S4>/Divide1'
   *  Sum: '<S1>/Sum'
   *  Switch: '<S7>/Switch2'
   */
  rtY->r[0] = 10.0 * rtDW->Sum[0] / rtDW->rtb_Switch2_idx_0;

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->UnitDelay1_DSTATE[0] = rtDW->Sum[0];

  /* Update for DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_DSTATE[0] = rtDW->DiscreteTimeIntegrator_idx_0;

  /* Outport: '<Root>/P' incorporates:
   *  Switch: '<S7>/Switch2'
   */
  rtY->P_g[1] = rtDW->rtb_Switch2_idx_1;

  /* Outport: '<Root>/r' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Outport: '<Root>/P'
   *  Product: '<S4>/Divide1'
   *  Sum: '<S1>/Sum'
   *  Switch: '<S7>/Switch2'
   */
  rtY->r[1] = 10.0 * rtDW->Sum[1] / rtDW->rtb_Switch2_idx_1;

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->UnitDelay1_DSTATE[1] = rtDW->Sum[1];

  /* Update for DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_DSTATE[1] = rtDW->DiscreteTimeIntegrator_idx_1;

  /* Outport: '<Root>/P' incorporates:
   *  Switch: '<S7>/Switch2'
   */
  rtY->P_g[2] = rtDW->rtb_Switch2_idx_2;

  /* Outport: '<Root>/r' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Outport: '<Root>/P'
   *  Product: '<S4>/Divide1'
   *  Sum: '<S1>/Sum'
   *  Switch: '<S7>/Switch2'
   */
  rtY->r[2] = 10.0 * rtDW->Sum[2] / rtDW->rtb_Switch2_idx_2;

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->UnitDelay1_DSTATE[2] = rtDW->Sum[2];

  /* Update for DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_DSTATE[2] = rtDW->DiscreteTimeIntegrator_idx_2;

  /* Outport: '<Root>/P' */
  rtY->P_g[3] = rtDW->rtb_Switch2_k;

  /* Outport: '<Root>/r' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Product: '<S4>/Divide1'
   *  Sum: '<S1>/Sum'
   */
  rtY->r[3] = 10.0 * rtDW->Sum[3] / rtDW->rtb_Switch2_k;

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  Gain: '<S4>/Gain1'
   *  Sum: '<S1>/Sum'
   */
  rtDW->UnitDelay1_DSTATE[3] = rtDW->Sum[3];

  /* Update for DiscreteIntegrator: '<S2>/Discrete-Time Integrator' */
  rtDW->DiscreteTimeIntegrator_DSTATE[3] = rtDW->DiscreteTimeIntegrator;

  /* Update for UnitDelay: '<S5>/Unit Delay' incorporates:
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   */
  rtDW->UnitDelay_DSTATE[3] = intrp2d_l(rtDW->bpIndices_c, rtDW->fractions_c,
    rtConstP.uDLookupTable_tableData_b, 16U);
}

/* Model initialize function */
void MC_PL0_initialize(RT_MODEL *const rtM)
{
  DW *rtDW = rtM->dwork;

  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* InitializeConditions for UnitDelay: '<S5>/Unit Delay' */
  rtDW->UnitDelay_DSTATE[0] = 0.31;
  rtDW->UnitDelay_DSTATE[1] = 0.31;
  rtDW->UnitDelay_DSTATE[2] = 0.31;
  rtDW->UnitDelay_DSTATE[3] = 0.31;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
