/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: tv_types.h
 *
 * Code generated for Simulink model 'tv'.
 *
 * Model version                  : 1.31
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Sat Apr 20 09:30:06 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_tv_types_h_
#define RTW_HEADER_tv_types_h_
#ifndef struct_tag_2ykk7BbtZEWHYu0XKNZ1LE
#define struct_tag_2ykk7BbtZEWHYu0XKNZ1LE

struct tag_2ykk7BbtZEWHYu0XKNZ1LE
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pCumSum;
  real_T pCumSumRev[19];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_2ykk7BbtZEWHYu0XKNZ1LE */

#ifndef typedef_h_dsp_internal_SlidingWindow_tv
#define typedef_h_dsp_internal_SlidingWindow_tv

typedef struct tag_2ykk7BbtZEWHYu0XKNZ1LE h_dsp_internal_SlidingWindow_tv;

#endif                             /* typedef_h_dsp_internal_SlidingWindow_tv */

#ifndef struct_tag_J38farKz2epFg1DFUoymjH
#define struct_tag_J38farKz2epFg1DFUoymjH

struct tag_J38farKz2epFg1DFUoymjH
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pCumSum;
  real_T pCumSumRev[5];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_J38farKz2epFg1DFUoymjH */

#ifndef typedef_h_dsp_internal_SlidingWind_j_tv
#define typedef_h_dsp_internal_SlidingWind_j_tv

typedef struct tag_J38farKz2epFg1DFUoymjH h_dsp_internal_SlidingWind_j_tv;

#endif                             /* typedef_h_dsp_internal_SlidingWind_j_tv */

#ifndef struct_tag_BlgwLpgj2bjudmbmVKWwDE
#define struct_tag_BlgwLpgj2bjudmbmVKWwDE

struct tag_BlgwLpgj2bjudmbmVKWwDE
{
  uint32_T f1[8];
};

#endif                                 /* struct_tag_BlgwLpgj2bjudmbmVKWwDE */

#ifndef typedef_cell_wrap_tv
#define typedef_cell_wrap_tv

typedef struct tag_BlgwLpgj2bjudmbmVKWwDE cell_wrap_tv;

#endif                                 /* typedef_cell_wrap_tv */

#ifndef struct_tag_A9AnQ4YrsYeUCGYxAP6xUF
#define struct_tag_A9AnQ4YrsYeUCGYxAP6xUF

struct tag_A9AnQ4YrsYeUCGYxAP6xUF
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_tv inputVarSize;
  h_dsp_internal_SlidingWindow_tv *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  h_dsp_internal_SlidingWindow_tv _pobj0;
};

#endif                                 /* struct_tag_A9AnQ4YrsYeUCGYxAP6xUF */

#ifndef typedef_dsp_simulink_MovingAverage_tv
#define typedef_dsp_simulink_MovingAverage_tv

typedef struct tag_A9AnQ4YrsYeUCGYxAP6xUF dsp_simulink_MovingAverage_tv;

#endif                               /* typedef_dsp_simulink_MovingAverage_tv */

#ifndef struct_tag_nCn326iMCNAOlldGGgUCWC
#define struct_tag_nCn326iMCNAOlldGGgUCWC

struct tag_nCn326iMCNAOlldGGgUCWC
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_tv inputVarSize;
  h_dsp_internal_SlidingWind_j_tv *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  h_dsp_internal_SlidingWind_j_tv _pobj0;
};

#endif                                 /* struct_tag_nCn326iMCNAOlldGGgUCWC */

#ifndef typedef_dsp_simulink_MovingAverage_j_tv
#define typedef_dsp_simulink_MovingAverage_j_tv

typedef struct tag_nCn326iMCNAOlldGGgUCWC dsp_simulink_MovingAverage_j_tv;

#endif                             /* typedef_dsp_simulink_MovingAverage_j_tv */

/* Parameters (default storage) */
typedef struct P_tv_ P_tv;

/* Forward declaration for rtModel */
typedef struct tag_RTM_tv RT_MODEL_tv;

#endif                                 /* RTW_HEADER_tv_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
