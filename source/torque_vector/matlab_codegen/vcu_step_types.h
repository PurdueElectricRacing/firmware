#ifndef VCU_STEP_TYPES_H
#define VCU_STEP_TYPES_H

#include "rtwtypes.h"

#ifndef typedef_pVCU_struc
#define typedef_pVCU_struc
typedef struct {
  double r;
  double ht[2];
  double wb;
  double gr;
  double MAX_TO_ABS;
  double PB_derating_full_T;
  double PB_derating_half_T;
  double PB_derating_FR;
  double INV_T_derating_full_T;
  double INV_derating_zero_T;
  double IGBT_derating_full_T;
  double IGBT_derating_zero_T;
  double MT_derating_full_T;
  double MT_derating_zero_T;
  double BT_derating_full_T;
  double BT_derating_zero_T;
  double VB_derating_full_T;
  double VB_derating_zero_T;
  double IB_derating_full_T;
  double IB_derating_zero_T;
} pVCU_struc;
#endif

#ifndef typedef_xVCU_struc
#define typedef_xVCU_struc
typedef struct {
  double TH_RAW;
  double ST_RAW;
  double VB_RAW;
  double WM_RAW[4];
  double GS_RAW;
  double AV_RAW[3];
  double IB_RAW;
  double MT_RAW;
  double IGBT_T_RAW;
  double INV_T_RAW;
  double MC_RAW;
  double IC_RAW;
  double BT_RAW;
  double TO_RAW[4];
} xVCU_struc;
#endif

#ifndef typedef_yVCU_struc
#define typedef_yVCU_struc
typedef struct {
  double TH;
  double TH_PO;
  double TH_RG;
  double ST;
  double VB;
  double WM[4];
  double GS;
  double AV[3];
  double IB;
  double MT;
  double IGBT_T;
  double INV_T;
  double MC;
  double IC;
  double BT;
  double TO[4];
  double PB;
  double TO_BL_PO[4];
  double TORQUE_OUT[4];
} yVCU_struc;
#endif

#endif
