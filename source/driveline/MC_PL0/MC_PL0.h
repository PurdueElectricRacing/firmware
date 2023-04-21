/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: MC_PL0.h
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

#ifndef RTW_HEADER_MC_PL0_h_
#define RTW_HEADER_MC_PL0_h_
#ifndef MC_PL0_COMMON_INCLUDES_
#define MC_PL0_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* MC_PL0_COMMON_INCLUDES_ */

#include "MC_PL0_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T UnitDelay1_DSTATE[4];         /* '<S1>/Unit Delay1' */
  real_T DiscreteTimeIntegrator_DSTATE[4];/* '<S2>/Discrete-Time Integrator' */
  real_T UnitDelay_DSTATE[4];          /* '<S5>/Unit Delay' */
  real_T PrevY[4];                     /* '<S5>/Rate Limiter' */
  real_T PrevY_l[4];                   /* '<S1>/Rate Limiter' */
  real_T MatrixMultiply[8];            /* '<S4>/MatrixMultiply' */
  real_T Switch[4];                    /* '<S9>/Switch' */
  real_T Sum[4];                       /* '<S1>/Sum' */
  real_T fractions[2];
  real_T fractions_m[2];
  real_T fractions_c[2];
  real_T Sum1;                         /* '<S4>/Sum1' */
  real_T rtb_Switch2_k;
  real_T DiscreteTimeIntegrator;
  real_T rtb_RateLimiter_m_c;
  real_T rtb_Switch_c_b;
  real_T rtb_Switch2_idx_0;
  real_T DiscreteTimeIntegrator_idx_0;
  real_T rtb_Switch2_idx_1;
  real_T DiscreteTimeIntegrator_idx_1;
  real_T rtb_Switch2_idx_2;
  real_T DiscreteTimeIntegrator_idx_2;
  real_T rtb_RateLimiter_m_idx_0;
  real_T rtb_RateLimiter_m_idx_1;
  uint32_T bpIndices[2];
  uint32_T bpIndices_p[2];
  uint32_T bpIndices_c[2];
} DW;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: power_input_grid
   * Referenced by: '<S4>/2-D Lookup Table'
   */
  real_T uDLookupTable_tableData[1712];

  /* Expression: max_torque
   * Referenced by: '<S5>/1-D Lookup Table'
   */
  real_T uDLookupTable_tableData_g[68];

  /* Expression: max_rpm
   * Referenced by: '<S5>/1-D Lookup Table'
   */
  real_T uDLookupTable_bp01Data[68];

  /* Expression: voltage_grid
   * Referenced by: '<S3>/2-D Lookup Table1'
   */
  real_T uDLookupTable1_tableData[1712];

  /* Expression: efficiency_grid
   * Referenced by: '<S5>/2-D Lookup Table'
   */
  real_T uDLookupTable_tableData_b[1712];

  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S3>/2-D Lookup Table1'
   *   '<S4>/2-D Lookup Table'
   *   '<S5>/2-D Lookup Table'
   */
  uint32_T pooled6[2];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Wx[4];                        /* '<Root>/Wx' */
  real_T Tx[4];                        /* '<Root>/Tx' */
  real_T motor_T[4];                   /* '<Root>/motor_T' */
  real_T power_limits[2];              /* '<Root>/power_limits' */
  real_T Motor_V[4];                   /* '<Root>/Vbatt' */
  real_T Motor_I[4];                   /* '<Root>/Motor_I' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T k[4];                         /* '<Root>/k' */
  real_T P_g[4];                       /* '<Root>/P' */
  real_T T[4];                         /* '<Root>/T' */
  real_T r[4];                         /* '<Root>/r' */
} ExtY;

/* Parameters (default storage) */
struct P_ {
  real_T feebackThreshhold;            /* Variable: feebackThreshhold
                                        * Referenced by: '<S1>/Switch'
                                        */
};

/* Code_Instrumentation_Declarations_Placeholder */

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
extern void MC_PL0_initialize(RT_MODEL *const rtM);
extern void MC_PL0_step(RT_MODEL *const rtM, ExtU *rtU, ExtY *rtY);
void rt_OneStep(RT_MODEL *const rtM);

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S2>/Constant' : Unused code path elimination
 * Block '<S2>/Divide' : Unused code path elimination
 * Block '<S2>/Divide2' : Unused code path elimination
 * Block '<S2>/Gain2' : Unused code path elimination
 * Block '<S2>/Gain3' : Unused code path elimination
 * Block '<S2>/Product' : Unused code path elimination
 * Block '<S2>/Saturation' : Unused code path elimination
 * Block '<S6>/Data Type Duplicate' : Unused code path elimination
 * Block '<S6>/Data Type Propagation' : Unused code path elimination
 * Block '<S6>/LowerRelop1' : Unused code path elimination
 * Block '<S6>/Switch' : Unused code path elimination
 * Block '<S6>/Switch2' : Unused code path elimination
 * Block '<S6>/UpperRelop' : Unused code path elimination
 * Block '<S2>/Saturation1' : Unused code path elimination
 * Block '<S2>/Sum' : Unused code path elimination
 * Block '<S2>/Sum2' : Unused code path elimination
 * Block '<S2>/Sum3' : Unused code path elimination
 * Block '<S7>/Data Type Duplicate' : Unused code path elimination
 * Block '<S7>/Data Type Propagation' : Unused code path elimination
 * Block '<S5>/Abs' : Unused code path elimination
 * Block '<S5>/Abs1' : Unused code path elimination
 * Block '<S5>/Add' : Unused code path elimination
 * Block '<S5>/Add1' : Unused code path elimination
 * Block '<S5>/Bias' : Unused code path elimination
 * Block '<S5>/Constant' : Unused code path elimination
 * Block '<S5>/Constant1' : Unused code path elimination
 * Block '<S5>/Constant11' : Unused code path elimination
 * Block '<S5>/Constant2' : Unused code path elimination
 * Block '<S5>/Constant3' : Unused code path elimination
 * Block '<S5>/Constant4' : Unused code path elimination
 * Block '<S5>/Constant5' : Unused code path elimination
 * Block '<S5>/Constant6' : Unused code path elimination
 * Block '<S5>/Constant7' : Unused code path elimination
 * Block '<S5>/Constant8' : Unused code path elimination
 * Block '<S5>/Constant9' : Unused code path elimination
 * Block '<S5>/Divide' : Unused code path elimination
 * Block '<S5>/Divide2' : Unused code path elimination
 * Block '<S5>/Dot Product' : Unused code path elimination
 * Block '<S5>/Floor' : Unused code path elimination
 * Block '<S5>/Floor1' : Unused code path elimination
 * Block '<S5>/Gain' : Unused code path elimination
 * Block '<S5>/Gain1' : Unused code path elimination
 * Block '<S5>/Gain2' : Unused code path elimination
 * Block '<S5>/Gain4' : Unused code path elimination
 * Block '<S5>/GreaterThan' : Unused code path elimination
 * Block '<S5>/Less Than' : Unused code path elimination
 * Block '<S5>/Less Than1' : Unused code path elimination
 * Block '<S5>/MatrixMultiply1' : Unused code path elimination
 * Block '<S5>/Maximum' : Unused code path elimination
 * Block '<S5>/Minimum' : Unused code path elimination
 * Block '<S5>/Minimum1' : Unused code path elimination
 * Block '<S5>/OR' : Unused code path elimination
 * Block '<S5>/Product' : Unused code path elimination
 * Block '<S5>/Product1' : Unused code path elimination
 * Block '<S5>/Product2' : Unused code path elimination
 * Block '<S5>/Product4' : Unused code path elimination
 * Block '<S8>/Data Type Duplicate' : Unused code path elimination
 * Block '<S8>/Data Type Propagation' : Unused code path elimination
 * Block '<S9>/Data Type Duplicate' : Unused code path elimination
 * Block '<S9>/Data Type Propagation' : Unused code path elimination
 * Block '<S5>/Saturation1' : Unused code path elimination
 * Block '<S5>/Selector' : Unused code path elimination
 * Block '<S5>/Selector1' : Unused code path elimination
 * Block '<S5>/Selector2' : Unused code path elimination
 * Block '<S5>/Selector3' : Unused code path elimination
 * Block '<S5>/Selector4' : Unused code path elimination
 * Block '<S5>/Sign' : Unused code path elimination
 * Block '<S5>/Sum' : Unused code path elimination
 * Block '<S5>/Sum1' : Unused code path elimination
 * Block '<S5>/Sum2' : Unused code path elimination
 * Block '<S5>/Sum3' : Unused code path elimination
 * Block '<S5>/Switch1' : Unused code path elimination
 * Block '<S1>/Unit Delay' : Unused code path elimination
 */

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
 * hilite_system('complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL')    - opens subsystem complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL
 * hilite_system('complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery'
 * '<S1>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL'
 * '<S2>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Feed Back Control'
 * '<S3>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Feed Forward Control'
 * '<S4>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Power Limiting'
 * '<S5>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Torque Limiting'
 * '<S6>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Feed Back Control/Saturation Dynamic'
 * '<S7>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Power Limiting/Saturation Dynamic'
 * '<S8>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Torque Limiting/Saturation Dynamic1'
 * '<S9>'   : 'complete_plant_v6/Vehicle Model/Powertrain/Motor and Battery/MC_PL/Torque Limiting/Saturation Dynamic2'
 */
#endif                                 /* RTW_HEADER_MC_PL0_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
