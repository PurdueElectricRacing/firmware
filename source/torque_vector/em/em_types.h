/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em_types.h
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

#ifndef RTW_HEADER_em_types_h_
#define RTW_HEADER_em_types_h_
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

#ifndef typedef_h_dsp_internal_SlidingWindow_em
#define typedef_h_dsp_internal_SlidingWindow_em

typedef struct tag_J38farKz2epFg1DFUoymjH h_dsp_internal_SlidingWindow_em;

#endif                             /* typedef_h_dsp_internal_SlidingWindow_em */

#ifndef struct_tag_BlgwLpgj2bjudmbmVKWwDE
#define struct_tag_BlgwLpgj2bjudmbmVKWwDE

struct tag_BlgwLpgj2bjudmbmVKWwDE
{
  uint32_T f1[8];
};

#endif                                 /* struct_tag_BlgwLpgj2bjudmbmVKWwDE */

#ifndef typedef_cell_wrap_em
#define typedef_cell_wrap_em

typedef struct tag_BlgwLpgj2bjudmbmVKWwDE cell_wrap_em;

#endif                                 /* typedef_cell_wrap_em */

#ifndef struct_tag_nCn326iMCNAOlldGGgUCWC
#define struct_tag_nCn326iMCNAOlldGGgUCWC

struct tag_nCn326iMCNAOlldGGgUCWC
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_em inputVarSize;
  h_dsp_internal_SlidingWindow_em *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  h_dsp_internal_SlidingWindow_em _pobj0;
};

#endif                                 /* struct_tag_nCn326iMCNAOlldGGgUCWC */

#ifndef typedef_dsp_simulink_MovingAverage_em
#define typedef_dsp_simulink_MovingAverage_em

typedef struct tag_nCn326iMCNAOlldGGgUCWC dsp_simulink_MovingAverage_em;

#endif                               /* typedef_dsp_simulink_MovingAverage_em */

/* Parameters (default storage) */
typedef struct P_em_ P_em;

/* Forward declaration for rtModel */
typedef struct tag_RTM_em RT_MODEL_em;

#endif                                 /* RTW_HEADER_em_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
