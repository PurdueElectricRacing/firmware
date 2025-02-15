#include "vcu.h"
#include <math.h>
#include <string.h>

static float b_interp1(const float varargin_1[2], const float varargin_2[2],
                       float varargin_3);

static float b_interp2(const float varargin_1[53], const float varargin_2[51],
                       const float varargin_3[2703], float varargin_4,
                       float varargin_5);

static float b_mean(const float x[10]);

static bool b_minimum(const bool x[22]);

static bool c_minimum(const bool x[27]);

static bool d_minimum(const bool x[37]);

static float e_minimum(const float x[8]);

static float interp1(const float varargin_1[506], const float varargin_2[506],
                     float varargin_3);

static float interp2(const float varargin_1[150], const float varargin_2[50],
                     const float varargin_3[7500], float varargin_4,
                     float varargin_5);

static float maximum(const float x[2]);

static float mean(const float x[5]);

static bool minimum(const bool x[3]);

static float b_interp1(const float varargin_1[2], const float varargin_2[2],
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

static float b_interp2(const float varargin_1[53], const float varargin_2[51],
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

static float b_mean(const float x[10])
{
  float accumulatedData;
  int k;
  accumulatedData = x[0];
  for (k = 0; k < 9; k++) {
    accumulatedData += x[k + 1];
  }
  return accumulatedData / 10.0F;
}

static bool b_minimum(const bool x[22])
{
  int k;
  bool ex;
  ex = x[0];
  for (k = 0; k < 21; k++) {
    ex = (((int)ex <= (int)x[k + 1]) && ex);
  }
  return ex;
}

static bool c_minimum(const bool x[27])
{
  int k;
  bool ex;
  ex = x[0];
  for (k = 0; k < 26; k++) {
    ex = (((int)ex <= (int)x[k + 1]) && ex);
  }
  return ex;
}

static bool d_minimum(const bool x[37])
{
  int k;
  bool ex;
  ex = x[0];
  for (k = 0; k < 36; k++) {
    ex = (((int)ex <= (int)x[k + 1]) && ex);
  }
  return ex;
}

static float e_minimum(const float x[8])
{
  float b_ex;
  int k;
  b_ex = x[0];
  for (k = 0; k < 7; k++) {
    float f;
    f = x[k + 1];
    if (b_ex > f) {
      b_ex = f;
    }
  }
  return b_ex;
}

static float interp1(const float varargin_1[506], const float varargin_2[506],
                     float varargin_3)
{
  float x[506];
  float y[506];
  float Vq;
  int b_j1;
  memcpy(&y[0], &varargin_2[0], 506U * sizeof(float));
  memcpy(&x[0], &varargin_1[0], 506U * sizeof(float));
  if (varargin_1[1] < varargin_1[0]) {
    for (b_j1 = 0; b_j1 < 253; b_j1++) {
      float b_xtmp;
      float xtmp;
      xtmp = x[b_j1];
      x[b_j1] = x[505 - b_j1];
      x[505 - b_j1] = xtmp;
      b_xtmp = y[b_j1];
      y[b_j1] = y[505 - b_j1];
      y[505 - b_j1] = b_xtmp;
    }
  }
  Vq = 0.0F;
  if ((varargin_3 <= x[505]) && (varargin_3 >= x[0])) {
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
      if (varargin_3 >= x[mid_i - 1]) {
        low_i = mid_i;
        low_ip1 = mid_i + 1;
      } else {
        high_i = mid_i;
      }
    }
    float r_tmp;
    r_tmp = x[low_i - 1];
    r = (varargin_3 - r_tmp) / (x[low_i] - r_tmp);
    if (r == 0.0F) {
      Vq = y[low_i - 1];
    } else if (r == 1.0F) {
      Vq = y[low_i];
    } else {
      float Vq_tmp;
      Vq_tmp = y[low_i - 1];
      if (Vq_tmp == y[low_i]) {
        Vq = Vq_tmp;
      } else {
        Vq = (1.0F - r) * Vq_tmp + r * y[low_i];
      }
    }
  }
  return Vq;
}

static float interp2(const float varargin_1[150], const float varargin_2[50],
                     const float varargin_3[7500], float varargin_4,
                     float varargin_5)
{
  float Vq;
  if ((varargin_4 >= varargin_1[0]) && (varargin_4 <= varargin_1[149]) &&
      (varargin_5 >= varargin_2[0]) && (varargin_5 <= varargin_2[49])) {
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
    high_i = 150;
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
    b_high_i = 50;
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
      qx1_tmp = b_low_i + 50 * low_i;
      qx1 = varargin_3[qx1_tmp - 1];
      qx2 = varargin_3[qx1_tmp];
    } else {
      float f;
      f = varargin_1[low_i + 1];
      if (varargin_4 == f) {
        int b_qx1_tmp;
        b_qx1_tmp = b_low_i + 50 * (low_i + 1);
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
        qx1_tmp_tmp = b_low_i + 50 * low_i;
        c_qx1_tmp = varargin_3[qx1_tmp_tmp - 1];
        b_qx1_tmp_tmp = b_low_i + 50 * (low_i + 1);
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

static float maximum(const float x[2])
{
  float ex;
  if (x[0] < x[1]) {
    ex = x[1];
  } else {
    ex = x[0];
  }
  return ex;
}

static float mean(const float x[5])
{
  return ((((x[0] + x[1]) + x[2]) + x[3]) + x[4]) / 5.0F;
}

static bool minimum(const bool x[3])
{
  bool ex;
  ex = x[0];
  ex = (((signed char)x[0] <= (signed char)x[1]) && ex);
  return ((int)ex <= (int)x[2]) && ex;
}

void vcu_step(const pVCU_struct *p, const fVCU_struct *f, const xVCU_struct *x,
              yVCU_struct *y)
{
  float d_f[14];
  float e_p[14];
  float c_f[11];
  float d_p[11];
  float b_f[9];
  float c_p[9];
  float b_p[2];
  float fv[2];
  float e_f;
  float f1;
  float f2;
  float f3;
  float f4;
  float f5;
  int b_k;
  int i;
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  int i6;
  int k;
  bool c_VT_sensors[37];
  bool c_VS_sensors[27];
  bool VT_sensors[23];
  bool b_VT_sensors[23];
  bool c_PT_sensors[22];
  bool VS_sensors[16];
  bool b_VS_sensors[16];
  bool PT_sensors[13];
  bool b_PT_sensors[13];
  bool b_x[3];
  b_x[0] = ((x->TH_RAW >= p->TH_lb) && (x->TH_RAW <= p->TH_ub));
  b_x[1] = (p->CS_SFLAG_True == f->CS_SFLAG);
  b_x[2] = (p->TB_SFLAG_True == f->TB_SFLAG);
  y->ET_permit_buffer[0] = y->ET_permit_buffer[1];
  y->ET_permit_buffer[1] = y->ET_permit_buffer[2];
  y->ET_permit_buffer[2] = y->ET_permit_buffer[3];
  y->ET_permit_buffer[3] = y->ET_permit_buffer[4];
  y->ET_permit_buffer[4] = minimum(b_x);
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
  c_p[0] = p->CS_SFLAG_True;
  c_p[1] = p->TB_SFLAG_True;
  c_p[2] = p->WT_SFLAG_True;
  c_p[3] = p->IV_SFLAG_True;
  c_p[4] = p->BT_SFLAG_True;
  c_p[5] = p->IAC_SFLAG_True;
  c_p[6] = p->IAT_SFLAG_True;
  c_p[7] = p->IBC_SFLAG_True;
  c_p[8] = p->IBT_SFLAG_True;
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
    c_PT_sensors[i] = (PT_sensors[i] && b_PT_sensors[i]);
  }
  for (i1 = 0; i1 < 9; i1++) {
    c_PT_sensors[i1 + 13] = (c_p[i1] == b_f[i1]);
  }
  y->PT_permit_buffer[0] = y->PT_permit_buffer[1];
  y->PT_permit_buffer[1] = y->PT_permit_buffer[2];
  y->PT_permit_buffer[2] = y->PT_permit_buffer[3];
  y->PT_permit_buffer[3] = y->PT_permit_buffer[4];
  y->PT_permit_buffer[4] = b_minimum(c_PT_sensors);
  VS_sensors[0] = (x->TH_RAW >= p->TH_lb);
  VS_sensors[1] = (x->VB_RAW >= p->VB_lb);
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
  d_p[0] = p->CS_SFLAG_True;
  d_p[1] = p->TB_SFLAG_True;
  d_p[2] = p->WT_SFLAG_True;
  d_p[3] = p->IV_SFLAG_True;
  d_p[4] = p->BT_SFLAG_True;
  d_p[5] = p->IAC_SFLAG_True;
  d_p[6] = p->IAT_SFLAG_True;
  d_p[7] = p->IBC_SFLAG_True;
  d_p[8] = p->IBT_SFLAG_True;
  d_p[9] = p->GS_FFLAG_True;
  d_p[10] = p->VCU_PFLAG_VS;
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
  for (i2 = 0; i2 < 16; i2++) {
    c_VS_sensors[i2] = (VS_sensors[i2] && b_VS_sensors[i2]);
  }
  for (i3 = 0; i3 < 11; i3++) {
    c_VS_sensors[i3 + 16] = (d_p[i3] == c_f[i3]);
  }
  y->VS_permit_buffer[0] = y->VS_permit_buffer[1];
  y->VS_permit_buffer[1] = y->VS_permit_buffer[2];
  y->VS_permit_buffer[2] = y->VS_permit_buffer[3];
  y->VS_permit_buffer[3] = y->VS_permit_buffer[4];
  y->VS_permit_buffer[4] = c_minimum(c_VS_sensors);
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
  VT_sensors[20] = (x->DB_RAW >= p->DB_lb);
  VT_sensors[21] = (x->PI_RAW >= p->PI_lb);
  VT_sensors[22] = (x->PP_RAW >= p->PP_lb);
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
  b_VT_sensors[20] = (x->DB_RAW <= p->DB_ub);
  b_VT_sensors[21] = (x->PI_RAW <= p->PI_ub);
  b_VT_sensors[22] = (x->PP_RAW <= p->PP_ub);
  e_p[0] = p->CS_SFLAG_True;
  e_p[1] = p->TB_SFLAG_True;
  e_p[2] = p->SS_SFLAG_True;
  e_p[3] = p->WT_SFLAG_True;
  e_p[4] = p->IV_SFLAG_True;
  e_p[5] = p->BT_SFLAG_True;
  e_p[6] = p->IAC_SFLAG_True;
  e_p[7] = p->IAT_SFLAG_True;
  e_p[8] = p->IBC_SFLAG_True;
  e_p[9] = p->IBT_SFLAG_True;
  e_p[10] = p->SS_FFLAG_True;
  e_p[11] = p->AV_FFLAG_True;
  e_p[12] = p->GS_FFLAG_True;
  e_p[13] = p->VCU_PFLAG_VT;
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
    c_VT_sensors[i4] = (VT_sensors[i4] && b_VT_sensors[i4]);
  }
  for (i5 = 0; i5 < 14; i5++) {
    c_VT_sensors[i5 + 23] = (e_p[i5] == d_f[i5]);
  }
  y->VT_permit_buffer[0] = y->VT_permit_buffer[1];
  y->VT_permit_buffer[1] = y->VT_permit_buffer[2];
  y->VT_permit_buffer[2] = y->VT_permit_buffer[3];
  y->VT_permit_buffer[3] = y->VT_permit_buffer[4];
  y->VT_permit_buffer[4] = d_minimum(c_VT_sensors);
  if (roundf(mean(y->VT_permit_buffer)) != 0.0F) {
    y->VCU_mode = 4.0F;
  } else if (roundf(mean(y->VS_permit_buffer)) != 0.0F) {
    y->VCU_mode = 3.0F;
  } else if (roundf(mean(y->PT_permit_buffer)) != 0.0F) {
    y->VCU_mode = 2.0F;
  } else {
    y->VCU_mode = (float)(roundf(mean(y->ET_permit_buffer)) != 0.0F);
  }
  y->TH_CF = fmaxf(fminf(x->TH_RAW, p->TH_ub), p->TH_lb);
  y->ST_CF = fmaxf(fminf(x->ST_RAW, p->ST_ub), p->ST_lb);
  y->VB_CF = fmaxf(fminf(x->VB_RAW, p->VB_ub), p->VB_lb);
  y->WT_CF[0] = fmaxf(fminf(x->WT_RAW[0], p->WT_ub[0]), p->WT_lb[0]);
  y->WT_CF[1] = fmaxf(fminf(x->WT_RAW[1], p->WT_ub[1]), p->WT_lb[1]);
  y->GS_CF = fmaxf(fminf(x->GS_RAW, p->GS_ub), p->GS_lb);
  e_f = x->AV_RAW[0];
  f1 = x->AV_RAW[1];
  f2 = x->AV_RAW[2];
  for (k = 0; k < 3; k++) {
    y->AV_CF[k] =
        fmaxf(fminf((p->R[k] * e_f + p->R[k + 3] * f1) + p->R[k + 6] * f2,
                    p->AV_ub[k]),
              p->AV_lb[k]);
  }
  for (i6 = 0; i6 < 9; i6++) {
    y->IB_CF_buffer[i6] = y->IB_CF_buffer[i6 + 1];
  }
  y->IB_CF_buffer[9] = fmaxf(fminf(x->IB_RAW, p->IB_ub), p->IB_lb);
  y->IB_CF = b_mean(y->IB_CF_buffer);
  y->MT_CF = fmaxf(fminf(x->MT_RAW, p->MT_ub), p->MT_lb);
  y->CT_CF = fmaxf(fminf(x->CT_RAW, p->CT_ub), p->CT_lb);
  y->IT_CF = fmaxf(fminf(x->IT_RAW, p->IT_ub), p->IT_lb);
  y->MC_CF = fmaxf(fminf(x->MC_RAW, p->MC_ub), p->MC_lb);
  y->IC_CF = fmaxf(fminf(x->IC_RAW, p->IC_ub), p->IC_lb);
  y->BT_CF = fmaxf(fminf(x->BT_RAW, p->BT_ub), p->BT_lb);
  f3 = x->AG_RAW[0];
  f4 = x->AG_RAW[1];
  f5 = x->AG_RAW[2];
  for (b_k = 0; b_k < 3; b_k++) {
    y->AG_CF[b_k] =
        fmaxf(fminf((p->R[b_k] * f3 + p->R[b_k + 3] * f4) + p->R[b_k + 6] * f5,
                    p->AG_ub[b_k]),
              p->AG_lb[b_k]);
  }
  y->TO_CF[0] = fmaxf(fminf(x->TO_RAW[0], p->TO_ub[0]), p->TO_lb[0]);
  y->TO_CF[1] = fmaxf(fminf(x->TO_RAW[1], p->TO_ub[1]), p->TO_lb[1]);
  y->DB_CF = fmaxf(fminf(x->DB_RAW, p->DB_ub), p->DB_lb);
  y->PI_CF = fmaxf(fminf(x->PI_RAW, p->PI_ub), p->PI_lb);
  y->PP_CF = fmaxf(fminf(x->PP_RAW, p->PP_ub), p->PP_lb);
  y->zero_current_counter =
      (y->zero_current_counter + 1.0F) * (float)(y->IB_CF == 0.0F);
  if (y->zero_current_counter >= p->zero_currents_to_update_SOC) {
    y->Batt_Voc = y->VB_CF;
    b_p[0] = p->Batt_cell_zero_SOC_capacity;
    b_p[1] = p->Batt_cell_full_SOC_capacity;
    fv[0] = 0.0F;
    fv[1] = 1.0F;
    y->Batt_SOC = fmaxf(
        fminf(b_interp1(b_p, fv,
                        interp1(p->Batt_Voc_brk, p->Batt_As_Discharged_tbl,
                                y->VB_CF / p->Ns)),
              1.0F),
        0.0F);
  }
  if (y->VCU_mode >= 1.0F) {
    float out;
    out = y->TH_CF * p->MAX_TORQUE_NOM;
    y->TO_ET[0] = out;
    y->TO_ET[1] = out;
  }
  if (y->VCU_mode >= 2.0F) {
    float m_p[8];
    float b_y[2];
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
    float a;
    b_y[0] = y->WT_CF[0] * p->gr;
    b_y[1] = y->WT_CF[1] * p->gr;
    y->TO_AB_MX = fmaxf(
        fminf(interp2(p->PT_WM_brkpt, p->PT_VB_brkpt, p->PT_TO_table,
                      fmaxf(fminf(maximum(b_y), p->PT_WM_ub), p->PT_WM_lb),
                      fmaxf(fminf(y->VB_CF, p->PT_VB_ub), p->PT_VB_lb)),
              p->MAX_TORQUE_NOM),
        0.0F);
    b_p[0] = p->mT_derating_full_T;
    b_p[1] = p->mT_derating_zero_T;
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
    m_p[0] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(b_p, fv, y->MT_CF), 1.0F), 0.0F);
    m_p[1] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(f_p, fv1, y->CT_CF), 1.0F), 0.0F);
    m_p[2] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(g_p, fv2, y->IT_CF), 1.0F), 0.0F);
    m_p[3] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(h_p, fv3, y->MC_CF), 1.0F), 0.0F);
    m_p[4] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(i_p, fv4, y->IC_CF), 1.0F), 0.0F);
    m_p[5] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(j_p, fv5, y->BT_CF), 1.0F), 0.0F);
    m_p[6] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(k_p, fv6, y->IB_CF), 1.0F), 0.0F);
    m_p[7] = p->MAX_TORQUE_NOM *
             fmaxf(fminf(b_interp1(l_p, fv7, y->VB_CF), 1.0F), 0.0F);
    y->TO_DR_MX = e_minimum(m_p);
    a = fminf(y->TO_AB_MX * y->TH_CF, y->TO_DR_MX);
    y->TO_PT[0] = a;
    y->TO_PT[1] = a;
  }
  if (y->VCU_mode == 3.0F) {
    y->WM_VS[0] = 1.0F;
    y->WM_VS[1] = 1.0F;
  }
  if (y->VCU_mode == 4.0F) {
    float f6;
    float f7;
    f6 = fabsf(y->ST_CF);
    f7 = y->DB_CF + p->dST_DB;
    if (f6 < f7) {
      y->VT_mode = 2.0F;
    } else if (f6 > f7) {
      y->VT_mode = 1.0F;
    }
    if (y->VT_mode == 1.0F) {
      float out_tmp;
      out_tmp =
          (y->WT_CF[0] + y->WT_CF[1]) / 2.0F * p->r / (y->GS_CF + p->TC_eps);
      y->sl = out_tmp - 1.0F;
      if (out_tmp - 1.0F >= p->TC_sl_threshold) {
        y->TC_highs++;
        y->TC_lows = 0.0F;
      } else if (out_tmp - 1.0F < p->TC_sl_threshold) {
        y->TC_lows++;
        y->TC_highs = 0.0F;
      } else {
        y->TC_lows = 0.0F;
        y->TC_highs = 0.0F;
      }
      if (y->TC_highs >= p->TC_highs_to_engage) {
        y->TO_VT[0] = y->TO_PT[0] * p->TC_throttle_mult;
        y->TO_VT[1] = y->TO_PT[1] * p->TC_throttle_mult;
      } else if (y->TC_lows >= p->TC_lows_to_disengage) {
        y->TO_VT[0] = y->TO_PT[0];
        y->TO_VT[1] = y->TO_PT[1];
      }
    } else if (y->VT_mode == 2.0F) {
      y->TV_AV_ref =
          b_interp2(p->TV_ST_brkpt, p->TV_GS_brkpt, p->TV_AV_table,
                    fmaxf(fminf(y->ST_CF, p->TV_ST_ub), p->TV_ST_lb),
                    fmaxf(fminf(y->GS_CF, p->TV_GS_ub), p->TV_GS_lb));
      y->TV_delta_torque = fmaxf(
          fminf((y->TV_AV_ref * y->PI_CF - y->AV_CF[2]) * y->PP_CF * p->ht[1],
                y->TO_PT[0] * p->r_power_sat),
          -y->TO_PT[0] * p->r_power_sat);
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
