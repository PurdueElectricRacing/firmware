#include <math.h>
#include "vcu.h"

static float interp1(const float varargin_1[2], const float varargin_2[2],
                     float varargin_3);

static float interp1(const float varargin_1[2], const float varargin_2[2],
                     float varargin_3)
{
  float Vq;
  float x_idx_0;
  float x_idx_1;
  float y_idx_0;
  float y_idx_1;
  y_idx_0 = varargin_2[0];
  x_idx_0 = varargin_1[0];
  y_idx_1 = varargin_2[1];
  x_idx_1 = varargin_1[1];
  if (varargin_1[1] < varargin_1[0]) {
    x_idx_0 = varargin_1[1];
    x_idx_1 = varargin_1[0];
    y_idx_0 = varargin_2[1];
    y_idx_1 = varargin_2[0];
  }
  Vq = 0.0F;
  if ((varargin_3 <= x_idx_1) && (varargin_3 >= x_idx_0)) {
    float r;
    r = (varargin_3 - x_idx_0) / (x_idx_1 - x_idx_0);
    if (r == 0.0F) {
      Vq = y_idx_0;
    } else if (r == 1.0F) {
      Vq = y_idx_1;
    } else if (y_idx_0 == y_idx_1) {
      Vq = y_idx_0;
    } else {
      Vq = (1.0F - r) * y_idx_0 + r * y_idx_1;
    }
  }
  return Vq;
}

void vcu_step(const pVCU_struct *p, const xVCU_struct *x, yVCU_struct *y)
{
  int b_i;
  int b_j;
  int i;
  int j;
  y->TH = x->TH_RAW;
  y->TH_PO = fminf(fmaxf(x->TH_RAW, 0.0F), 1.0F);
  y->TH_RG = fabsf(fminf(fmaxf(x->TH_RAW, -1.0F), 0.0F));
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
  y->PB = x->VB_RAW * x->IB_RAW;
  if (x->TH_RAW >= 0.0F) {
    float varargin_1[28];
    float b_p[2];
    float fv[2];
    float PB_derate_front;
    float PB_derate_rear;
    float PB_snipped;
    float d_b;
    float f_b;
    float h_b;
    float j_b;
    float l_b;
    float m_b;
    float out;
    out = y->TH_PO * p->MAX_TO_ABS_PO;
    PB_snipped =
        fmaxf(fminf(y->PB, p->PB_derating_half_T), p->PB_derating_full_T);
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    fv[0] = 1.0F;
    fv[1] = 1.0F - p->PB_derating_FR;
    PB_derate_front = interp1(b_p, fv, PB_snipped);
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    fv[0] = 1.0F;
    fv[1] = p->PB_derating_FR;
    PB_derate_rear = interp1(b_p, fv, PB_snipped);
    b_p[0] = p->INV_T_derating_full_T;
    b_p[1] = p->INV_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    d_b = interp1(b_p, fv,
                  fmaxf(fminf(x->INV_T_RAW, p->INV_T_derating_zero_T),
                        p->INV_T_derating_full_T));
    b_p[0] = p->IGBT_T_derating_full_T;
    b_p[1] = p->IGBT_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    f_b = interp1(b_p, fv,
                  fmaxf(fminf(x->IGBT_T_RAW, p->IGBT_T_derating_zero_T),
                        p->IGBT_T_derating_full_T));
    b_p[0] = p->MT_derating_full_T;
    b_p[1] = p->MT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    h_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->MT_RAW, p->MT_derating_zero_T), p->MT_derating_full_T));
    b_p[0] = p->BT_derating_full_T;
    b_p[1] = p->BT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    j_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->BT_RAW, p->BT_derating_zero_T), p->BT_derating_full_T));
    b_p[0] = p->VB_derating_full_T;
    b_p[1] = p->VB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    l_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->VB_RAW, p->VB_derating_zero_T), p->VB_derating_full_T));
    b_p[0] = p->IB_derating_full_T;
    b_p[1] = p->IB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    m_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->IB_RAW, p->IB_derating_zero_T), p->IB_derating_full_T));
    varargin_1[0] = PB_derate_front;
    varargin_1[7] = PB_derate_front;
    varargin_1[14] = PB_derate_rear;
    varargin_1[21] = PB_derate_rear;
    for (b_j = 0; b_j < 4; b_j++) {
      float f3;
      varargin_1[7 * b_j + 1] = d_b;
      varargin_1[7 * b_j + 2] = f_b;
      varargin_1[7 * b_j + 3] = h_b;
      varargin_1[7 * b_j + 4] = j_b;
      varargin_1[7 * b_j + 5] = l_b;
      varargin_1[7 * b_j + 6] = m_b;
      f3 = varargin_1[7 * b_j];
      for (b_i = 0; b_i < 6; b_i++) {
        float f5;
        f5 = varargin_1[(b_i + 7 * b_j) + 1];
        if (f3 > f5) {
          f3 = f5;
        }
      }
      float f4;
      f4 = fminf(out, p->MAX_TO_ABS_PO * f3);
      y->TORQUE_OUT[b_j] = f4;
      y->TO_BL_PO[b_j] = f4;
    }
  } else if (x->TH_RAW < 0.0F) {
    float varargin_1[28];
    float b_p[2];
    float fv[2];
    float b;
    float b_b;
    float b_out;
    float c_b;
    float e_b;
    float g_b;
    float i_b;
    float k_b;
    b_out = y->TH_RG * p->MAX_TO_ABS_RG;
    b_p[0] = p->GS_RG_derating_full;
    b_p[1] = p->GS_RG_derating_zero;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    b = interp1(b_p, fv,
                fmaxf(fminf(x->GS_RAW, p->GS_RG_derating_zero),
                      p->GS_RG_derating_full));
    b_p[0] = p->INV_T_derating_full_T;
    b_p[1] = p->INV_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    b_b = interp1(b_p, fv,
                  fmaxf(fminf(x->INV_T_RAW, p->INV_T_derating_zero_T),
                        p->INV_T_derating_full_T));
    b_p[0] = p->IGBT_T_derating_full_T;
    b_p[1] = p->IGBT_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    c_b = interp1(b_p, fv,
                  fmaxf(fminf(x->IGBT_T_RAW, p->IGBT_T_derating_zero_T),
                        p->IGBT_T_derating_full_T));
    b_p[0] = p->MT_derating_full_T;
    b_p[1] = p->MT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    e_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->MT_RAW, p->MT_derating_zero_T), p->MT_derating_full_T));
    b_p[0] = p->BT_derating_full_T;
    b_p[1] = p->BT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    g_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->BT_RAW, p->BT_derating_zero_T), p->BT_derating_full_T));
    b_p[0] = p->VB_RG_derating_full_T;
    b_p[1] = p->VB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    i_b = interp1(b_p, fv,
                  fmaxf(fminf(x->VB_RAW, p->VB_RG_derating_zero_T),
                        p->VB_RG_derating_full_T));
    b_p[0] = p->IB_RG_derating_full_T;
    b_p[1] = p->IB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    k_b = interp1(b_p, fv,
                  fmaxf(fminf(x->IB_RAW, p->IB_RG_derating_zero_T),
                        p->IB_RG_derating_full_T));
    for (j = 0; j < 4; j++) {
      float f;
      varargin_1[7 * j] = b_b;
      varargin_1[7 * j + 1] = c_b;
      varargin_1[7 * j + 2] = e_b;
      varargin_1[7 * j + 3] = g_b;
      varargin_1[7 * j + 4] = i_b;
      varargin_1[7 * j + 5] = k_b;
      varargin_1[7 * j + 6] = b;
      f = b_b;
      for (i = 0; i < 6; i++) {
        float f2;
        f2 = varargin_1[(i + 7 * j) + 1];
        if (f > f2) {
          f = f2;
        }
      }
      float f1;
      f1 = -fminf(b_out, p->MAX_TO_ABS_RG * f);
      y->TO_BL_RG[j] = f1;
      y->TORQUE_OUT[j] = f1;
    }
  }
}
