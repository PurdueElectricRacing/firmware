/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: SFS.h
 *
 * Code generated for Simulink model 'SFS'.
 *
 * Model version                  : 2.298
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Mar  7 19:18:20 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SFS_h_
#define RTW_HEADER_SFS_h_
#ifndef SFS_COMMON_INCLUDES_
#define SFS_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* SFS_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Custom Type definition for MATLAB Function: '<S2>/fusion' */
#ifndef struct_tag_Qswwf45pAGwdJmfEwBddhC
#define struct_tag_Qswwf45pAGwdJmfEwBddhC

struct tag_Qswwf45pAGwdJmfEwBddhC
{
  int32_T __dummy;
};

#endif                                 /* struct_tag_Qswwf45pAGwdJmfEwBddhC */

#ifndef typedef_fusion_internal_frames_NED
#define typedef_fusion_internal_frames_NED

typedef struct tag_Qswwf45pAGwdJmfEwBddhC fusion_internal_frames_NED;

#endif                                 /* typedef_fusion_internal_frames_NED */

#ifndef struct_tag_qHRwoV6ocLgXRwuZ4G6fCE
#define struct_tag_qHRwoV6ocLgXRwuZ4G6fCE

struct tag_qHRwoV6ocLgXRwuZ4G6fCE
{
  real_T OrientationIdx[4];
  fusion_internal_frames_NED ReferenceFrameObject;
  real_T ReferenceLocation[3];
  real_T QuaternionNoise[4];
  real_T AngularVelocityNoise[3];
  real_T PositionNoise[3];
  real_T VelocityNoise[3];
  real_T AccelerationNoise[3];
  real_T GyroscopeBiasNoise[3];
  real_T AccelerometerBiasNoise[3];
  real_T GeomagneticVectorNoise[3];
  real_T MagnetometerBiasNoise[3];
  real_T pState[28];
  real_T pStateCovariance[784];
};

#endif                                 /* struct_tag_qHRwoV6ocLgXRwuZ4G6fCE */

#ifndef typedef_insfilterAsync
#define typedef_insfilterAsync

typedef struct tag_qHRwoV6ocLgXRwuZ4G6fCE insfilterAsync;

#endif                                 /* typedef_insfilterAsync */

/* Block signals and states (default storage) for system '<Root>/SFS' */
typedef struct {
  insfilterAsync filter;
  real_T UnitDelay3_DSTATE[784];       /* '<S2>/Unit Delay3' */
  real_T UnitDelay2_DSTATE[28];        /* '<S2>/Unit Delay2' */
  real_T val[784];
  real_T Pdot[784];
  real_T procNoise[784];
  real_T dfdx[784];
  real_T obj[784];
  real_T P_m[784];
  real_T X_c[784];
  real_T obj_k[784];
  real_T W[784];
  real_T P_c[784];
  real_T UnitDelay1_DSTATE;            /* '<S2>/Unit Delay1' */
} DW_SFS;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  DW_SFS SFS_ds;                       /* '<Root>/SFS' */
} DW;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: covarience_matrix_IC
   * Referenced by: '<S2>/Unit Delay3'
   */
  real_T UnitDelay3_InitialCondition[784];

  /* Expression: state_IC
   * Referenced by: '<S2>/Unit Delay2'
   */
  real_T UnitDelay2_InitialCondition[28];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T mag[3];                       /* '<Root>/mag' */
  real_T gyro[3];                      /* '<Root>/gyro' */
  real_T acc[3];                       /* '<Root>/acc' */
  real_T pos[3];                       /* '<Root>/pos' */
  real_T vel[3];                       /* '<Root>/vel' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T angvel_VNED[3];               /* '<Root>/angvel_VNED' */
  real_T ang_NED[4];                   /* '<Root>/ang_NED' */
  real_T pos_VNED[3];                  /* '<Root>/pos_VNED' */
  real_T vel_VNED[3];                  /* '<Root>/vel_VNED' */
  real_T acc_VNED[3];                  /* '<Root>/acc_VNED' */
  real_T mag_VNED[3];                  /* '<Root>/mag_VNED' */
  real_T acc_bias_VNED[3];             /* '<Root>/acc_bias_VNED' */
  real_T gyro_bias_VNED[3];            /* '<Root>/gyro_bias_VNED' */
  real_T mag_bias_VNED[3];             /* '<Root>/mag_bias_VNED' */
} ExtY;

/* Parameters (default storage) */
struct P_ {
  real_T covA;                         /* Variable: covA
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T covG;                         /* Variable: covG
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T covM;                         /* Variable: covM
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T covP;                         /* Variable: covP
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T covV;                         /* Variable: covV
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T fusion_t;                     /* Variable: fusion_t
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T gps_ratio;                    /* Variable: gps_ratio
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T location_lla_IC[3];           /* Variable: location_lla_IC
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T noise_state[28];              /* Variable: noise_state
                                        * Referenced by: '<S2>/fusion'
                                        */
  real_T phi;                          /* Variable: phi
                                        * Referenced by: '<S2>/SNED to VNED'
                                        */
};

/* Parameters (default storage) */
typedef struct P_ P;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
  DW *dwork;
};

/* Block parameters (default storage) */
extern P rtP;

/* Constant parameters (default storage) */
extern const ConstP rtConstP;

/* Model entry point functions */
extern void SFS_initialize(RT_MODEL *const rtM);
extern void SFS_step(RT_MODEL *const rtM, ExtU *rtU, ExtY *rtY);
void SFS_MAIN(void);

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('car_model/Control Systems/SFS')    - opens subsystem car_model/Control Systems/SFS
 * hilite_system('car_model/Control Systems/SFS/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'car_model/Control Systems'
 * '<S1>'   : 'car_model/Control Systems/SFS'
 * '<S2>'   : 'car_model/Control Systems/SFS/Sensor Fusion'
 * '<S3>'   : 'car_model/Control Systems/SFS/Sensor Fusion/NED2VNED'
 * '<S4>'   : 'car_model/Control Systems/SFS/Sensor Fusion/SNED to VNED'
 * '<S5>'   : 'car_model/Control Systems/SFS/Sensor Fusion/fusion'
 */
#endif                                 /* RTW_HEADER_SFS_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */