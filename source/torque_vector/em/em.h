/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: em.h
 *
 * Code generated for Simulink model 'em'.
 *
 * Model version                  : 1.20
 * Simulink Coder version         : 23.2 (R2023b) 01-Aug-2023
 * C/C++ source code generated on : Sat Apr  6 13:25:04 2024
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
  real_T UnitDelay_DSTATE[2];          /* '<S1>/Unit Delay' */
  real_T UnitDelay1_DSTATE;            /* '<S1>/Unit Delay1' */
  real_T UnitDelay2_DSTATE;            /* '<S1>/Unit Delay2' */
} DW_em;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: minK
   * Referenced by: '<S1>/k_min'
   */
  real_T k_min_tableData[2782];

  /* Expression: dK
   * Referenced by: '<S1>/dk'
   */
  real_T dk_tableData[2782];

  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S1>/dk'
   *   '<S1>/k_min'
   */
  uint32_T pooled1[2];
} ConstP_em;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T rTVS[2];                      /* '<Root>/rTVS' */
  real_T rEQUAL[2];                    /* '<Root>/rEQUAL' */
  real_T V;                            /* '<Root>/V' */
  real_T w[2];                         /* '<Root>/w' */
  real_T TVS_STATE;                    /* '<Root>/TVS_STATE' */
} ExtU_em;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T k[2];                         /* '<Root>/k' */
  real_T TVS_PERMIT;                   /* '<Root>/TVS_PERMIT' */
} ExtY_em;

/* Parameters (default storage) */
struct P_em_ {
  real_T FINISHED_SMOOTHENING_IC;      /* Variable: FINISHED_SMOOTHENING_IC
                                        * Referenced by: '<S1>/Unit Delay2'
                                        */
  real_T LAST_TVS_PERMIT_IC;           /* Variable: LAST_TVS_PERMIT_IC
                                        * Referenced by: '<S1>/Unit Delay1'
                                        */
  real_T V[26];                        /* Variable: V
                                        * Referenced by:
                                        *   '<S1>/dk'
                                        *   '<S1>/k_min'
                                        */
  real_T alpha;                        /* Variable: alpha
                                        * Referenced by: '<S1>/MATLAB Function'
                                        */
  real_T dk_thresh;                    /* Variable: dk_thresh
                                        * Referenced by: '<S1>/MATLAB Function'
                                        */
  real_T k_IC[2];                      /* Variable: k_IC
                                        * Referenced by: '<S1>/Unit Delay'
                                        */
  real_T w[107];                       /* Variable: w
                                        * Referenced by:
                                        *   '<S1>/dk'
                                        *   '<S1>/k_min'
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
 * '<S1>'   : 'em/em'
 * '<S2>'   : 'em/em/MATLAB Function'
 */
#endif                                 /* RTW_HEADER_em_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
