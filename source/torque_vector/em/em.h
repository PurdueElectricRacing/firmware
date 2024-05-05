/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em.h
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

#ifndef RTW_HEADER_em_h_
#define RTW_HEADER_em_h_
#ifndef em_COMMON_INCLUDES_
#define em_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* em_COMMON_INCLUDES_ */

#include "em_types.h"

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  dsp_simulink_MovingAverage_em obj;   /* '<S1>/Moving Average1' */
} DW_em;

/* Constant parameters (default storage) */
typedef struct {
  /* Computed Parameter: k_max_maxIndex
   * Referenced by: '<S2>/k_max'
   */
  uint32_T k_max_maxIndex[2];
} ConstP_em;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T rTV[2];                       /* '<Root>/rTVS' */
  real_T rEQUAL;                       /* '<Root>/rEQUAL' */
  boolean_T F_raw[4];                  /* '<Root>/F_raw' */
  real_T D_raw[3];                     /* '<Root>/D_raw' */
} ExtU_em;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T kTVS[2];                      /* '<Root>/kTVS' */
  real_T kEQUAL[2];                    /* '<Root>/kEQUAL' */
  boolean_T MM_FLAGS[10];              /* '<Root>/MM_FLAGS' */
  real_T MM_STATE;                     /* '<Root>/MM_STATE' */
} ExtY_em;

/* Parameters (default storage) */
struct P_em_ {
  real_T V[26];                        /* Variable: V
                                        * Referenced by: '<S2>/k_max'
                                        */
  real_T epsilon;                      /* Variable: epsilon
                                        * Referenced by:
                                        *   '<S1>/Constant4'
                                        *   '<S1>/Constant5'
                                        */
  real_T lb_mm[3];                     /* Variable: lb_mm
                                        * Referenced by:
                                        *   '<S1>/Constant5'
                                        *   '<S1>/Saturation'
                                        */
  real_T maxK[2782];                   /* Variable: maxK
                                        * Referenced by: '<S2>/k_max'
                                        */
  real_T ub_mm[3];                     /* Variable: ub_mm
                                        * Referenced by:
                                        *   '<S1>/Constant4'
                                        *   '<S1>/Saturation'
                                        */
  real_T w[107];                       /* Variable: w
                                        * Referenced by: '<S2>/k_max'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_em {
  DW_em *dwork;
};

/* Block parameters (default storage) */
extern P_em rtP_em;

/* Constant parameters (default storage) */
extern const ConstP_em rtConstP_em;

/* Model entry point functions */
extern void em_initialize(RT_MODEL_em *const rtM_em);
extern void em_step(RT_MODEL_em *const rtM_em, ExtU_em *rtU_em, ExtY_em *rtY_em);
extern void em_terminate(RT_MODEL_em *const rtM_em);

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'em'
 * '<S1>'   : 'em/State Machine'
 * '<S2>'   : 'em/em'
 */
#endif                                 /* RTW_HEADER_em_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
