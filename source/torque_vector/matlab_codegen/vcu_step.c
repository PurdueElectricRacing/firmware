#include "vcu_step.h"
#include "interp1.h"
#include "minOrMax.h"
#include "vcu_step_types.h"
#include <math.h>

void vcu_step(const pVCU_struc *p, const xVCU_struc *x, yVCU_struc *y)
{
  y->TH = x->TH_RAW;
  y->TH_PO = fmin(fmax(x->TH_RAW, 0.0), 1.0);
  y->TH_RG = fabs(fmin(fmax(x->TH_RAW, -1.0), 0.0));
  y->ST = x->ST_RAW;
  y->VB = x->VB_RAW;
  y->WM[0] = x->WM_RAW[0];
  y->WM[1] = x->WM_RAW[1];
  y->WM[2] = x->WM_RAW[2];
  y->WM[3] = x->WM_RAW[3];
  y->GS = x->GS_RAW;
  y->AV[0] = x->AV_RAW[0];
  y->AV[1] = x->AV_RAW[1];
  y->AV[2] = x->AV_RAW[2];
  y->IB = x->IB_RAW;
  y->MT = x->MT_RAW;
  y->IGBT_T = x->IGBT_T_RAW;
  y->INV_T = x->INV_T_RAW;
  y->MC = x->MC_RAW;
  y->IC = x->IC_RAW;
  y->BT = x->BT_RAW;
  y->TO[0] = x->TO_RAW[0];
  y->TO[1] = x->TO_RAW[1];
  y->TO[2] = x->TO_RAW[2];
  y->TO[3] = x->TO_RAW[3];
  if (x->TH_RAW >= 0.0) {
    double b_PB_derate_front[28];
    double dv1[4];
    double b_p[2];
    double dv[2];
    double PB_derate_front;
    double PB_derate_rear;
    double PB_snipped;
    double b;
    double b_b;
    double c_b;
    double d;
    double d_b;
    double e_b;
    double f_b;
    PB_snipped =
        fmax(fmin(y->PB, p->PB_derating_half_T), p->PB_derating_full_T);
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    dv[0] = 1.0;
    dv[1] = 1.0 - p->PB_derating_FR;
    PB_derate_front = interp1(b_p, dv, PB_snipped);
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    dv[0] = 1.0;
    dv[1] = p->PB_derating_FR;
    PB_derate_rear = interp1(b_p, dv, PB_snipped);
    b_p[0] = p->INV_T_derating_full_T;
    b_p[1] = p->INV_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    b = interp1(b_p, dv,
                fmax(fmin(x->INV_T_RAW, p->INV_derating_zero_T),
                     p->INV_T_derating_full_T));
    b_p[0] = p->IGBT_derating_full_T;
    b_p[1] = p->IGBT_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    b_b = interp1(b_p, dv,
                  fmax(fmin(x->IGBT_T_RAW, p->IGBT_derating_zero_T),
                       p->IGBT_derating_full_T));
    b_p[0] = p->MT_derating_full_T;
    b_p[1] = p->MT_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    c_b = interp1(
        b_p, dv,
        fmax(fmin(x->MT_RAW, p->MT_derating_zero_T), p->MT_derating_full_T));
    b_p[0] = p->BT_derating_full_T;
    b_p[1] = p->BT_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    d_b = interp1(
        b_p, dv,
        fmax(fmin(x->BT_RAW, p->BT_derating_zero_T), p->BT_derating_full_T));
    b_p[0] = p->VB_derating_full_T;
    b_p[1] = p->VB_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    e_b = interp1(
        b_p, dv,
        fmax(fmin(x->VB_RAW, p->VB_derating_zero_T), p->VB_derating_full_T));
    b_p[0] = p->IB_derating_full_T;
    b_p[1] = p->IB_derating_zero_T;
    dv[0] = 1.0;
    dv[1] = 0.0;
    f_b = interp1(
        b_p, dv,
        fmax(fmin(x->IB_RAW, p->IB_derating_zero_T), p->IB_derating_full_T));
    b_PB_derate_front[0] = PB_derate_front;
    b_PB_derate_front[7] = PB_derate_front;
    b_PB_derate_front[14] = PB_derate_rear;
    b_PB_derate_front[21] = PB_derate_rear;
    b_PB_derate_front[1] = b;
    b_PB_derate_front[2] = b_b;
    b_PB_derate_front[3] = c_b;
    b_PB_derate_front[4] = d_b;
    b_PB_derate_front[5] = e_b;
    b_PB_derate_front[6] = f_b;
    b_PB_derate_front[8] = b;
    b_PB_derate_front[9] = b_b;
    b_PB_derate_front[10] = c_b;
    b_PB_derate_front[11] = d_b;
    b_PB_derate_front[12] = e_b;
    b_PB_derate_front[13] = f_b;
    b_PB_derate_front[15] = b;
    b_PB_derate_front[16] = b_b;
    b_PB_derate_front[17] = c_b;
    b_PB_derate_front[18] = d_b;
    b_PB_derate_front[19] = e_b;
    b_PB_derate_front[20] = f_b;
    b_PB_derate_front[22] = b;
    b_PB_derate_front[23] = b_b;
    b_PB_derate_front[24] = c_b;
    b_PB_derate_front[25] = d_b;
    b_PB_derate_front[26] = e_b;
    b_PB_derate_front[27] = f_b;
    minimum(b_PB_derate_front, dv1);
    d = fmin(y->TH_PO * p->MAX_TO_ABS, p->MAX_TO_ABS * dv1[0]);
    y->TO_BL_PO[0] = d;
    y->TORQUE_OUT[0] = d;
    d = fmin(y->TH_PO * p->MAX_TO_ABS, p->MAX_TO_ABS * dv1[1]);
    y->TO_BL_PO[1] = d;
    y->TORQUE_OUT[1] = d;
    d = fmin(y->TH_PO * p->MAX_TO_ABS, p->MAX_TO_ABS * dv1[2]);
    y->TO_BL_PO[2] = d;
    y->TORQUE_OUT[2] = d;
    d = fmin(y->TH_PO * p->MAX_TO_ABS, p->MAX_TO_ABS * dv1[3]);
    y->TO_BL_PO[3] = d;
    y->TORQUE_OUT[3] = d;
  } else if (x->TH_RAW < 0.0) {
    y->TORQUE_OUT[0] = 0.0;
    y->TORQUE_OUT[1] = 0.0;
    y->TORQUE_OUT[2] = 0.0;
    y->TORQUE_OUT[3] = 0.0;
  }
}
