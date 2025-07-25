#include "gps.h"

// VCU Structs
typedef struct {
    float CS_SFLAG;
    float TB_SFLAG;
    float SS_SFLAG;
    float WT_SFLAG;
    float IV_SFLAG;
    float BT_SFLAG;
    float IAC_SFLAG;
    float IAT_SFLAG;
    float IBC_SFLAG;
    float IBT_SFLAG;
    float SS_FFLAG;
    float AV_FFLAG;
    float GS_FFLAG;
    float VCU_PFLAG;
    float VCU_CFLAG;
} fVCU_struct;

typedef struct {
    float TH_RAW;
    float ST_RAW;
    float VB_RAW;
    float WT_RAW[2];
    float WM_RAW[2];
    float GS_RAW;
    float AV_RAW[3];
    float IB_RAW;
    float MT_RAW;
    float CT_RAW;
    float IT_RAW;
    float MC_RAW;
    float IC_RAW;
    float BT_RAW;
    float AG_RAW[3];
    float TO_RAW[2];
    float VT_DB_RAW;
    float TV_PP_RAW;
    float TC_TR_RAW;
    float VS_MAX_SR_RAW;
} xVCU_struct;

typedef struct {
    float PT_permit_buffer[5];
    float VS_permit_buffer[5];
    float VT_permit_buffer[5];
    float VCU_mode;
    float IB_CF_buffer[10];
    float TH_CF;
    float ST_CF;
    float VB_CF;
    float WT_CF[2];
    float WM_CF[2];
    float W_CF[2];
    float GS_CF;
    float AV_CF[3];
    float IB_CF;
    float MT_CF;
    float CT_CF;
    float IT_CF;
    float MC_CF;
    float IC_CF;
    float BT_CF;
    float AG_CF[3];
    float TO_CF[2];
    float VT_DB_CF;
    float TV_PP_CF;
    float TC_TR_CF;
    float VS_MAX_SR_CF;
    float zero_current_counter;
    float Batt_SOC;
    float Batt_Voc;
    float WM_CS[2];
    float TO_ET[2];
    float TO_AB_MX;
    float TO_DR_MX;
    float TO_PT[2];
    float VT_mode;
    float TO_VT[2];
    float TV_AV_ref;
    float TV_delta_torque;
    float TC_highs;
    float TC_lows;
    float SR;
    float WM_VS[2];
    float SR_VS;
} yVCU_struct;

typedef struct {
    float r;
    float ht[2];
    float gr;
    float Ns;
    float PT_permit_N;
    float VS_permit_N;
    float VT_permit_N;
    float CS_SFLAG_True;
    float TB_SFLAG_True;
    float SS_SFLAG_True;
    float WT_SFLAG_True;
    float IV_SFLAG_True;
    float BT_SFLAG_True;
    float IAC_SFLAG_True;
    float IAT_SFLAG_True;
    float IBC_SFLAG_True;
    float IBT_SFLAG_True;
    float SS_FFLAG_True;
    float AV_FFLAG_True;
    float GS_FFLAG_True;
    float VCU_PFLAG_VS;
    float VCU_PFLAG_VT;
    float VCU_CFLAG_CS;
    float VCU_CFLAG_CT;
    float TH_lb;
    float ST_lb;
    float VB_lb;
    float WT_lb[2];
    float WM_lb[2];
    float GS_lb;
    float AV_lb[3];
    float IB_lb;
    float MT_lb;
    float CT_lb;
    float IT_lb;
    float MC_lb;
    float IC_lb;
    float BT_lb;
    float AG_lb[3];
    float TO_lb[2];
    float VT_DB_lb;
    float TV_PP_lb;
    float TC_TR_lb;
    float VS_MAX_SR_lb;
    float TH_ub;
    float ST_ub;
    float VB_ub;
    float WT_ub[2];
    float WM_ub[2];
    float GS_ub;
    float AV_ub[3];
    float IB_ub;
    float MT_ub;
    float CT_ub;
    float IT_ub;
    float MC_ub;
    float IC_ub;
    float BT_ub;
    float AG_ub[3];
    float TO_ub[2];
    float VT_DB_ub;
    float TV_PP_ub;
    float TC_TR_ub;
    float VS_MAX_SR_ub;
    float CF_IB_filter_N;
    float R[9];
    float W_CF_SELECTION;
    float Batt_Voc_brk[506];
    float Batt_As_Discharged_tbl[506];
    float zero_currents_to_update_SOC;
    float Batt_cell_zero_SOC_voltage;
    float Batt_cell_zero_SOC_capacity;
    float Batt_cell_full_SOC_voltage;
    float Batt_cell_full_SOC_capacity;
    float MAX_SPEED_NOM;
    float MAX_TORQUE_NOM;
    float PT_WM_brkpt[150];
    float PT_VB_brkpt[50];
    float PT_TO_table[7500];
    float PT_WM_lb;
    float PT_WM_ub;
    float PT_VB_lb;
    float PT_VB_ub;
    float mT_derating_full_T;
    float mT_derating_zero_T;
    float cT_derating_full_T;
    float cT_derating_zero_T;
    float bT_derating_full_T;
    float bT_derating_zero_T;
    float bI_derating_full_T;
    float bI_derating_zero_T;
    float Vb_derating_full_T;
    float Vb_derating_zero_T;
    float Ci_derating_full_T;
    float Ci_derating_zero_T;
    float Cm_derating_full_T;
    float Cm_derating_zero_T;
    float iT_derating_full_T;
    float iT_derating_zero_T;
    float dST_DB;
    float MAX_r;
    float TV_GS_brkpt[51];
    float TV_ST_brkpt[53];
    float TV_AV_table[2703];
    float TV_ST_lb;
    float TV_ST_ub;
    float TV_GS_lb;
    float TV_GS_ub;
    float TV_PI;
    float TC_eps;
    float TC_SR_threshold;
    float TC_highs_to_engage;
    float TC_lows_to_disengage;
    float WM_VS_LS;
} pVCU_struct;

#define CMODE_SPEED_CTRL  (0)
#define CMODE_TORQUE_CTRL (1)
#define FMODE_VAR         (1)
#define FMODE_NONE        (0)

// VCU struct initialization functions
fVCU_struct init_fVCU(void);
xVCU_struct init_xVCU(void);
yVCU_struct init_yVCU(void);
pVCU_struct init_pVCU(void);

// VCU pre-process
void vcu_pp(fVCU_struct* fVCU, xVCU_struct* xVCU, GPS_Handle_t* GPS);
// VCU dummy pre-process function, sets structs to constant values instead of reading from sensors
void vcu_pp_tester(fVCU_struct* fVCU, xVCU_struct* xVCU);

// MATLAB codegen
void vcu_step(const pVCU_struct* p, const fVCU_struct* f, const xVCU_struct* x, yVCU_struct* y);