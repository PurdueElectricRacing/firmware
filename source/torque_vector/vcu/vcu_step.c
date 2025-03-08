#include "vcu.h"
#include <math.h>
#include <string.h>

static float interp1(const float varargin_1[2], const float varargin_2[2],
                     float varargin_3);

static float interp2(const float varargin_1[53], const float varargin_2[51],
                     const float varargin_3[2703], float varargin_4,
                     float varargin_5);

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

static float interp2(const float varargin_1[53], const float varargin_2[51],
                     const float varargin_3[2703], float varargin_4,
                     float varargin_5)
{
  float Vq;
  if ((varargin_4 >= varargin_1[0]) && (varargin_4 <= varargin_1[52]) &&
      (varargin_5 >= varargin_2[0]) && (varargin_5 <= varargin_2[50])) {
    float f1;
    float qx1;
    float qx2;
    int b_high_i;
    int b_low_i;
    int b_low_ip1;
    int high_i;
    int low_i;
    int low_ip1;
    low_i = 0;
    low_ip1 = 2;
    high_i = 53;
    while (high_i > low_ip1) {
      int mid_i;
      mid_i = ((low_i + high_i) + 1) >> 1;
      if (varargin_4 >= varargin_1[mid_i - 1]) {
        low_i = mid_i - 1;
        low_ip1 = mid_i + 1;
      } else {
        high_i = mid_i;
      }
    }
    b_low_i = 1;
    b_low_ip1 = 2;
    b_high_i = 51;
    while (b_high_i > b_low_ip1) {
      int b_mid_i;
      b_mid_i = (b_low_i + b_high_i) >> 1;
      if (varargin_5 >= varargin_2[b_mid_i - 1]) {
        b_low_i = b_mid_i;
        b_low_ip1 = b_mid_i + 1;
      } else {
        b_high_i = b_mid_i;
      }
    }
    if (varargin_4 == varargin_1[low_i]) {
      int qx1_tmp;
      qx1_tmp = b_low_i + 51 * low_i;
      qx1 = varargin_3[qx1_tmp - 1];
      qx2 = varargin_3[qx1_tmp];
    } else {
      float f;
      f = varargin_1[low_i + 1];
      if (varargin_4 == f) {
        int b_qx1_tmp;
        b_qx1_tmp = b_low_i + 51 * (low_i + 1);
        qx1 = varargin_3[b_qx1_tmp - 1];
        qx2 = varargin_3[b_qx1_tmp];
      } else {
        float b_qx2_tmp;
        float c_qx1_tmp;
        float d_qx1_tmp;
        float qx2_tmp;
        float rx;
        int b_qx1_tmp_tmp;
        int qx1_tmp_tmp;
        rx = (varargin_4 - varargin_1[low_i]) / (f - varargin_1[low_i]);
        qx1_tmp_tmp = b_low_i + 51 * low_i;
        c_qx1_tmp = varargin_3[qx1_tmp_tmp - 1];
        b_qx1_tmp_tmp = b_low_i + 51 * (low_i + 1);
        d_qx1_tmp = varargin_3[b_qx1_tmp_tmp - 1];
        if (c_qx1_tmp == d_qx1_tmp) {
          qx1 = c_qx1_tmp;
        } else {
          qx1 = (1.0F - rx) * c_qx1_tmp + rx * d_qx1_tmp;
        }
        qx2_tmp = varargin_3[qx1_tmp_tmp];
        b_qx2_tmp = varargin_3[b_qx1_tmp_tmp];
        if (qx2_tmp == b_qx2_tmp) {
          qx2 = qx2_tmp;
        } else {
          qx2 = (1.0F - rx) * qx2_tmp + rx * b_qx2_tmp;
        }
      }
    }
    f1 = varargin_2[b_low_i - 1];
    if ((varargin_5 == f1) || (qx1 == qx2)) {
      Vq = qx1;
    } else if (varargin_5 == varargin_2[b_low_i]) {
      Vq = qx2;
    } else {
      float ry;
      ry = (varargin_5 - f1) / (varargin_2[b_low_i] - f1);
      Vq = (1.0F - ry) * qx1 + ry * qx2;
    }
  } else {
    Vq = 0.0F;
  }
  return Vq;
}

void vcu_step(const pVCU_struct *p, const fVCU_struct *f, const xVCU_struct *x,
              yVCU_struct *y)
{
  float d_varargin_1[506];
  float varargin_2[506];
  float d_f[14];
  float d_p[14];
  float c_f[11];
  float c_p[11];
  float b_f[9];
  float b_p[9];
  float e_p[2];
  float fv[2];
  float b_x;
  float e_f;
  float f1;
  float f2;
  float f3;
  float f4;
  float f5;
  float f6;
  float out;
  int b_j1;
  int b_k;
  int c_k;
  int d_k;
  int e_k;
  int f_k;
  int g_k;
  int i;
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  int i6;
  int k;
  bool c_varargin_1[37];
  bool b_varargin_1[28];
  bool VT_sensors[23];
  bool b_VT_sensors[23];
  bool varargin_1[22];
  bool VS_sensors[17];
  bool b_VS_sensors[17];
  bool PT_sensors[13];
  bool b_PT_sensors[13];
  bool b_minval;
  bool c_minval;
  bool minval;
  PT_sensors[0] = (x->TH_RAW >= p->TH_lb);
  PT_sensors[1] = (x->VB_RAW >= p->VB_lb);
  PT_sensors[6] = (x->IB_RAW >= p->IB_lb);
  PT_sensors[7] = (x->MT_RAW >= p->MT_lb);
  PT_sensors[8] = (x->CT_RAW >= p->CT_lb);
  PT_sensors[9] = (x->IT_RAW >= p->IT_lb);
  PT_sensors[10] = (x->MC_RAW >= p->MC_lb);
  PT_sensors[11] = (x->IC_RAW >= p->IC_lb);
  PT_sensors[12] = (x->BT_RAW >= p->BT_lb);
  b_PT_sensors[0] = (x->TH_RAW <= p->TH_ub);
  b_PT_sensors[1] = (x->VB_RAW <= p->VB_ub);
  PT_sensors[2] = (x->WT_RAW[0] >= p->WT_lb[0]);
  PT_sensors[4] = (x->WM_RAW[0] >= p->WM_lb[0]);
  b_PT_sensors[2] = (x->WT_RAW[0] <= p->WT_ub[0]);
  b_PT_sensors[4] = (x->WM_RAW[0] <= p->WM_ub[0]);
  PT_sensors[3] = (x->WT_RAW[1] >= p->WT_lb[1]);
  PT_sensors[5] = (x->WM_RAW[1] >= p->WM_lb[1]);
  b_PT_sensors[3] = (x->WT_RAW[1] <= p->WT_ub[1]);
  b_PT_sensors[5] = (x->WM_RAW[1] <= p->WM_ub[1]);
  b_PT_sensors[6] = (x->IB_RAW <= p->IB_ub);
  b_PT_sensors[7] = (x->MT_RAW <= p->MT_ub);
  b_PT_sensors[8] = (x->CT_RAW <= p->CT_ub);
  b_PT_sensors[9] = (x->IT_RAW <= p->IT_ub);
  b_PT_sensors[10] = (x->MC_RAW <= p->MC_ub);
  b_PT_sensors[11] = (x->IC_RAW <= p->IC_ub);
  b_PT_sensors[12] = (x->BT_RAW <= p->BT_ub);
  b_p[0] = p->CS_SFLAG_True;
  b_p[1] = p->TB_SFLAG_True;
  b_p[2] = p->WT_SFLAG_True;
  b_p[3] = p->IV_SFLAG_True;
  b_p[4] = p->BT_SFLAG_True;
  b_p[5] = p->IAC_SFLAG_True;
  b_p[6] = p->IAT_SFLAG_True;
  b_p[7] = p->IBC_SFLAG_True;
  b_p[8] = p->IBT_SFLAG_True;
  b_f[0] = f->CS_SFLAG;
  b_f[1] = f->TB_SFLAG;
  b_f[2] = f->WT_SFLAG;
  b_f[3] = f->IV_SFLAG;
  b_f[4] = f->BT_SFLAG;
  b_f[5] = f->IAC_SFLAG;
  b_f[6] = f->IAT_SFLAG;
  b_f[7] = f->IBC_SFLAG;
  b_f[8] = f->IBT_SFLAG;
  for (i = 0; i < 13; i++) {
    varargin_1[i] = (PT_sensors[i] && b_PT_sensors[i]);
  }
  for (i1 = 0; i1 < 9; i1++) {
    varargin_1[i1 + 13] = (b_p[i1] == b_f[i1]);
  }
  minval = varargin_1[0];
  for (k = 0; k < 21; k++) {
    minval = (((int)minval <= (int)varargin_1[k + 1]) && minval);
  }
  y->PT_permit_buffer[0] = y->PT_permit_buffer[1];
  y->PT_permit_buffer[1] = y->PT_permit_buffer[2];
  y->PT_permit_buffer[2] = y->PT_permit_buffer[3];
  y->PT_permit_buffer[3] = y->PT_permit_buffer[4];
  y->PT_permit_buffer[4] = minval;
  VS_sensors[0] = (x->TH_RAW >= p->TH_lb);
  VS_sensors[1] = (x->VB_RAW >= p->VB_lb);
  VS_sensors[16] = (x->VS_MAX_SR_RAW >= p->VS_MAX_SR_lb);
  b_VS_sensors[0] = (x->TH_RAW <= p->TH_ub);
  b_VS_sensors[1] = (x->VB_RAW <= p->VB_ub);
  VS_sensors[2] = (x->WT_RAW[0] >= p->WT_lb[0]);
  VS_sensors[4] = (x->WM_RAW[0] >= p->WM_lb[0]);
  VS_sensors[14] = (x->TO_RAW[0] >= p->TO_lb[0]);
  b_VS_sensors[2] = (x->WT_RAW[0] <= p->WT_ub[0]);
  b_VS_sensors[4] = (x->WM_RAW[0] <= p->WM_ub[0]);
  b_VS_sensors[14] = (x->TO_RAW[0] <= p->TO_ub[0]);
  VS_sensors[3] = (x->WT_RAW[1] >= p->WT_lb[1]);
  VS_sensors[5] = (x->WM_RAW[1] >= p->WM_lb[1]);
  VS_sensors[15] = (x->TO_RAW[1] >= p->TO_lb[1]);
  b_VS_sensors[3] = (x->WT_RAW[1] <= p->WT_ub[1]);
  b_VS_sensors[5] = (x->WM_RAW[1] <= p->WM_ub[1]);
  b_VS_sensors[15] = (x->TO_RAW[1] <= p->TO_ub[1]);
  VS_sensors[6] = (x->GS_RAW >= p->GS_lb);
  VS_sensors[7] = (x->IB_RAW >= p->IB_lb);
  VS_sensors[8] = (x->MT_RAW >= p->MT_lb);
  VS_sensors[9] = (x->CT_RAW >= p->CT_lb);
  VS_sensors[10] = (x->IT_RAW >= p->IT_lb);
  VS_sensors[11] = (x->MC_RAW >= p->MC_lb);
  VS_sensors[12] = (x->IC_RAW >= p->IC_lb);
  VS_sensors[13] = (x->BT_RAW >= p->BT_lb);
  b_VS_sensors[6] = (x->GS_RAW <= p->GS_ub);
  b_VS_sensors[7] = (x->IB_RAW <= p->IB_ub);
  b_VS_sensors[8] = (x->MT_RAW <= p->MT_ub);
  b_VS_sensors[9] = (x->CT_RAW <= p->CT_ub);
  b_VS_sensors[10] = (x->IT_RAW <= p->IT_ub);
  b_VS_sensors[11] = (x->MC_RAW <= p->MC_ub);
  b_VS_sensors[12] = (x->IC_RAW <= p->IC_ub);
  b_VS_sensors[13] = (x->BT_RAW <= p->BT_ub);
  b_VS_sensors[16] = (x->VS_MAX_SR_RAW <= p->VS_MAX_SR_ub);
  c_p[0] = p->CS_SFLAG_True;
  c_p[1] = p->TB_SFLAG_True;
  c_p[2] = p->WT_SFLAG_True;
  c_p[3] = p->IV_SFLAG_True;
  c_p[4] = p->BT_SFLAG_True;
  c_p[5] = p->IAC_SFLAG_True;
  c_p[6] = p->IAT_SFLAG_True;
  c_p[7] = p->IBC_SFLAG_True;
  c_p[8] = p->IBT_SFLAG_True;
  c_p[9] = p->GS_FFLAG_True;
  c_p[10] = p->VCU_PFLAG_VS;
  c_f[0] = f->CS_SFLAG;
  c_f[1] = f->TB_SFLAG;
  c_f[2] = f->WT_SFLAG;
  c_f[3] = f->IV_SFLAG;
  c_f[4] = f->BT_SFLAG;
  c_f[5] = f->IAC_SFLAG;
  c_f[6] = f->IAT_SFLAG;
  c_f[7] = f->IBC_SFLAG;
  c_f[8] = f->IBT_SFLAG;
  c_f[9] = f->GS_FFLAG;
  c_f[10] = f->VCU_PFLAG;
  for (i2 = 0; i2 < 17; i2++) {
    b_varargin_1[i2] = (VS_sensors[i2] && b_VS_sensors[i2]);
  }
  for (i3 = 0; i3 < 11; i3++) {
    b_varargin_1[i3 + 17] = (c_p[i3] == c_f[i3]);
  }
  b_minval = b_varargin_1[0];
  for (b_k = 0; b_k < 27; b_k++) {
    b_minval = (((int)b_minval <= (int)b_varargin_1[b_k + 1]) && b_minval);
  }
  y->VS_permit_buffer[0] = y->VS_permit_buffer[1];
  y->VS_permit_buffer[1] = y->VS_permit_buffer[2];
  y->VS_permit_buffer[2] = y->VS_permit_buffer[3];
  y->VS_permit_buffer[3] = y->VS_permit_buffer[4];
  y->VS_permit_buffer[4] = b_minval;
  VT_sensors[0] = (x->TH_RAW >= p->TH_lb);
  VT_sensors[1] = (x->ST_RAW >= p->ST_lb);
  VT_sensors[2] = (x->VB_RAW >= p->VB_lb);
  VT_sensors[3] = (x->WT_RAW[0] >= p->WT_lb[0]);
  VT_sensors[5] = (x->WM_RAW[0] >= p->WM_lb[0]);
  VT_sensors[4] = (x->WT_RAW[1] >= p->WT_lb[1]);
  VT_sensors[6] = (x->WM_RAW[1] >= p->WM_lb[1]);
  VT_sensors[7] = (x->GS_RAW >= p->GS_lb);
  VT_sensors[8] = (x->AV_RAW[0] >= p->AV_lb[0]);
  VT_sensors[9] = (x->AV_RAW[1] >= p->AV_lb[1]);
  VT_sensors[10] = (x->AV_RAW[2] >= p->AV_lb[2]);
  VT_sensors[11] = (x->IB_RAW >= p->IB_lb);
  VT_sensors[12] = (x->MT_RAW >= p->MT_lb);
  VT_sensors[13] = (x->CT_RAW >= p->CT_lb);
  VT_sensors[14] = (x->IT_RAW >= p->IT_lb);
  VT_sensors[15] = (x->MC_RAW >= p->MC_lb);
  VT_sensors[16] = (x->IC_RAW >= p->IC_lb);
  VT_sensors[17] = (x->BT_RAW >= p->BT_lb);
  VT_sensors[20] = (x->VT_DB_RAW >= p->VT_DB_lb);
  VT_sensors[21] = (x->TV_PP_RAW >= p->TV_PP_lb);
  VT_sensors[22] = (x->TC_TR_RAW >= p->TC_TR_lb);
  b_VT_sensors[0] = (x->TH_RAW <= p->TH_ub);
  b_VT_sensors[1] = (x->ST_RAW <= p->ST_ub);
  b_VT_sensors[2] = (x->VB_RAW <= p->VB_ub);
  VT_sensors[18] = (x->TO_RAW[0] >= p->TO_lb[0]);
  b_VT_sensors[3] = (x->WT_RAW[0] <= p->WT_ub[0]);
  b_VT_sensors[5] = (x->WM_RAW[0] <= p->WM_ub[0]);
  VT_sensors[19] = (x->TO_RAW[1] >= p->TO_lb[1]);
  b_VT_sensors[4] = (x->WT_RAW[1] <= p->WT_ub[1]);
  b_VT_sensors[6] = (x->WM_RAW[1] <= p->WM_ub[1]);
  b_VT_sensors[7] = (x->GS_RAW <= p->GS_ub);
  b_VT_sensors[8] = (x->AV_RAW[0] <= p->AV_ub[0]);
  b_VT_sensors[9] = (x->AV_RAW[1] <= p->AV_ub[1]);
  b_VT_sensors[10] = (x->AV_RAW[2] <= p->AV_ub[2]);
  b_VT_sensors[11] = (x->IB_RAW <= p->IB_ub);
  b_VT_sensors[12] = (x->MT_RAW <= p->MT_ub);
  b_VT_sensors[13] = (x->CT_RAW <= p->CT_ub);
  b_VT_sensors[14] = (x->IT_RAW <= p->IT_ub);
  b_VT_sensors[15] = (x->MC_RAW <= p->MC_ub);
  b_VT_sensors[16] = (x->IC_RAW <= p->IC_ub);
  b_VT_sensors[17] = (x->BT_RAW <= p->BT_ub);
  b_VT_sensors[18] = (x->TO_RAW[0] <= p->TO_ub[0]);
  b_VT_sensors[19] = (x->TO_RAW[1] <= p->TO_ub[1]);
  b_VT_sensors[20] = (x->VT_DB_RAW <= p->VT_DB_ub);
  b_VT_sensors[21] = (x->TV_PP_RAW <= p->TV_PP_ub);
  b_VT_sensors[22] = (x->TC_TR_RAW <= p->TC_TR_ub);
  d_p[0] = p->CS_SFLAG_True;
  d_p[1] = p->TB_SFLAG_True;
  d_p[2] = p->SS_SFLAG_True;
  d_p[3] = p->WT_SFLAG_True;
  d_p[4] = p->IV_SFLAG_True;
  d_p[5] = p->BT_SFLAG_True;
  d_p[6] = p->IAC_SFLAG_True;
  d_p[7] = p->IAT_SFLAG_True;
  d_p[8] = p->IBC_SFLAG_True;
  d_p[9] = p->IBT_SFLAG_True;
  d_p[10] = p->SS_FFLAG_True;
  d_p[11] = p->AV_FFLAG_True;
  d_p[12] = p->GS_FFLAG_True;
  d_p[13] = p->VCU_PFLAG_VT;
  d_f[0] = f->CS_SFLAG;
  d_f[1] = f->TB_SFLAG;
  d_f[2] = f->SS_SFLAG;
  d_f[3] = f->WT_SFLAG;
  d_f[4] = f->IV_SFLAG;
  d_f[5] = f->BT_SFLAG;
  d_f[6] = f->IAC_SFLAG;
  d_f[7] = f->IAT_SFLAG;
  d_f[8] = f->IBC_SFLAG;
  d_f[9] = f->IBT_SFLAG;
  d_f[10] = f->SS_FFLAG;
  d_f[11] = f->AV_FFLAG;
  d_f[12] = f->GS_FFLAG;
  d_f[13] = f->VCU_PFLAG;
  for (i4 = 0; i4 < 23; i4++) {
    c_varargin_1[i4] = (VT_sensors[i4] && b_VT_sensors[i4]);
  }
  for (i5 = 0; i5 < 14; i5++) {
    c_varargin_1[i5 + 23] = (d_p[i5] == d_f[i5]);
  }
  c_minval = c_varargin_1[0];
  for (c_k = 0; c_k < 36; c_k++) {
    c_minval = (((int)c_minval <= (int)c_varargin_1[c_k + 1]) && c_minval);
  }
  y->VT_permit_buffer[0] = y->VT_permit_buffer[1];
  y->VT_permit_buffer[1] = y->VT_permit_buffer[2];
  y->VT_permit_buffer[2] = y->VT_permit_buffer[3];
  y->VT_permit_buffer[3] = y->VT_permit_buffer[4];
  y->VT_permit_buffer[4] = c_minval;
  if (f->VCU_CFLAG == p->VCU_CFLAG_CS) {
    if (roundf(((((y->VS_permit_buffer[0] + y->VS_permit_buffer[1]) +
                  y->VS_permit_buffer[2]) +
                 y->VS_permit_buffer[3]) +
                y->VS_permit_buffer[4]) /
               5.0F) != 0.0F) {
      y->VCU_mode = 3.0F;
    } else {
      y->VCU_mode = 0.0F;
    }
  } else if (roundf(((((y->VT_permit_buffer[0] + y->VT_permit_buffer[1]) +
                       y->VT_permit_buffer[2]) +
                      y->VT_permit_buffer[3]) +
                     y->VT_permit_buffer[4]) /
                    5.0F) != 0.0F) {
    y->VCU_mode = 4.0F;
  } else if (roundf(((((y->PT_permit_buffer[0] + y->PT_permit_buffer[1]) +
                       y->PT_permit_buffer[2]) +
                      y->PT_permit_buffer[3]) +
                     y->PT_permit_buffer[4]) /
                    5.0F) != 0.0F) {
    y->VCU_mode = 2.0F;
  } else {
    y->VCU_mode = 1.0F;
  }
  y->TH_CF = fmaxf(fminf(x->TH_RAW, p->TH_ub), p->TH_lb);
  y->ST_CF = fmaxf(fminf(x->ST_RAW, p->ST_ub), p->ST_lb);
  y->VB_CF = fmaxf(fminf(x->VB_RAW, p->VB_ub), p->VB_lb);
  y->WT_CF[0] = fmaxf(fminf(x->WT_RAW[0], p->WT_ub[0]), p->WT_lb[0]);
  e_f = fmaxf(fminf(x->WM_RAW[0], p->WM_ub[0]), p->WM_lb[0]);
  y->TO_CF[0] = e_f;
  y->WM_CF[0] = e_f;
  y->WT_CF[1] = fmaxf(fminf(x->WT_RAW[1], p->WT_ub[1]), p->WT_lb[1]);
  e_f = fmaxf(fminf(x->WM_RAW[1], p->WM_ub[1]), p->WM_lb[1]);
  y->TO_CF[1] = e_f;
  y->WM_CF[1] = e_f;
  y->GS_CF = fmaxf(fminf(x->GS_RAW, p->GS_ub), p->GS_lb);
  f1 = x->AV_RAW[0];
  f2 = x->AV_RAW[1];
  f3 = x->AV_RAW[2];
  for (d_k = 0; d_k < 3; d_k++) {
    y->AV_CF[d_k] =
        fmaxf(fminf((p->R[d_k] * f1 + p->R[d_k + 3] * f2) + p->R[d_k + 6] * f3,
                    p->AV_ub[d_k]),
              p->AV_lb[d_k]);
  }
  for (i6 = 0; i6 < 9; i6++) {
    y->IB_CF_buffer[i6] = y->IB_CF_buffer[i6 + 1];
  }
  y->IB_CF_buffer[9] = fmaxf(fminf(x->IB_RAW, p->IB_ub), p->IB_lb);
  b_x = y->IB_CF_buffer[0];
  for (e_k = 0; e_k < 9; e_k++) {
    b_x += y->IB_CF_buffer[e_k + 1];
  }
  y->IB_CF = b_x / 10.0F;
  y->MT_CF = fmaxf(fminf(x->MT_RAW, p->MT_ub), p->MT_lb);
  y->CT_CF = fmaxf(fminf(x->CT_RAW, p->CT_ub), p->CT_lb);
  y->IT_CF = fmaxf(fminf(x->IT_RAW, p->IT_ub), p->IT_lb);
  y->MC_CF = fmaxf(fminf(x->MC_RAW, p->MC_ub), p->MC_lb);
  y->IC_CF = fmaxf(fminf(x->IC_RAW, p->IC_ub), p->IC_lb);
  y->BT_CF = fmaxf(fminf(x->BT_RAW, p->BT_ub), p->BT_lb);
  f4 = x->AG_RAW[0];
  f5 = x->AG_RAW[1];
  f6 = x->AG_RAW[2];
  for (f_k = 0; f_k < 3; f_k++) {
    y->AG_CF[f_k] =
        fmaxf(fminf((p->R[f_k] * f4 + p->R[f_k + 3] * f5) + p->R[f_k + 6] * f6,
                    p->AG_ub[f_k]),
              p->AG_lb[f_k]);
  }
  y->TO_CF[0] = fmaxf(fminf(x->TO_RAW[0], p->TO_ub[0]), p->TO_lb[0]);
  y->TO_CF[1] = fmaxf(fminf(x->TO_RAW[1], p->TO_ub[1]), p->TO_lb[1]);
  y->VT_DB_CF = fmaxf(fminf(x->VT_DB_RAW, p->VT_DB_ub), p->VT_DB_lb);
  y->TV_PP_CF = fmaxf(fminf(x->TV_PP_RAW, p->TV_PP_ub), p->TV_PP_lb);
  y->TC_TR_CF = fmaxf(fminf(x->TC_TR_RAW, p->TC_TR_ub), p->TC_TR_lb);
  y->VS_MAX_SR_CF =
      fmaxf(fminf(x->VS_MAX_SR_RAW, p->VS_MAX_SR_ub), p->VS_MAX_SR_lb);
  y->zero_current_counter =
      (y->zero_current_counter + 1.0F) * (float)(y->IB_CF == 0.0F);
  if (y->zero_current_counter >= p->zero_currents_to_update_SOC) {
    float capacity_used;
    float xi;
    y->Batt_Voc = y->VB_CF;
    xi = y->VB_CF / p->Ns;
    memcpy(&d_varargin_1[0], &p->Batt_Voc_brk[0], 506U * sizeof(float));
    memcpy(&varargin_2[0], &p->Batt_As_Discharged_tbl[0], 506U * sizeof(float));
    if (p->Batt_Voc_brk[1] < p->Batt_Voc_brk[0]) {
      for (b_j1 = 0; b_j1 < 253; b_j1++) {
        float b_xtmp;
        float xtmp;
        xtmp = d_varargin_1[b_j1];
        d_varargin_1[b_j1] = d_varargin_1[505 - b_j1];
        d_varargin_1[505 - b_j1] = xtmp;
        b_xtmp = varargin_2[b_j1];
        varargin_2[b_j1] = varargin_2[505 - b_j1];
        varargin_2[505 - b_j1] = b_xtmp;
      }
    }
    capacity_used = 0.0F;
    if ((xi <= d_varargin_1[505]) && (xi >= d_varargin_1[0])) {
      float r;
      int high_i;
      int low_i;
      int low_ip1;
      low_i = 1;
      low_ip1 = 2;
      high_i = 506;
      while (high_i > low_ip1) {
        int mid_i;
        mid_i = (low_i + high_i) >> 1;
        if (xi >= d_varargin_1[mid_i - 1]) {
          low_i = mid_i;
          low_ip1 = mid_i + 1;
        } else {
          high_i = mid_i;
        }
      }
      float r_tmp;
      r_tmp = d_varargin_1[low_i - 1];
      r = (xi - r_tmp) / (d_varargin_1[low_i] - r_tmp);
      if (r == 0.0F) {
        capacity_used = varargin_2[low_i - 1];
      } else if (r == 1.0F) {
        capacity_used = varargin_2[low_i];
      } else {
        float capacity_used_tmp;
        capacity_used_tmp = varargin_2[low_i - 1];
        if (capacity_used_tmp == varargin_2[low_i]) {
          capacity_used = capacity_used_tmp;
        } else {
          capacity_used =
              (1.0F - r) * capacity_used_tmp + r * varargin_2[low_i];
        }
      }
    }
    e_p[0] = p->Batt_cell_zero_SOC_capacity;
    e_p[1] = p->Batt_cell_full_SOC_capacity;
    fv[0] = 0.0F;
    fv[1] = 1.0F;
    y->Batt_SOC = fmaxf(fminf(interp1(e_p, fv, capacity_used), 1.0F), 0.0F);
  }
  out = y->TH_CF * p->MAX_TORQUE_NOM;
  y->TO_ET[0] = out;
  y->WM_CS[0] = p->MAX_SPEED_NOM;
  y->TO_ET[1] = out;
  y->WM_CS[1] = p->MAX_SPEED_NOM;
  if (y->VCU_mode >= 2.0F) {
    float e_varargin_1[8];
    float f_p[2];
    float fv1[2];
    float fv2[2];
    float fv3[2];
    float fv4[2];
    float fv5[2];
    float fv6[2];
    float fv7[2];
    float g_p[2];
    float h_p[2];
    float i_p[2];
    float j_p[2];
    float k_p[2];
    float l_p[2];
    float Xqk;
    float Yqk;
    float c_idx_0;
    float c_idx_1;
    float c_x;
    float ex;
    float f7;
    c_idx_0 = y->WT_CF[0] * p->gr;
    c_idx_1 = y->WT_CF[1] * p->gr;
    if (c_idx_0 < c_idx_1) {
      f7 = c_idx_1;
    } else {
      f7 = c_idx_0;
    }
    Xqk = fmaxf(fminf(f7, p->PT_WM_ub), p->PT_WM_lb);
    Yqk = fmaxf(fminf(y->VB_CF, p->PT_VB_ub), p->PT_VB_lb);
    if ((Xqk >= p->PT_WM_brkpt[0]) && (Xqk <= p->PT_WM_brkpt[149]) &&
        (Yqk >= p->PT_VB_brkpt[0]) && (Yqk <= p->PT_VB_brkpt[49])) {
      float f10;
      float qx1;
      float qx2;
      int b_high_i;
      int b_low_i;
      int b_low_ip1;
      int c_high_i;
      int c_low_i;
      int c_low_ip1;
      b_low_i = 0;
      b_low_ip1 = 2;
      b_high_i = 150;
      while (b_high_i > b_low_ip1) {
        int b_mid_i;
        b_mid_i = ((b_low_i + b_high_i) + 1) >> 1;
        if (Xqk >= p->PT_WM_brkpt[b_mid_i - 1]) {
          b_low_i = b_mid_i - 1;
          b_low_ip1 = b_mid_i + 1;
        } else {
          b_high_i = b_mid_i;
        }
      }
      c_low_i = 1;
      c_low_ip1 = 2;
      c_high_i = 50;
      while (c_high_i > c_low_ip1) {
        int c_mid_i;
        c_mid_i = (c_low_i + c_high_i) >> 1;
        if (Yqk >= p->PT_VB_brkpt[c_mid_i - 1]) {
          c_low_i = c_mid_i;
          c_low_ip1 = c_mid_i + 1;
        } else {
          c_high_i = c_mid_i;
        }
      }
      if (Xqk == p->PT_WM_brkpt[b_low_i]) {
        int qx1_tmp;
        qx1_tmp = c_low_i + 50 * b_low_i;
        qx1 = p->PT_TO_table[qx1_tmp - 1];
        qx2 = p->PT_TO_table[qx1_tmp];
      } else {
        float f9;
        f9 = p->PT_WM_brkpt[b_low_i + 1];
        if (Xqk == f9) {
          int b_qx1_tmp;
          b_qx1_tmp = c_low_i + 50 * (b_low_i + 1);
          qx1 = p->PT_TO_table[b_qx1_tmp - 1];
          qx2 = p->PT_TO_table[b_qx1_tmp];
        } else {
          float b_qx2_tmp;
          float c_qx1_tmp;
          float d_qx1_tmp;
          float qx2_tmp;
          float rx;
          int b_qx1_tmp_tmp;
          int qx1_tmp_tmp;
          rx = (Xqk - p->PT_WM_brkpt[b_low_i]) / (f9 - p->PT_WM_brkpt[b_low_i]);
          qx1_tmp_tmp = c_low_i + 50 * b_low_i;
          c_qx1_tmp = p->PT_TO_table[qx1_tmp_tmp - 1];
          b_qx1_tmp_tmp = c_low_i + 50 * (b_low_i + 1);
          d_qx1_tmp = p->PT_TO_table[b_qx1_tmp_tmp - 1];
          if (c_qx1_tmp == d_qx1_tmp) {
            qx1 = c_qx1_tmp;
          } else {
            qx1 = (1.0F - rx) * c_qx1_tmp + rx * d_qx1_tmp;
          }
          qx2_tmp = p->PT_TO_table[qx1_tmp_tmp];
          b_qx2_tmp = p->PT_TO_table[b_qx1_tmp_tmp];
          if (qx2_tmp == b_qx2_tmp) {
            qx2 = qx2_tmp;
          } else {
            qx2 = (1.0F - rx) * qx2_tmp + rx * b_qx2_tmp;
          }
        }
      }
      f10 = p->PT_VB_brkpt[c_low_i - 1];
      if ((Yqk == f10) || (qx1 == qx2)) {
        c_x = qx1;
      } else if (Yqk == p->PT_VB_brkpt[c_low_i]) {
        c_x = qx2;
      } else {
        float ry;
        ry = (Yqk - f10) / (p->PT_VB_brkpt[c_low_i] - f10);
        c_x = (1.0F - ry) * qx1 + ry * qx2;
      }
    } else {
      c_x = 0.0F;
    }
    y->TO_AB_MX = fmaxf(fminf(c_x, p->MAX_TORQUE_NOM), 0.0F);
    e_p[0] = p->mT_derating_full_T;
    e_p[1] = p->mT_derating_zero_T;
    f_p[0] = p->cT_derating_full_T;
    f_p[1] = p->cT_derating_zero_T;
    g_p[0] = p->iT_derating_full_T;
    g_p[1] = p->iT_derating_zero_T;
    h_p[0] = p->Cm_derating_full_T;
    h_p[1] = p->Cm_derating_zero_T;
    i_p[0] = p->Ci_derating_full_T;
    i_p[1] = p->Ci_derating_zero_T;
    j_p[0] = p->bT_derating_full_T;
    j_p[1] = p->bT_derating_zero_T;
    k_p[0] = p->bI_derating_full_T;
    k_p[1] = p->bI_derating_zero_T;
    l_p[0] = p->Vb_derating_full_T;
    l_p[1] = p->Vb_derating_zero_T;
    fv[0] = 1.0F;
    fv1[0] = 1.0F;
    fv2[0] = 1.0F;
    fv3[0] = 1.0F;
    fv4[0] = 1.0F;
    fv5[0] = 1.0F;
    fv6[0] = 1.0F;
    fv7[0] = 1.0F;
    fv[1] = 0.0F;
    fv1[1] = 0.0F;
    fv2[1] = 0.0F;
    fv3[1] = 0.0F;
    fv4[1] = 0.0F;
    fv5[1] = 0.0F;
    fv6[1] = 0.0F;
    fv7[1] = 0.0F;
    e_varargin_1[0] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(e_p, fv, y->MT_CF), 1.0F), 0.0F);
    e_varargin_1[1] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(f_p, fv1, y->CT_CF), 1.0F), 0.0F);
    e_varargin_1[2] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(g_p, fv2, y->IT_CF), 1.0F), 0.0F);
    e_varargin_1[3] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(h_p, fv3, y->MC_CF), 1.0F), 0.0F);
    e_varargin_1[4] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(i_p, fv4, y->IC_CF), 1.0F), 0.0F);
    e_varargin_1[5] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(j_p, fv5, y->BT_CF), 1.0F), 0.0F);
    e_varargin_1[6] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(k_p, fv6, y->IB_CF), 1.0F), 0.0F);
    e_varargin_1[7] = p->MAX_TORQUE_NOM *
                      fmaxf(fminf(interp1(l_p, fv7, y->VB_CF), 1.0F), 0.0F);
    ex = e_varargin_1[0];
    for (g_k = 0; g_k < 7; g_k++) {
      float f11;
      f11 = e_varargin_1[g_k + 1];
      if (ex > f11) {
        ex = f11;
      }
    }
    float a;
    y->TO_DR_MX = ex;
    a = fminf(y->TO_AB_MX * y->TH_CF, ex);
    y->TO_PT[0] = a;
    y->TO_PT[1] = a;
  }
  if (y->VCU_mode == 3.0F) {
    float b_out;
    y->SR_VS = y->TH_CF * y->VS_MAX_SR_CF;
    b_out = fmaxf(p->WM_VS_LS, y->GS_CF / p->r * (y->SR_VS + 1.0F));
    y->WM_VS[0] = b_out;
    y->WM_VS[1] = b_out;
  }
  if (y->VCU_mode == 4.0F) {
    float f8;
    f8 = fabsf(y->ST_CF);
    if (f8 < y->VT_DB_CF - p->dST_DB) {
      y->VT_mode = 1.0F;
      y->SR = 0.5F * (y->WT_CF[0] + y->WT_CF[1]) * p->r /
                  fmaxf(fminf(y->GS_CF, y->GS_CF), p->TC_eps) -
              1.0F;
      y->TC_highs = (y->TC_highs + 1.0F) * (float)(y->SR >= p->TC_SR_threshold);
      y->TC_lows = (y->TC_lows + 1.0F) * (float)(y->SR < p->TC_SR_threshold);
      if (y->TC_highs >= p->TC_highs_to_engage) {
        y->TO_VT[0] = y->TO_PT[0] * y->TC_TR_CF;
        y->TO_VT[1] = y->TO_PT[1] * y->TC_TR_CF;
      } else if (y->TC_lows >= p->TC_lows_to_disengage) {
        y->TO_VT[0] = y->TO_PT[0];
        y->TO_VT[1] = y->TO_PT[1];
      }
    } else if (f8 > y->VT_DB_CF + p->dST_DB) {
      y->VT_mode = 2.0F;
      y->TV_AV_ref = interp2(p->TV_ST_brkpt, p->TV_GS_brkpt, p->TV_AV_table,
                             fmaxf(fminf(y->ST_CF, p->TV_ST_ub), p->TV_ST_lb),
                             fmaxf(fminf(y->GS_CF, p->TV_GS_ub), p->TV_GS_lb));
      y->TV_delta_torque = fmaxf(fminf((y->TV_AV_ref * p->TV_PI - y->AV_CF[2]) *
                                           y->TV_PP_CF * p->ht[1],
                                       y->TO_PT[0] * p->MAX_r),
                                 -y->TO_PT[0] * p->MAX_r);
      if (y->TV_delta_torque > 0.0F) {
        y->TO_VT[0] = y->TO_PT[0];
        y->TO_VT[1] = y->TO_PT[0] - y->TV_delta_torque;
      } else {
        y->TO_VT[0] = y->TO_PT[0] + y->TV_delta_torque;
        y->TO_VT[1] = y->TO_PT[0];
      }
    }
  }
}
