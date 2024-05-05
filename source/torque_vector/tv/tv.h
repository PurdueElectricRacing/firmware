/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: tv.h
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

#ifndef RTW_HEADER_tv_h_
#define RTW_HEADER_tv_h_
#ifndef tv_COMMON_INCLUDES_
#define tv_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* tv_COMMON_INCLUDES_ */

#include "tv_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals and states (default storage) for system '<S2>/Median Filter' */
typedef struct {
  dsp_simulink_MedianFilter_tv obj;    /* '<S2>/Median Filter' */
  real_T MedianFilter_c;               /* '<S2>/Median Filter' */
  boolean_T objisempty;                /* '<S2>/Median Filter' */
} DW_MedianFilter_tv;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  DW_MedianFilter_tv MedianFilter2;    /* '<S2>/Median Filter' */
  DW_MedianFilter_tv MedianFilter1;    /* '<S2>/Median Filter' */
  DW_MedianFilter_tv MedianFilter_g;   /* '<S2>/Median Filter' */
  dsp_simulink_MovingAverage_tv obj;   /* '<S2>/Moving Average1' */
  dsp_simulink_MovingAverage_j_tv obj_m;/* '<S3>/Moving Average1' */
} DW_tv;

/* Constant parameters (default storage) */
typedef struct {
  /* Variable: lb
   * Referenced by: '<S3>/Constant5'
   */
  real_T Constant5_lb[16];

  /* Expression: lb
   * Referenced by: '<S3>/Saturation'
   */
  real_T Saturation_LowerSat[16];

  /* Computed Parameter: uDLookupTable_maxIndex
   * Referenced by: '<S4>/2-D Lookup Table'
   */
  uint32_T uDLookupTable_maxIndex[2];
} ConstP_tv;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T D_raw[16];                    /* '<Root>/D_raw' */
  boolean_T F_raw[13];                 /* '<Root>/F_raw' */
  real_T R[9];                         /* '<Root>/R' */
  real_T dphi;                         /* '<Root>/dphi' */
  real_T TVS_P;                        /* '<Root>/TVS_P' */
  real_T TVS_I;                        /* '<Root>/TVS_I' */
} ExtU_tv;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T rTVS[2];                      /* '<Root>/rTVS' */
  real_T rEQUAL;                       /* '<Root>/rEQUAL' */
  real_T max_K;                        /* '<Root>/max_K' */
  real_T TVS_STATE;                    /* '<Root>/TVS_STATE' */
  boolean_T F_TVS[39];                 /* '<Root>/F_TVS' */
  real_T sig_trunc[15];                /* '<Root>/sig_trunc' */
  real_T sig_filt[15];                 /* '<Root>/sig_filt' */
} ExtY_tv;

/* Parameters (default storage) */
struct P_tv_ {
  real_T bI_bias;                      /* Variable: bI_bias
                                        * Referenced by: '<S4>/Bias3'
                                        */
  real_T bI_gain;                      /* Variable: bI_gain
                                        * Referenced by: '<S4>/Gain5'
                                        */
  real_T bT_bias;                      /* Variable: bT_bias
                                        * Referenced by: '<S4>/Bias2'
                                        */
  real_T bT_gain;                      /* Variable: bT_gain
                                        * Referenced by: '<S4>/Gain3'
                                        */
  real_T epsilon;                      /* Variable: epsilon
                                        * Referenced by:
                                        *   '<S3>/Constant4'
                                        *   '<S3>/Constant5'
                                        */
  real_T half_track[2];                /* Variable: half_track
                                        * Referenced by: '<S4>/P_gain'
                                        */
  real_T mT_bias;                      /* Variable: mT_bias
                                        * Referenced by: '<S4>/Bias'
                                        */
  real_T mT_gain;                      /* Variable: mT_gain
                                        * Referenced by: '<S4>/Gain'
                                        */
  real_T mcT_bias;                     /* Variable: mcT_bias
                                        * Referenced by: '<S4>/Bias1'
                                        */
  real_T mcT_gain;                     /* Variable: mcT_gain
                                        * Referenced by: '<S4>/Gain1'
                                        */
  real_T r_power_sat;                  /* Variable: r_power_sat
                                        * Referenced by: '<S4>/Gain4'
                                        */
  real_T s[53];                        /* Variable: s
                                        * Referenced by: '<S4>/2-D Lookup Table'
                                        */
  real_T ub[16];                       /* Variable: ub
                                        * Referenced by:
                                        *   '<S3>/Constant4'
                                        *   '<S3>/Saturation'
                                        */
  real_T v[51];                        /* Variable: v
                                        * Referenced by: '<S4>/2-D Lookup Table'
                                        */
  real_T yaw_table[2703];              /* Variable: yaw_table
                                        * Referenced by: '<S4>/2-D Lookup Table'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_tv {
  const char_T * volatile errorStatus;
  DW_tv *dwork;
};

/* Block parameters (default storage) */
extern P_tv rtP_tv;

/* Constant parameters (default storage) */
extern const ConstP_tv rtConstP_tv;

/* Model entry point functions */
extern void tv_initialize(RT_MODEL_tv *const rtM_tv);
extern void tv_step(RT_MODEL_tv *const rtM_tv, ExtU_tv *rtU_tv, ExtY_tv *rtY_tv);
extern void tv_terminate(RT_MODEL_tv *const rtM_tv);

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
 * '<Root>' : 'tv'
 * '<S1>'   : 'tv/tv'
 * '<S2>'   : 'tv/tv/State Estimation'
 * '<S3>'   : 'tv/tv/State Machine'
 * '<S4>'   : 'tv/tv/Torque Vectoring'
 * '<S5>'   : 'tv/tv/Torque Vectoring/Power Limit'
 * '<S6>'   : 'tv/tv/Torque Vectoring/Saturation Dynamic'
 * '<S7>'   : 'tv/tv/Torque Vectoring/compute_r_TVS'
 */
#endif                                 /* RTW_HEADER_tv_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
