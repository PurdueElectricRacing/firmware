/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em.h
 *
 * Code generated for Simulink model 'em'.
 *
 * Model version                  : 1.41
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Sat Apr 20 17:57:48 2024
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

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: maxK
   * Referenced by: '<S1>/k_max'
   */
  real_T k_max_tableData[2782];

  /* Computed Parameter: k_max_maxIndex
   * Referenced by: '<S1>/k_max'
   */
  uint32_T k_max_maxIndex[2];
} ConstP_em;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T rTVS[2];                      /* '<Root>/rTVS' */
  real_T rEQUAL[2];                    /* '<Root>/rEQUAL' */
  real_T V;                            /* '<Root>/V' */
  real_T w[2];                         /* '<Root>/w' */
} ExtU_em;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T kTVS[2];                      /* '<Root>/kTVS' */
  real_T kEQUAL[2];                    /* '<Root>/kEQUAL' */
} ExtY_em;

/* Parameters (default storage) */
struct P_em_ {
  real_T V[26];                        /* Variable: V
                                        * Referenced by: '<S1>/k_max'
                                        */
  real_T w[107];                       /* Variable: w
                                        * Referenced by: '<S1>/k_max'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_em {
  char_T rt_unused;
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
 * '<S1>'   : 'em/em'
 */
#endif                                 /* RTW_HEADER_em_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
