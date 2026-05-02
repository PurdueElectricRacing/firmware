#include "vcu.h"
#include <math.h>


static void b_interp1(const float varargin_1[3], const float varargin_2[3],
                      const float varargin_3[4], float Vq[4]);

static float interp1(const float varargin_1[2], const float varargin_2[2],
                     float varargin_3);

static float interp2(const float varargin_1[27], const float varargin_2[51],
                     const float varargin_3[1377], float varargin_4,
                     float varargin_5);

static void b_interp1(const float varargin_1[3], const float varargin_2[3],
                      const float varargin_3[4], float Vq[4])
{
  float x[3];
  float y[3];
  float r;
  int low_i;
  y[0] = varargin_2[0];
  x[0] = varargin_1[0];
  y[1] = varargin_2[1];
  x[1] = varargin_1[1];
  y[2] = varargin_2[2];
  x[2] = varargin_1[2];
  if (varargin_1[1] < varargin_1[0]) {
    x[0] = varargin_1[2];
    x[2] = varargin_1[0];
    y[0] = varargin_2[2];
    y[2] = varargin_2[0];
  }
  Vq[0] = 0.0F;
  if ((varargin_3[0] <= x[2]) && (varargin_3[0] >= x[0])) {
    low_i = 0;
    if (varargin_3[0] >= varargin_1[1]) {
      low_i = 1;
    }
    r = (varargin_3[0] - x[low_i]) / (x[low_i + 1] - x[low_i]);
    if (r == 0.0F) {
      Vq[0] = y[low_i];
    } else if (r == 1.0F) {
      Vq[0] = y[low_i + 1];
    } else {
      float Vq_tmp;
      Vq_tmp = y[low_i + 1];
      if (y[low_i] == Vq_tmp) {
        Vq[0] = y[low_i];
      } else {
        Vq[0] = (1.0F - r) * y[low_i] + r * Vq_tmp;
      }
    }
  }
  Vq[1] = 0.0F;
  if ((varargin_3[1] <= x[2]) && (varargin_3[1] >= x[0])) {
    low_i = 0;
    if (varargin_3[1] >= varargin_1[1]) {
      low_i = 1;
    }
    r = (varargin_3[1] - x[low_i]) / (x[low_i + 1] - x[low_i]);
    if (r == 0.0F) {
      Vq[1] = y[low_i];
    } else if (r == 1.0F) {
      Vq[1] = y[low_i + 1];
    } else {
      float b_Vq_tmp;
      b_Vq_tmp = y[low_i + 1];
      if (y[low_i] == b_Vq_tmp) {
        Vq[1] = y[low_i];
      } else {
        Vq[1] = (1.0F - r) * y[low_i] + r * b_Vq_tmp;
      }
    }
  }
  Vq[2] = 0.0F;
  if ((varargin_3[2] <= x[2]) && (varargin_3[2] >= x[0])) {
    low_i = 0;
    if (varargin_3[2] >= varargin_1[1]) {
      low_i = 1;
    }
    r = (varargin_3[2] - x[low_i]) / (x[low_i + 1] - x[low_i]);
    if (r == 0.0F) {
      Vq[2] = y[low_i];
    } else if (r == 1.0F) {
      Vq[2] = y[low_i + 1];
    } else {
      float c_Vq_tmp;
      c_Vq_tmp = y[low_i + 1];
      if (y[low_i] == c_Vq_tmp) {
        Vq[2] = y[low_i];
      } else {
        Vq[2] = (1.0F - r) * y[low_i] + r * c_Vq_tmp;
      }
    }
  }
  Vq[3] = 0.0F;
  if ((varargin_3[3] <= x[2]) && (varargin_3[3] >= x[0])) {
    low_i = 0;
    if (varargin_3[3] >= varargin_1[1]) {
      low_i = 1;
    }
    r = (varargin_3[3] - x[low_i]) / (x[low_i + 1] - x[low_i]);
    if (r == 0.0F) {
      Vq[3] = y[low_i];
    } else if (r == 1.0F) {
      Vq[3] = y[low_i + 1];
    } else {
      float d_Vq_tmp;
      d_Vq_tmp = y[low_i + 1];
      if (y[low_i] == d_Vq_tmp) {
        Vq[3] = y[low_i];
      } else {
        Vq[3] = (1.0F - r) * y[low_i] + r * d_Vq_tmp;
      }
    }
  }
}

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

static float interp2(const float varargin_1[27], const float varargin_2[51],
                     const float varargin_3[1377], float varargin_4,
                     float varargin_5)
{
  float Vq;
  if ((varargin_4 >= varargin_1[0]) && (varargin_4 <= varargin_1[26]) &&
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
    high_i = 27;
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

void vcu_step(const pVCU_struct *p, const xVCU_struct *x, yVCU_struct *y)
{
  float b_p[2];
  float fv[2];
  float b_x;
  float value_tmp;
  int b_i;
  int b_j;
  int c_i;
  int i;
  int j;
  int k;
  if (x->REGEN_RAW > 0.0F) {
    y->TH = -fminf(x->REGEN_RAW, 1.0F);
  } else {
    y->TH = fmaxf(fminf(x->THROT_RAW, 1.0F), 0.0F);
  }
  value_tmp = fmaxf(y->TH, 0.0F);
  y->TH_PO = value_tmp;
  y->TH_RG = fabsf(fminf(y->TH, 0.0F));
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
  y->AV[2] = -x->AV_RAW[2];
  y->IB = x->IB_RAW;
  y->MT = x->MT_RAW;
  y->IGBT_T = x->IGBT_T_RAW;
  y->INV_T = x->INV_T_RAW;
  y->BT = x->BT_RAW;
  y->OV_MOT[0] = x->OV_MOT[0];
  y->OV_INV[0] = x->OV_INV[0];
  y->TO[0] = x->TO_RAW[0];
  y->OV_MOT[1] = x->OV_MOT[1];
  y->OV_INV[1] = x->OV_INV[1];
  y->TO[1] = x->TO_RAW[1];
  y->OV_MOT[2] = x->OV_MOT[2];
  y->OV_INV[2] = x->OV_INV[2];
  y->TO[2] = x->TO_RAW[2];
  y->OV_MOT[3] = x->OV_MOT[3];
  y->OV_INV[3] = x->OV_INV[3];
  y->TO[3] = x->TO_RAW[3];
  y->RG_FR_split = fmaxf(fminf(x->RG_FR_split_RAW, 1.0F), 0.0F);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->SK_FR_split_lb;
  b_p[1] = p->SK_FR_split_ub;
  y->SK_FR_split = interp1(fv, b_p, x->SK_FR_split_RAW);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->SK_LR_gain_lb;
  b_p[1] = p->SK_LR_gain_ub;
  y->SK_LR_gain = interp1(fv, b_p, x->SK_LR_gain_RAW);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->AX_FR_split_lb;
  b_p[1] = p->AX_FR_split_ub;
  y->AX_FR_split = interp1(fv, b_p, x->AX_FR_split_RAW);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->AX_LR_control_force_lb;
  b_p[1] = p->AX_LR_control_force_ub;
  y->AX_LR_control_force = interp1(fv, b_p, x->AX_LR_control_force_RAW);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->TS_FR_split_lb;
  b_p[1] = p->TS_FR_split_ub;
  y->TS_FR_split = interp1(fv, b_p, x->TS_FR_split_RAW);
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->TS_LR_split_lb;
  b_p[1] = p->TS_LR_split_ub;
  y->TS_LR_split = interp1(fv, b_p, x->TS_LR_split_RAW);
  for (i = 0; i < 9; i++) {
    y->IB_AVG_buffer[i] = y->IB_AVG_buffer[i + 1];
  }
  y->IB_AVG_buffer[9] = x->IB_RAW;
  b_x = y->IB_AVG_buffer[0];
  for (k = 0; k < 9; k++) {
    b_x += y->IB_AVG_buffer[k + 1];
  }
  y->IB_AVG = b_x / 10.0F;
  y->PB = x->VB_RAW * x->IB_RAW;
  y->WW[0] = x->WM_RAW[0] / p->gr;
  y->WW[1] = x->WM_RAW[1] / p->gr;
  y->WW[2] = x->WM_RAW[2] / p->gr;
  y->WW[3] = x->WM_RAW[3] / p->gr;
  if (x->VCU_MODE_REQ == 0.0F) {
    y->VCU_MODE = 1.0F;
  } else if (x->VCU_MODE_REQ == 1.0F) {
    y->VCU_MODE = 2.0F;
  } else if (x->VCU_MODE_REQ == 2.0F) {
    y->VCU_MODE = 3.0F;
  } else if (x->VCU_MODE_REQ == 3.0F) {
    y->VCU_MODE = 4.0F;
  } else {
    y->VCU_MODE = 0.0F;
  }
  if (y->TH > 0.0F) {
    float varargin_1[28];
    float minval[4];
    float PB_derate_front;
    float PB_derate_rear;
    float PB_snipped;
    float b;
    float c_b;
    float e_b;
    float g_b;
    float i_b;
    float k_b;
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
    b = interp1(b_p, fv,
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
    b_p[0] = p->VB_derating_full_T;
    b_p[1] = p->VB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    i_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->VB_RAW, p->VB_derating_zero_T), p->VB_derating_full_T));
    b_p[0] = p->IB_derating_full_T;
    b_p[1] = p->IB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    k_b = interp1(
        b_p, fv,
        fmaxf(fminf(y->IB_AVG, p->IB_derating_zero_T), p->IB_derating_full_T));
    varargin_1[0] = PB_derate_front;
    varargin_1[7] = PB_derate_front;
    varargin_1[14] = PB_derate_rear;
    varargin_1[21] = PB_derate_rear;
    for (j = 0; j < 4; j++) {
      float f;
      float f1;
      float f4;
      varargin_1[7 * j + 1] = b;
      varargin_1[7 * j + 2] = c_b;
      varargin_1[7 * j + 3] = e_b;
      varargin_1[7 * j + 4] = g_b;
      varargin_1[7 * j + 5] = i_b;
      varargin_1[7 * j + 6] = k_b;
      f = varargin_1[7 * j];
      for (b_i = 0; b_i < 6; b_i++) {
        float f2;
        f2 = varargin_1[(b_i + 7 * j) + 1];
        if (f > f2) {
          f = f2;
        }
      }
      f1 = p->MAX_TO_ABS_PO * f;
      f4 = out;
      minval[j] = out;
      if (out > f1) {
        f4 = f1;
        minval[j] = f1;
      }
      y->TO_BL_PO[j] = f4;
    }
    if (y->VCU_MODE == 0.0F) {
      y->TORQUE_LIM_NEG[0] = 0.0F;
      y->TORQUE_LIM_POS[0] = minval[0];
      y->SPEED_OUT[0] = p->MAX_ABS_WM;
      y->TORQUE_OUT[0] = minval[0];
      y->TORQUE_LIM_NEG[1] = 0.0F;
      y->TORQUE_LIM_POS[1] = minval[1];
      y->SPEED_OUT[1] = p->MAX_ABS_WM;
      y->TORQUE_OUT[1] = minval[1];
      y->TORQUE_LIM_NEG[2] = 0.0F;
      y->TORQUE_LIM_POS[2] = minval[2];
      y->SPEED_OUT[2] = p->MAX_ABS_WM;
      y->TORQUE_OUT[2] = minval[2];
      y->TORQUE_LIM_NEG[3] = 0.0F;
      y->TORQUE_LIM_POS[3] = minval[3];
      y->SPEED_OUT[3] = p->MAX_ABS_WM;
      y->TORQUE_OUT[3] = minval[3];
    } else if (y->VCU_MODE == 1.0F) {
      float WW_snipped[4];
      float fv1[4];
      float f3;
      WW_snipped[0] = fmaxf(fminf(y->WW[0], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[1] = fmaxf(fminf(y->WW[1], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[2] = fmaxf(fminf(y->WW[2], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[3] = fmaxf(fminf(y->WW[3], p->AC_brkpt_ub), p->AC_brkpt_lb);
      b_interp1(p->AC_speed_brkpt, p->AC_speed_table, WW_snipped, fv1);
      f3 = fv1[0] * p->gr;
      y->AC_MW[0] = f3;
      y->TORQUE_LIM_NEG[0] = 0.0F;
      y->TORQUE_LIM_POS[0] = minval[0];
      y->SPEED_OUT[0] = f3;
      y->TORQUE_OUT[0] = minval[0];
      f3 = fv1[1] * p->gr;
      y->AC_MW[1] = f3;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      y->TORQUE_LIM_POS[1] = minval[1];
      y->SPEED_OUT[1] = f3;
      y->TORQUE_OUT[1] = minval[1];
      f3 = fv1[2] * p->gr;
      y->AC_MW[2] = f3;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      y->TORQUE_LIM_POS[2] = minval[2];
      y->SPEED_OUT[2] = f3;
      y->TORQUE_OUT[2] = minval[2];
      f3 = fv1[3] * p->gr;
      y->AC_MW[3] = f3;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      y->TORQUE_LIM_POS[3] = minval[3];
      y->SPEED_OUT[3] = f3;
      y->TORQUE_OUT[3] = minval[3];
    } else if (y->VCU_MODE == 2.0F) {
      float SK_TO_DES_idx_0;
      float SK_TO_DES_idx_1;
      float SK_TO_DES_idx_2;
      float SK_TO_DES_idx_3;
      float c_LR;
      float control_force;
      float d_ex;
      float f9;
      float i_varargin_1_tmp;
      float j_varargin_1_tmp;
      float k_varargin_1_tmp;
      float l_varargin_1_tmp;
      int d_k;
      int i1;
      bool e_out;
      bool exitg1;
      b_p[0] = p->SK_ST_ZERO_TV;
      b_p[1] = p->SK_ST_FULL_TV;
      fv[0] = 0.0F;
      fv[1] = 1.0F;
      control_force = interp1(
          b_p, fv,
          fmaxf(fminf(fabsf(x->ST_RAW), p->SK_ST_FULL_TV), p->SK_ST_ZERO_TV));
      if (x->ST_RAW < 0.0F) {
        i1 = -1;
      } else {
        i1 = (x->ST_RAW > 0.0F);
      }
      c_LR = (1.0F - control_force) * 0.5F +
             control_force *
                 fmaxf(fminf(p->SK_LR_split_des +
                                 ((float)i1 * p->SK_YAW_des - (-x->AV_RAW[2])) *
                                     y->SK_LR_gain,
                             0.65F),
                       0.35F);
      i_varargin_1_tmp = y->SK_FR_split * c_LR;
      j_varargin_1_tmp = y->SK_FR_split * (1.0F - c_LR);
      k_varargin_1_tmp = (1.0F - y->SK_FR_split) * c_LR;
      l_varargin_1_tmp = (1.0F - y->SK_FR_split) * (1.0F - c_LR);
      d_ex = i_varargin_1_tmp;
      if (i_varargin_1_tmp < j_varargin_1_tmp) {
        d_ex = j_varargin_1_tmp;
      }
      if (d_ex < k_varargin_1_tmp) {
        d_ex = k_varargin_1_tmp;
      }
      if (d_ex < l_varargin_1_tmp) {
        d_ex = l_varargin_1_tmp;
      }
      SK_TO_DES_idx_0 = i_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_1 = j_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_2 = k_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_3 = l_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      e_out = false;
      d_k = 0;
      exitg1 = false;
      while ((!exitg1) && (d_k < 4)) {
        if (minval[d_k] == 0.0F) {
          e_out = true;
          exitg1 = true;
        } else {
          d_k++;
        }
      }
      if (e_out) {
        y->SK_TO[0] = 0.0F;
        y->SK_TO[1] = 0.0F;
        y->SK_TO[2] = 0.0F;
        y->SK_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float c_best_scale;
        float c_scale;
        float d_value;
        c_best_scale = 0.0F;
        if (SK_TO_DES_idx_0 != 0.0F) {
          int j_k;
          bool k_out;
          c_scale = minval[0] / SK_TO_DES_idx_0;
          TO_scaled[0] = SK_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * c_scale;
          k_out = true;
          j_k = 0;
          exitg1 = false;
          while ((!exitg1) && (j_k < 4)) {
            if (TO_scaled[j_k] > minval[j_k]) {
              k_out = false;
              exitg1 = true;
            } else {
              j_k++;
            }
          }
          if (k_out) {
            c_best_scale = fmaxf(0.0F, c_scale);
          }
        }
        if (SK_TO_DES_idx_1 != 0.0F) {
          int m_k;
          bool n_out;
          c_scale = minval[1] / SK_TO_DES_idx_1;
          TO_scaled[0] = SK_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * c_scale;
          n_out = true;
          m_k = 0;
          exitg1 = false;
          while ((!exitg1) && (m_k < 4)) {
            if (TO_scaled[m_k] > minval[m_k]) {
              n_out = false;
              exitg1 = true;
            } else {
              m_k++;
            }
          }
          if (n_out) {
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        if (SK_TO_DES_idx_2 != 0.0F) {
          int o_k;
          bool p_out;
          c_scale = minval[2] / SK_TO_DES_idx_2;
          TO_scaled[0] = SK_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * c_scale;
          p_out = true;
          o_k = 0;
          exitg1 = false;
          while ((!exitg1) && (o_k < 4)) {
            if (TO_scaled[o_k] > minval[o_k]) {
              p_out = false;
              exitg1 = true;
            } else {
              o_k++;
            }
          }
          if (p_out) {
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        if (SK_TO_DES_idx_3 != 0.0F) {
          int p_k;
          bool q_out;
          c_scale = minval[3] / SK_TO_DES_idx_3;
          TO_scaled[0] = SK_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * c_scale;
          q_out = true;
          p_k = 0;
          exitg1 = false;
          while ((!exitg1) && (p_k < 4)) {
            if (TO_scaled[p_k] > minval[p_k]) {
              q_out = false;
              exitg1 = true;
            } else {
              p_k++;
            }
          }
          if (q_out) {
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        d_value = fminf(c_best_scale, 1.0F);
        y->SK_TO[0] = SK_TO_DES_idx_0 * d_value;
        y->SK_TO[1] = SK_TO_DES_idx_1 * d_value;
        y->SK_TO[2] = SK_TO_DES_idx_2 * d_value;
        y->SK_TO[3] = SK_TO_DES_idx_3 * d_value;
      }
      y->TORQUE_LIM_NEG[0] = 0.0F;
      f9 = y->SK_TO[0];
      y->TORQUE_LIM_POS[0] = f9;
      y->SPEED_OUT[0] = p->MAX_ABS_WM;
      y->TORQUE_OUT[0] = f9;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      f9 = y->SK_TO[1];
      y->TORQUE_LIM_POS[1] = f9;
      y->SPEED_OUT[1] = p->MAX_ABS_WM;
      y->TORQUE_OUT[1] = f9;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      f9 = y->SK_TO[2];
      y->TORQUE_LIM_POS[2] = f9;
      y->SPEED_OUT[2] = p->MAX_ABS_WM;
      y->TORQUE_OUT[2] = f9;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      f9 = y->SK_TO[3];
      y->TORQUE_LIM_POS[3] = f9;
      y->SPEED_OUT[3] = p->MAX_ABS_WM;
      y->TORQUE_OUT[3] = f9;
    } else if (y->VCU_MODE == 3.0F) {
      float AX_TO_DES_idx_0;
      float AX_TO_DES_idx_1;
      float AX_TO_DES_idx_2;
      float AX_TO_DES_idx_3;
      float LR;
      float ST_lookup_yaw_tmp;
      float b_ex;
      float b_varargin_1_tmp;
      float d_varargin_1_tmp;
      float f8;
      float f_varargin_1_tmp;
      float varargin_1_tmp;
      int AX_YAW_des_tmp;
      int b_k;
      bool c_out;
      bool exitg1;
      ST_lookup_yaw_tmp = fabsf(x->ST_RAW);
      if (x->ST_RAW < 0.0F) {
        AX_YAW_des_tmp = -1;
      } else {
        AX_YAW_des_tmp = (x->ST_RAW > 0.0F);
      }
      LR = (1.0F - y->AX_LR_control_force) * 0.5F +
           y->AX_LR_control_force *
               fmaxf(
                   fminf(((float)AX_YAW_des_tmp *
                              interp2(p->AX_TV_split_ST_brkpt,
                                      p->AX_TV_split_GS_brkpt,
                                      p->AX_TV_split_table,
                                      fmaxf(fminf(ST_lookup_yaw_tmp,
                                                  p->AX_TV_split_ST_brkpt[26]),
                                            p->AX_TV_split_ST_brkpt[0]),
                                      fmaxf(fminf(x->GS_RAW,
                                                  p->AX_TV_split_GS_brkpt[50]),
                                            p->AX_TV_split_GS_brkpt[0])) /
                              2.0F * p->AX_LR_split_max +
                          0.5F) +
                             ((float)AX_YAW_des_tmp *
                                  interp2(
                                      p->AX_TV_yaw_ST_brkpt,
                                      p->AX_TV_yaw_GS_brkpt, p->AX_TV_yaw_table,
                                      fmaxf(fminf(ST_lookup_yaw_tmp,
                                                  p->AX_TV_yaw_ST_brkpt[26]),
                                            p->AX_TV_yaw_ST_brkpt[0]),
                                      fmaxf(fminf(x->GS_RAW,
                                                  p->AX_TV_yaw_GS_brkpt[50]),
                                            p->AX_TV_yaw_GS_brkpt[0])) -
                              (-x->AV_RAW[2])) *
                                 p->AX_LR_gain,
                         0.65F),
                   0.35F);
      varargin_1_tmp = y->AX_FR_split * LR;
      b_varargin_1_tmp = y->AX_FR_split * (1.0F - LR);
      d_varargin_1_tmp = (1.0F - y->AX_FR_split) * LR;
      f_varargin_1_tmp = (1.0F - y->AX_FR_split) * (1.0F - LR);
      b_ex = varargin_1_tmp;
      if (varargin_1_tmp < b_varargin_1_tmp) {
        b_ex = b_varargin_1_tmp;
      }
      if (b_ex < d_varargin_1_tmp) {
        b_ex = d_varargin_1_tmp;
      }
      if (b_ex < f_varargin_1_tmp) {
        b_ex = f_varargin_1_tmp;
      }
      AX_TO_DES_idx_0 = varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_1 = b_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_2 = d_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_3 = f_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      c_out = false;
      b_k = 0;
      exitg1 = false;
      while ((!exitg1) && (b_k < 4)) {
        if (minval[b_k] == 0.0F) {
          c_out = true;
          exitg1 = true;
        } else {
          b_k++;
        }
      }
      if (c_out) {
        y->AX_TO[0] = 0.0F;
        y->AX_TO[1] = 0.0F;
        y->AX_TO[2] = 0.0F;
        y->AX_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float b_value;
        float best_scale;
        float scale;
        best_scale = 0.0F;
        if (AX_TO_DES_idx_0 != 0.0F) {
          int e_k;
          bool f_out;
          scale = minval[0] / AX_TO_DES_idx_0;
          TO_scaled[0] = AX_TO_DES_idx_0 * scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * scale;
          f_out = true;
          e_k = 0;
          exitg1 = false;
          while ((!exitg1) && (e_k < 4)) {
            if (TO_scaled[e_k] > minval[e_k]) {
              f_out = false;
              exitg1 = true;
            } else {
              e_k++;
            }
          }
          if (f_out) {
            best_scale = fmaxf(0.0F, scale);
          }
        }
        if (AX_TO_DES_idx_1 != 0.0F) {
          int g_k;
          bool h_out;
          scale = minval[1] / AX_TO_DES_idx_1;
          TO_scaled[0] = AX_TO_DES_idx_0 * scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * scale;
          h_out = true;
          g_k = 0;
          exitg1 = false;
          while ((!exitg1) && (g_k < 4)) {
            if (TO_scaled[g_k] > minval[g_k]) {
              h_out = false;
              exitg1 = true;
            } else {
              g_k++;
            }
          }
          if (h_out) {
            best_scale = fmaxf(best_scale, scale);
          }
        }
        if (AX_TO_DES_idx_2 != 0.0F) {
          int i_k;
          bool j_out;
          scale = minval[2] / AX_TO_DES_idx_2;
          TO_scaled[0] = AX_TO_DES_idx_0 * scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * scale;
          j_out = true;
          i_k = 0;
          exitg1 = false;
          while ((!exitg1) && (i_k < 4)) {
            if (TO_scaled[i_k] > minval[i_k]) {
              j_out = false;
              exitg1 = true;
            } else {
              i_k++;
            }
          }
          if (j_out) {
            best_scale = fmaxf(best_scale, scale);
          }
        }
        if (AX_TO_DES_idx_3 != 0.0F) {
          int l_k;
          bool m_out;
          scale = minval[3] / AX_TO_DES_idx_3;
          TO_scaled[0] = AX_TO_DES_idx_0 * scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * scale;
          m_out = true;
          l_k = 0;
          exitg1 = false;
          while ((!exitg1) && (l_k < 4)) {
            if (TO_scaled[l_k] > minval[l_k]) {
              m_out = false;
              exitg1 = true;
            } else {
              l_k++;
            }
          }
          if (m_out) {
            best_scale = fmaxf(best_scale, scale);
          }
        }
        b_value = fminf(best_scale, 1.0F);
        y->AX_TO[0] = AX_TO_DES_idx_0 * b_value;
        y->AX_TO[1] = AX_TO_DES_idx_1 * b_value;
        y->AX_TO[2] = AX_TO_DES_idx_2 * b_value;
        y->AX_TO[3] = AX_TO_DES_idx_3 * b_value;
      }
      y->TORQUE_LIM_NEG[0] = 0.0F;
      f8 = y->AX_TO[0];
      y->TORQUE_LIM_POS[0] = f8;
      y->SPEED_OUT[0] = p->MAX_ABS_WM;
      y->TORQUE_OUT[0] = f8;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      f8 = y->AX_TO[1];
      y->TORQUE_LIM_POS[1] = f8;
      y->SPEED_OUT[1] = p->MAX_ABS_WM;
      y->TORQUE_OUT[1] = f8;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      f8 = y->AX_TO[2];
      y->TORQUE_LIM_POS[2] = f8;
      y->SPEED_OUT[2] = p->MAX_ABS_WM;
      y->TORQUE_OUT[2] = f8;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      f8 = y->AX_TO[3];
      y->TORQUE_LIM_POS[3] = f8;
      y->SPEED_OUT[3] = p->MAX_ABS_WM;
      y->TORQUE_OUT[3] = f8;
    } else {
      float TS_TO_des_idx_0;
      float TS_TO_des_idx_1;
      float TS_TO_des_idx_2;
      float TS_TO_des_idx_3;
      float b_LR;
      float c_ex;
      float c_varargin_1_tmp;
      float e_varargin_1_tmp;
      float g_varargin_1_tmp;
      float h_varargin_1_tmp;
      int c_k;
      bool d_out;
      bool exitg1;
      b_p[0] = -p->TS_LR_max_ST;
      b_p[1] = p->TS_LR_max_ST;
      fv[0] = 0.5F - y->TS_LR_split;
      fv[1] = y->TS_LR_split + 0.5F;
      b_LR = interp1(
          b_p, fv, fmaxf(fminf(x->ST_RAW, p->TS_LR_max_ST), -p->TS_LR_max_ST));
      c_varargin_1_tmp = y->TS_FR_split * b_LR;
      e_varargin_1_tmp = y->TS_FR_split * (1.0F - b_LR);
      g_varargin_1_tmp = (1.0F - y->TS_FR_split) * b_LR;
      h_varargin_1_tmp = (1.0F - y->TS_FR_split) * (1.0F - b_LR);
      c_ex = c_varargin_1_tmp;
      if (c_varargin_1_tmp < e_varargin_1_tmp) {
        c_ex = e_varargin_1_tmp;
      }
      if (c_ex < g_varargin_1_tmp) {
        c_ex = g_varargin_1_tmp;
      }
      if (c_ex < h_varargin_1_tmp) {
        c_ex = h_varargin_1_tmp;
      }
      TS_TO_des_idx_0 = c_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_1 = e_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_2 = g_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_3 = h_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      d_out = false;
      c_k = 0;
      exitg1 = false;
      while ((!exitg1) && (c_k < 4)) {
        if (minval[c_k] == 0.0F) {
          d_out = true;
          exitg1 = true;
        } else {
          c_k++;
        }
      }
      if (d_out) {
        y->TS_TO[0] = 0.0F;
        y->TS_TO[1] = 0.0F;
        y->TS_TO[2] = 0.0F;
        y->TS_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float b_best_scale;
        float b_scale;
        float c_value;
        b_best_scale = 0.0F;
        if (TS_TO_des_idx_0 != 0.0F) {
          int f_k;
          bool g_out;
          b_scale = minval[0] / TS_TO_des_idx_0;
          TO_scaled[0] = TS_TO_des_idx_0 * b_scale;
          TO_scaled[1] = TS_TO_des_idx_1 * b_scale;
          TO_scaled[2] = TS_TO_des_idx_2 * b_scale;
          TO_scaled[3] = TS_TO_des_idx_3 * b_scale;
          g_out = true;
          f_k = 0;
          exitg1 = false;
          while ((!exitg1) && (f_k < 4)) {
            if (TO_scaled[f_k] > minval[f_k]) {
              g_out = false;
              exitg1 = true;
            } else {
              f_k++;
            }
          }
          if (g_out) {
            b_best_scale = fmaxf(0.0F, b_scale);
          }
        }
        if (TS_TO_des_idx_1 != 0.0F) {
          int h_k;
          bool i_out;
          b_scale = minval[1] / TS_TO_des_idx_1;
          TO_scaled[0] = TS_TO_des_idx_0 * b_scale;
          TO_scaled[1] = TS_TO_des_idx_1 * b_scale;
          TO_scaled[2] = TS_TO_des_idx_2 * b_scale;
          TO_scaled[3] = TS_TO_des_idx_3 * b_scale;
          i_out = true;
          h_k = 0;
          exitg1 = false;
          while ((!exitg1) && (h_k < 4)) {
            if (TO_scaled[h_k] > minval[h_k]) {
              i_out = false;
              exitg1 = true;
            } else {
              h_k++;
            }
          }
          if (i_out) {
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        if (TS_TO_des_idx_2 != 0.0F) {
          int k_k;
          bool l_out;
          b_scale = minval[2] / TS_TO_des_idx_2;
          TO_scaled[0] = TS_TO_des_idx_0 * b_scale;
          TO_scaled[1] = TS_TO_des_idx_1 * b_scale;
          TO_scaled[2] = TS_TO_des_idx_2 * b_scale;
          TO_scaled[3] = TS_TO_des_idx_3 * b_scale;
          l_out = true;
          k_k = 0;
          exitg1 = false;
          while ((!exitg1) && (k_k < 4)) {
            if (TO_scaled[k_k] > minval[k_k]) {
              l_out = false;
              exitg1 = true;
            } else {
              k_k++;
            }
          }
          if (l_out) {
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        if (TS_TO_des_idx_3 != 0.0F) {
          int n_k;
          bool o_out;
          b_scale = minval[3] / TS_TO_des_idx_3;
          TO_scaled[0] = TS_TO_des_idx_0 * b_scale;
          TO_scaled[1] = TS_TO_des_idx_1 * b_scale;
          TO_scaled[2] = TS_TO_des_idx_2 * b_scale;
          TO_scaled[3] = TS_TO_des_idx_3 * b_scale;
          o_out = true;
          n_k = 0;
          exitg1 = false;
          while ((!exitg1) && (n_k < 4)) {
            if (TO_scaled[n_k] > minval[n_k]) {
              o_out = false;
              exitg1 = true;
            } else {
              n_k++;
            }
          }
          if (o_out) {
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        c_value = fminf(b_best_scale, 1.0F);
        y->TS_TO[0] = TS_TO_des_idx_0 * c_value;
        y->TS_TO[1] = TS_TO_des_idx_1 * c_value;
        y->TS_TO[2] = TS_TO_des_idx_2 * c_value;
        y->TS_TO[3] = TS_TO_des_idx_3 * c_value;
      }
    }
  } else if (y->TH < 0.0F) {
    float varargin_1[28];
    float TO_ET_RG[4];
    float TO_ET_RG_tmp;
    float b_TO_ET_RG_tmp;
    float b_b;
    float b_out;
    float c_idx_1;
    float c_idx_2;
    float c_idx_3;
    float d_b;
    float ex;
    float ex_tmp;
    float f_b;
    float h_b;
    float j_b;
    float l_b;
    float m;
    float m_b;
    m = fmaxf(y->RG_FR_split, 1.0F - y->RG_FR_split);
    b_out = y->TH_RG * p->MAX_TO_ABS_RG;
    TO_ET_RG_tmp = b_out * (y->RG_FR_split / m);
    TO_ET_RG[0] = TO_ET_RG_tmp;
    TO_ET_RG[1] = TO_ET_RG_tmp;
    b_TO_ET_RG_tmp = b_out * ((1.0F - y->RG_FR_split) / m);
    TO_ET_RG[2] = b_TO_ET_RG_tmp;
    TO_ET_RG[3] = b_TO_ET_RG_tmp;
    c_idx_1 = y->WW[1] * p->r;
    c_idx_2 = y->WW[2] * p->r;
    c_idx_3 = y->WW[3] * p->r;
    ex_tmp = y->WW[0] * p->r;
    ex = ex_tmp;
    if (ex_tmp > c_idx_1) {
      ex = c_idx_1;
    }
    if (ex > c_idx_2) {
      ex = c_idx_2;
    }
    if (ex > c_idx_3) {
      ex = c_idx_3;
    }
    b_p[0] = p->GS_RG_derating_full;
    b_p[1] = p->GS_RG_derating_zero;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    b_b = interp1(
        b_p, fv,
        fmaxf(fminf(ex, p->GS_RG_derating_zero), p->GS_RG_derating_full));
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
    b_p[0] = p->VB_RG_derating_full_T;
    b_p[1] = p->VB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    l_b = interp1(b_p, fv,
                  fmaxf(fminf(x->VB_RAW, p->VB_RG_derating_zero_T),
                        p->VB_RG_derating_full_T));
    b_p[0] = p->IB_RG_derating_full_T;
    b_p[1] = p->IB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    m_b = interp1(b_p, fv,
                  fmaxf(fminf(y->IB_AVG, p->IB_RG_derating_zero_T),
                        p->IB_RG_derating_full_T));
    for (b_j = 0; b_j < 4; b_j++) {
      float f5;
      varargin_1[7 * b_j] = d_b;
      varargin_1[7 * b_j + 1] = f_b;
      varargin_1[7 * b_j + 2] = h_b;
      varargin_1[7 * b_j + 3] = j_b;
      varargin_1[7 * b_j + 4] = l_b;
      varargin_1[7 * b_j + 5] = m_b;
      varargin_1[7 * b_j + 6] = b_b;
      f5 = d_b;
      for (c_i = 0; c_i < 6; c_i++) {
        float f7;
        f7 = varargin_1[(c_i + 7 * b_j) + 1];
        if (f5 > f7) {
          f5 = f7;
        }
      }
      float f6;
      f6 = fminf(TO_ET_RG[b_j], p->MAX_TO_ABS_RG * f5);
      y->TO_BL_RG[b_j] = -f6;
      y->TORQUE_LIM_NEG[b_j] = -f6;
      y->TORQUE_LIM_POS[b_j] = 0.0F;
      y->TORQUE_OUT[b_j] = -f6;
    }
  } else {
    y->TORQUE_LIM_NEG[0] = 0.0F;
    y->TORQUE_LIM_POS[0] = 0.0F;
    y->SPEED_OUT[0] = 0.0F;
    y->TORQUE_OUT[0] = 0.0F;
    y->TORQUE_LIM_NEG[1] = 0.0F;
    y->TORQUE_LIM_POS[1] = 0.0F;
    y->SPEED_OUT[1] = 0.0F;
    y->TORQUE_OUT[1] = 0.0F;
    y->TORQUE_LIM_NEG[2] = 0.0F;
    y->TORQUE_LIM_POS[2] = 0.0F;
    y->SPEED_OUT[2] = 0.0F;
    y->TORQUE_OUT[2] = 0.0F;
    y->TORQUE_LIM_NEG[3] = 0.0F;
    y->TORQUE_LIM_POS[3] = 0.0F;
    y->SPEED_OUT[3] = 0.0F;
    y->TORQUE_OUT[3] = 0.0F;
  }
}
