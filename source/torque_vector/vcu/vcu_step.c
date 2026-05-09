/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: vcu_step.c
 *
 * MATLAB Coder version            : 24.1
 * C/C++ source code generated on  : 08-May-2026 18:58:44
 */

/* Include Files */
#include "vcu.h"
#include <math.h>

/* Function Declarations */
static void b_interp1(const float varargin_1[3], const float varargin_2[3],
                      const float varargin_3[4], float Vq[4]);

static float interp1(const float varargin_1[2], const float varargin_2[2],
                     float varargin_3);

static float interp2(const float varargin_1[27], const float varargin_2[51],
                     const float varargin_3[1377], float varargin_4,
                     float varargin_5);

/* Function Definitions */
/*
 * Arguments    : const float varargin_1[3]
 *                const float varargin_2[3]
 *                const float varargin_3[4]
 *                float Vq[4]
 * Return Type  : void
 */
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

/*
 * Arguments    : const float varargin_1[2]
 *                const float varargin_2[2]
 *                float varargin_3
 * Return Type  : float
 */
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

/*
 * Arguments    : const float varargin_1[27]
 *                const float varargin_2[51]
 *                const float varargin_3[1377]
 *                float varargin_4
 *                float varargin_5
 * Return Type  : float
 */
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

/*
 * function y = vcu_step(p, x, y)
 *
 * clip and filter results
 *
 * Arguments    : const pVCU_struct *p
 *                const xVCU_struct *x
 *                yVCU_struct *y
 * Return Type  : void
 */
void vcu_step(const pVCU_struct *p, const xVCU_struct *x, yVCU_struct *y)
{
  float b_p[2];
  float fv[2];
  float fv1[2];
  float b_x;
  float value_tmp;
  int b_i;
  int b_j;
  int c_i;
  int i;
  int j;
  int k;
  /*  Function Description */
  /*  vcu_step runs every loop on the TV board */
  /*  */
  /*  Inputs */
  /*    p   vehicle paramater struct. constant */
  /*    x   Raw sensor data struct. filled with data read from CAN */
  /*            in main.c */
  /*    y   Function input and output struct. contains all clipped and */
  /*            filtered variables, variable buffers, and output from */
  /*            this function. */
  /*  Outputs */
  /*    y   modified version of input y */
  /* 'vcu_step:16' y = get_CF(p, x, y); */
  /*  VCU_mode */
  /*  Function Description */
  /*  This function applies truncation and filtering to all signals that need */
  /*  it. In addition, it distributes signals from x to y. */
  /*  */
  /*  Input      :  p - struct of all constant controller parameters */
  /*                x - struct of all raw sensor measurements */
  /*                y - struct of CF processed controller data at time t-1 */
  /*   */
  /*  Return     :  y - struct of CF processed controller data at time t */
  /*  Process raw inputs from x into y */
  /*  throttle */
  /* 'get_CF:17' if x.REGEN_RAW > 0 */
  if (x->REGEN_RAW > 0.0F) {
    /* 'get_CF:18' y.TH = -1 * snip(x.REGEN_RAW, 0, 1); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    y->TH = -fminf(x->REGEN_RAW, 1.0F);
  } else {
    /* 'get_CF:19' else */
    /* 'get_CF:20' y.TH = snip(x.THROT_RAW, 0, 1); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    y->TH = fmaxf(fminf(x->THROT_RAW, 1.0F), 0.0F);
  }
  /*  power throttle */
  /* 'get_CF:23' y.TH_PO = min(max(y.TH, 0), 1); */
  value_tmp = fmaxf(y->TH, 0.0F);
  y->TH_PO = value_tmp;
  /*  regen throttle */
  /* 'get_CF:26' y.TH_RG = abs(min(max(y.TH, -1), 0)); */
  y->TH_RG = fabsf(fminf(y->TH, 0.0F));
  /*  steering angle */
  /* 'get_CF:29' y.ST = x.ST_RAW; */
  y->ST = x->ST_RAW;
  /*  battery voltage */
  /* 'get_CF:32' y.VB = x.VB_RAW; */
  y->VB = x->VB_RAW;
  /*  motor shaft angular velocity */
  /* 'get_CF:35' y.WM = x.WM_RAW; */
  y->WM[0] = x->WM_RAW[0];
  y->WM[1] = x->WM_RAW[1];
  y->WM[2] = x->WM_RAW[2];
  y->WM[3] = x->WM_RAW[3];
  /*  groundspeed */
  /* 'get_CF:38' y.GS = x.GS_RAW; */
  y->GS = x->GS_RAW;
  /*  chasis angular velocity */
  /* 'get_CF:41' y.AV = x.AV_RAW; */
  y->AV[0] = x->AV_RAW[0];
  y->AV[1] = x->AV_RAW[1];
  y->AV[2] = x->AV_RAW[2];
  /* 'get_CF:42' y.AV(3) = -y.AV(3); */
  y->AV[2] = -x->AV_RAW[2];
  /*  battery current */
  /* 'get_CF:45' y.IB = x.IB_RAW; */
  y->IB = x->IB_RAW;
  /*  max motor temp */
  /* 'get_CF:48' y.MT = x.MT_RAW; */
  y->MT = x->MT_RAW;
  /*  max inverter IGBT temp */
  /* 'get_CF:51' y.IGBT_T = x.IGBT_T_RAW; */
  y->IGBT_T = x->IGBT_T_RAW;
  /*  max inverter cold plate temp */
  /* 'get_CF:54' y.INV_T = x.INV_T_RAW; */
  y->INV_T = x->INV_T_RAW;
  /*  motor overload percentage */
  /* 'get_CF:57' y.OV_MOT = x.OV_MOT; */
  /*  inverter overload percentage */
  /* 'get_CF:60' y.OV_INV = x.OV_INV; */
  /*  max battery cell temperature */
  /* 'get_CF:63' y.BT = x.BT_RAW; */
  y->BT = x->BT_RAW;
  /*  motor torque */
  /* 'get_CF:66' y.TO = x.TO_RAW; */
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
  /*  Process Raw Steering Wheel inputs */
  /*  Regen brake FR split */
  /*  y.RG_FR_split = snip(x.RG_FR_split_RAW, 0, 1); */
  /* 'get_CF:71' y.RG_FR_split = interp1([0 100], [0 1], x.RG_FR_split_RAW); */
  fv[0] = 0.0F;
  fv1[0] = 0.0F;
  fv[1] = 100.0F;
  fv1[1] = 1.0F;
  y->RG_FR_split = interp1(fv, fv1, x->RG_FR_split_RAW);
  /*  Skidpad gains */
  /* 'get_CF:75' y.SK_FR_split = interp1([0, 100], [0, 1], x.SK_FR_split_RAW);
   */
  fv[0] = 0.0F;
  fv1[0] = 0.0F;
  fv[1] = 100.0F;
  fv1[1] = 1.0F;
  y->SK_FR_split = interp1(fv, fv1, x->SK_FR_split_RAW);
  /* 'get_CF:76' y.SK_LR_gain = interp1([0, 100], [p.SK_LR_gain_lb,
   * p.SK_LR_gain_ub], x.SK_LR_gain_RAW); */
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->SK_LR_gain_lb;
  b_p[1] = p->SK_LR_gain_ub;
  y->SK_LR_gain = interp1(fv, b_p, x->SK_LR_gain_RAW);
  /*  Autocross gains */
  /* 'get_CF:79' y.AX_FR_split = interp1([0, 100], [0, 1], x.AX_FR_split_RAW);
   */
  fv[0] = 0.0F;
  fv1[0] = 0.0F;
  fv[1] = 100.0F;
  fv1[1] = 1.0F;
  y->AX_FR_split = interp1(fv, fv1, x->AX_FR_split_RAW);
  /* 'get_CF:80' y.AX_LR_control_force = interp1([0, 100], [0 1],
   * x.AX_LR_control_force_RAW); */
  fv[0] = 0.0F;
  fv1[0] = 0.0F;
  fv[1] = 100.0F;
  fv1[1] = 1.0F;
  y->AX_LR_control_force = interp1(fv, fv1, x->AX_LR_control_force_RAW);
  /*  Testing/Tuning mode gains */
  /* 'get_CF:83' y.TS_FR_split = interp1([0, 100], [0, 1], x.TS_FR_split_RAW);
   */
  fv[0] = 0.0F;
  fv1[0] = 0.0F;
  fv[1] = 100.0F;
  fv1[1] = 1.0F;
  y->TS_FR_split = interp1(fv, fv1, x->TS_FR_split_RAW);
  /* 'get_CF:84' y.TS_LR_split = interp1([0, 100], [p.TS_LR_split_lb,
   * p.TS_LR_split_ub], x.TS_LR_split_RAW); */
  fv[0] = 0.0F;
  fv[1] = 100.0F;
  b_p[0] = p->TS_LR_split_lb;
  b_p[1] = p->TS_LR_split_ub;
  y->TS_LR_split = interp1(fv, b_p, x->TS_LR_split_RAW);
  /*  Update Buffers */
  /*  Moving average battery current */
  /* 'get_CF:88' y.IB_AVG_buffer = [y.IB_AVG_buffer(2:end), y.IB]; */
  for (i = 0; i < 9; i++) {
    y->IB_AVG_buffer[i] = y->IB_AVG_buffer[i + 1];
  }
  y->IB_AVG_buffer[9] = x->IB_RAW;
  /* 'get_CF:89' y.IB_AVG = mean(y.IB_AVG_buffer); */
  b_x = y->IB_AVG_buffer[0];
  for (k = 0; k < 9; k++) {
    b_x += y->IB_AVG_buffer[k + 1];
  }
  y->IB_AVG = b_x / 10.0F;
  /*  Calculate values */
  /*  battery power draw */
  /* 'get_CF:93' y.PB = y.VB * y.IB; */
  y->PB = x->VB_RAW * x->IB_RAW;
  /*  wheel angualr velocity (rad/s) */
  /* 'get_CF:96' y.WW = y.WM / p.gr; */
  y->WW[0] = x->WM_RAW[0] / p->gr;
  y->WW[1] = x->WM_RAW[1] / p->gr;
  y->WW[2] = x->WM_RAW[2] / p->gr;
  y->WW[3] = x->WM_RAW[3] / p->gr;
  /*  determine VCU mode */
  /* 'vcu_step:19' y = get_VCU_mode(p,x,y); */
  /*  vcu_mode */
  /*  Function Description */
  /*  Stead-state skid-pad controller */
  /*  Inputs */
  /*    p   vehicle paramater struct. constant */
  /*    y   Function input and output struct. contains all clipped and */
  /*            filtered variables, variable buffers, and output from */
  /*            this function. */
  /*     */
  /*  Outputs */
  /*    y   modified version of input y */
  /* 'get_VCU_mode:13' if x.VCU_MODE_REQ == 0 */
  if (x->VCU_MODE_REQ == 0.0F) {
    /*  accel */
    /* 'get_VCU_mode:14' y.VCU_MODE = 1; */
    y->VCU_MODE = 1.0F;
  } else if (x->VCU_MODE_REQ == 1.0F) {
    /* 'get_VCU_mode:15' elseif x.VCU_MODE_REQ == 1 */
    /*  skidpad */
    /* 'get_VCU_mode:16' y.VCU_MODE = 2; */
    y->VCU_MODE = 2.0F;
  } else if (x->VCU_MODE_REQ == 2.0F) {
    /* 'get_VCU_mode:17' elseif x.VCU_MODE_REQ == 2 */
    /*  autocross */
    /* 'get_VCU_mode:18' y.VCU_MODE = 3; */
    y->VCU_MODE = 3.0F;
  } else if (x->VCU_MODE_REQ == 3.0F) {
    /* 'get_VCU_mode:19' elseif x.VCU_MODE_REQ == 3 */
    /*  endurance */
    /* 'get_VCU_mode:20' y.VCU_MODE = 4; */
    y->VCU_MODE = 4.0F;
  } else if (x->VCU_MODE_REQ == 4.0F) {
    /* 'get_VCU_mode:21' elseif x.VCU_MODE_REQ == 4 */
    /*  testing */
    /* 'get_VCU_mode:22' y.VCU_MODE = 5; */
    y->VCU_MODE = 5.0F;
  } else {
    /* 'get_VCU_mode:23' else */
    /* 'get_VCU_mode:24' y.VCU_MODE = 0; */
    y->VCU_MODE = 0.0F;
    /*  Default case for unrecognized VCU_MODE_REQ */
  }
  /*  regen */
  /* 'get_VCU_mode:28' if x.REGEN_EN == 1 */
  if (x->REGEN_EN == 1.0F) {
    /* 'get_VCU_mode:29' y.REGEN_EN = 1; */
    y->REGEN_EN = 1.0F;
  } else {
    /* 'get_VCU_mode:30' else */
    /* 'get_VCU_mode:31' y.REGEN_EN = 0; */
    y->REGEN_EN = 0.0F;
  }
  /*  switch between power and regen depending on throttle */
  /* 'vcu_step:22' if y.TH > 0 */
  if (y->TH > 0.0F) {
    float varargin_1[28];
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
    /*  baseline power torque */
    /*  torque limit after current, power, thermal derating */
    /* 'vcu_step:25' y = get_BL_PO(p, y); */
    /*  max torque allowed by throttle position */
    /*  Function Description */
    /*  calculates baseline torque for forward driving */
    /*  based on throttle position, 80kW derating, and safety derating */
    /*  NO OTHER controller/mode should request more than this torque */
    /*  */
    /*  Inputs */
    /*    p   vehicle paramater struct. constant */
    /*    y   Function input and output struct. contains all clipped and */
    /*            filtered variables, variable buffers, and output from */
    /*            this function. */
    /*  Outputs */
    /*    y   modified version of input y */
    /* 'get_BL_PO:16' TO_ET_PO = y.TH_PO * p.MAX_TO_ABS_PO .* [1 1 1 1]; */
    out = y->TH_PO * p->MAX_TO_ABS_PO;
    /*  80kW rules limit derating on battery power */
    /*  only derating to 50% total torque, F:R derating split can be changed */
    /* 'get_BL_PO:20' PB_snipped = snip(y.PB, p.PB_derating_full_T,
     * p.PB_derating_half_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    PB_snipped =
        fmaxf(fminf(y->PB, p->PB_derating_half_T), p->PB_derating_full_T);
    /* 'get_BL_PO:21' PB_derate_front = interp1([p.PB_derating_full_T,
     * p.PB_derating_half_T], [1,1-p.PB_derating_FR], PB_snipped); */
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    fv[0] = 1.0F;
    fv[1] = 1.0F - p->PB_derating_FR;
    PB_derate_front = interp1(b_p, fv, PB_snipped);
    /* 'get_BL_PO:22' PB_derate_rear = interp1([p.PB_derating_full_T,
     * p.PB_derating_half_T], [1,p.PB_derating_FR], PB_snipped); */
    b_p[0] = p->PB_derating_full_T;
    b_p[1] = p->PB_derating_half_T;
    fv[0] = 1.0F;
    fv[1] = p->PB_derating_FR;
    PB_derate_rear = interp1(b_p, fv, PB_snipped);
    /* 'get_BL_PO:23' PB_derate = [PB_derate_front, PB_derate_front,
     * PB_derate_rear, PB_derate_rear]; */
    /*  Inverter temp safetey derating - derate all motors based on highest
     * inverter temp */
    /* 'get_BL_PO:26' INV_T_snipped = snip(y.INV_T, p.INV_T_derating_full_T,
     * p.INV_T_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:27' INV_T_derate = [1 1 1 1] *
     * interp1([p.INV_T_derating_full_T, p.INV_T_derating_zero_T], [1,0],
     * INV_T_snipped); */
    b_p[0] = p->INV_T_derating_full_T;
    b_p[1] = p->INV_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    b = interp1(b_p, fv,
                fmaxf(fminf(x->INV_T_RAW, p->INV_T_derating_zero_T),
                      p->INV_T_derating_full_T));
    /*  IGBT temp safety derating - derate all motors based on highest IGBT temp
     */
    /* 'get_BL_PO:30' IGBT_T_snipped = snip(y.IGBT_T, p.IGBT_T_derating_full_T,
     * p.IGBT_T_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:31' IGBT_T_derate = [1 1 1 1] *
     * interp1([p.IGBT_T_derating_full_T, p.IGBT_T_derating_zero_T], [1,0],
     * IGBT_T_snipped); */
    b_p[0] = p->IGBT_T_derating_full_T;
    b_p[1] = p->IGBT_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    c_b = interp1(b_p, fv,
                  fmaxf(fminf(x->IGBT_T_RAW, p->IGBT_T_derating_zero_T),
                        p->IGBT_T_derating_full_T));
    /*  Motor temp safetey derating - derate all motors based on highest motor
     * temp */
    /* 'get_BL_PO:34' MT_snipped = snip(y.MT, p.MT_derating_full_T,
     * p.MT_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:35' MT_derate = [1 1 1 1] * interp1([p.MT_derating_full_T,
     * p.MT_derating_zero_T], [1,0], MT_snipped); */
    b_p[0] = p->MT_derating_full_T;
    b_p[1] = p->MT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    e_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->MT_RAW, p->MT_derating_zero_T), p->MT_derating_full_T));
    /*  Battery temp safety derating - derate all motors based on highest cell
     * temp */
    /* 'get_BL_PO:38' BT_snipped = snip(y.BT, p.BT_derating_full_T,
     * p.BT_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:39' BT_derate = [1 1 1 1] * interp1([p.BT_derating_full_T,
     * p.BT_derating_zero_T], [1,0], BT_snipped); */
    b_p[0] = p->BT_derating_full_T;
    b_p[1] = p->BT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    g_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->BT_RAW, p->BT_derating_zero_T), p->BT_derating_full_T));
    /*  Battery undervoltage safety derating */
    /* 'get_BL_PO:42' VB_snipped = snip(y.VB, p.VB_derating_full_T,
     * p.VB_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:43' VB_derate = [1 1 1 1] * interp1([p.VB_derating_full_T,
     * p.VB_derating_zero_T], [1,0], VB_snipped); */
    b_p[0] = p->VB_derating_full_T;
    b_p[1] = p->VB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    i_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->VB_RAW, p->VB_derating_zero_T), p->VB_derating_full_T));
    /*  Battery current safety derating */
    /* 'get_BL_PO:46' IB_snipped = snip(y.IB_AVG, p.IB_derating_full_T,
     * p.IB_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:47' IB_derate = [1 1 1 1] * interp1([p.IB_derating_full_T,
     * p.IB_derating_zero_T], [1,0], IB_snipped); */
    b_p[0] = p->IB_derating_full_T;
    b_p[1] = p->IB_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    k_b = interp1(
        b_p, fv,
        fmaxf(fminf(y->IB_AVG, p->IB_derating_zero_T), p->IB_derating_full_T));
    /*  Overloading */
    /* 'get_BL_PO:50' OV_MOT_snipped = snip(y.OV_MOT, p.OV_MOT_derating_full_T,
     * p.OV_MOT_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:51' OV_MOT_derate = interp1([p.OV_MOT_derating_full_T,
     * p.OV_MOT_derating_zero_T], [1,0], OV_MOT_snipped); */
    /*  Overloading */
    /* 'get_BL_PO:54' OV_INV_snipped = snip(y.OV_INV, p.OV_INV_derating_full_T,
     * p.OV_INV_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_PO:55' OV_INV_derate = interp1([p.OV_INV_derating_full_T,
     * p.OV_INV_derating_zero_T], [1,0], OV_INV_snipped); */
    /*  combine derating, multiply by abs max torque to get maximum torque
     * allowed */
    /* 'get_BL_PO:58' TO_DR_MAX = p.MAX_TO_ABS_PO * min([PB_derate;
     * INV_T_derate; IGBT_T_derate; MT_derate; BT_derate; VB_derate; IB_derate],
     * [], 1); */
    varargin_1[0] = PB_derate_front;
    varargin_1[7] = PB_derate_front;
    varargin_1[14] = PB_derate_rear;
    varargin_1[21] = PB_derate_rear;
    /*  TO_OV_MAX = p.MAX_TO_ABS_PO * min([OV_MOT_derate; OV_INV_derate], [],
     * 1); */
    /*  compute overall maximum torque */
    /*  y.TO_BL_PO = min([TO_ET_PO; TO_DR_MAX; TO_OV_MAX], [], 1); */
    /* 'get_BL_PO:63' y.TO_BL_PO = min([TO_ET_PO; TO_DR_MAX], [], 1); */
    for (b_j = 0; b_j < 4; b_j++) {
      float f;
      float f2;
      float f6;
      varargin_1[7 * b_j + 1] = b;
      varargin_1[7 * b_j + 2] = c_b;
      varargin_1[7 * b_j + 3] = e_b;
      varargin_1[7 * b_j + 4] = g_b;
      varargin_1[7 * b_j + 5] = i_b;
      varargin_1[7 * b_j + 6] = k_b;
      f = varargin_1[7 * b_j];
      for (b_i = 0; b_i < 6; b_i++) {
        float f3;
        f3 = varargin_1[(b_i + 7 * b_j) + 1];
        if (f > f3) {
          f = f3;
        }
      }
      f2 = p->MAX_TO_ABS_PO * f;
      f6 = out;
      y->TORQUE_LIM_POS[b_j] = out;
      if (out > f2) {
        f6 = f2;
        y->TORQUE_LIM_POS[b_j] = f2;
      }
      y->TO_BL_PO[b_j] = f6;
    }
    /* 'vcu_step:27' if y.VCU_MODE == 0 */
    if (y->VCU_MODE == 0.0F) {
      /*  fallback for no sensor data */
      /*  emulate torque control using high speed setpoint */
      /* 'vcu_step:29' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:30' y.TORQUE_LIM_POS = y.TO_BL_PO; */
      /* 'vcu_step:31' y.SPEED_OUT = p.MAX_ABS_WM .* [1 1 1 1]; */
      /* 'vcu_step:32' y.TORQUE_OUT = y.TO_BL_PO; */
      y->TORQUE_LIM_NEG[0] = 0.0F;
      y->SPEED_OUT[0] = p->MAX_ABS_WM;
      y->TORQUE_OUT[0] = y->TORQUE_LIM_POS[0];
      y->TORQUE_LIM_NEG[1] = 0.0F;
      y->SPEED_OUT[1] = p->MAX_ABS_WM;
      y->TORQUE_OUT[1] = y->TORQUE_LIM_POS[1];
      y->TORQUE_LIM_NEG[2] = 0.0F;
      y->SPEED_OUT[2] = p->MAX_ABS_WM;
      y->TORQUE_OUT[2] = y->TORQUE_LIM_POS[2];
      y->TORQUE_LIM_NEG[3] = 0.0F;
      y->SPEED_OUT[3] = p->MAX_ABS_WM;
      y->TORQUE_OUT[3] = y->TORQUE_LIM_POS[3];
      /*  because of no speed control */
    } else if (y->VCU_MODE == 1.0F) {
      float WW_snipped[4];
      float fv2[4];
      float f7;
      /* 'vcu_step:34' elseif y.VCU_MODE == 1 */
      /*  accel event controller */
      /*  Use torque limit from baseline controller */
      /*  speed limit from accel controller */
      /* 'vcu_step:37' y = get_ACCEL(p, y); */
      /*  clip wheelspeed to table */
      /*  Function Description */
      /*  calculates speed for accel */
      /*  torque limit is baseline torque GET_BL_PO */
      /*  speed limit is based on vehicle_speed -> wheelspeed map */
      /*  map is constant non-zero wheelspeed at low vehicle speed, */
      /*    transitions to target slip ratio at higher speeds */
      /*  Inputs */
      /*    p   vehicle paramater struct. constant */
      /*    y   Function input and output struct. contains all clipped and */
      /*            filtered variables, variable buffers, and output from */
      /*            this function. */
      /*  Outputs */
      /*    y   modified version of input y */
      /* 'get_ACCEL:17' WW_snipped = snip(y.WW, p.AC_brkpt_lb, p.AC_brkpt_ub);
       */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      WW_snipped[0] = fmaxf(fminf(y->WW[0], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[1] = fmaxf(fminf(y->WW[1], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[2] = fmaxf(fminf(y->WW[2], p->AC_brkpt_ub), p->AC_brkpt_lb);
      WW_snipped[3] = fmaxf(fminf(y->WW[3], p->AC_brkpt_ub), p->AC_brkpt_lb);
      /*  lookup in table, apply same speed to all wheels */
      /* 'get_ACCEL:20' AC_WW = interp1(p.AC_speed_brkpt, p.AC_speed_table,
       * WW_snipped) .* [1 1 1 1]; */
      /* 'get_ACCEL:21' y.AC_MW = AC_WW .* p.gr; */
      b_interp1(p->AC_speed_brkpt, p->AC_speed_table, WW_snipped, fv2);
      /* 'vcu_step:38' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:39' y.TORQUE_LIM_POS = y.TO_BL_PO; */
      /* 'vcu_step:40' y.SPEED_OUT = y.AC_MW; */
      /* 'vcu_step:41' y.TORQUE_OUT = y.TO_BL_PO; */
      f7 = fv2[0] * p->gr;
      y->AC_MW[0] = f7;
      y->TORQUE_LIM_NEG[0] = 0.0F;
      y->SPEED_OUT[0] = f7;
      y->TORQUE_OUT[0] = y->TORQUE_LIM_POS[0];
      f7 = fv2[1] * p->gr;
      y->AC_MW[1] = f7;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      y->SPEED_OUT[1] = f7;
      y->TORQUE_OUT[1] = y->TORQUE_LIM_POS[1];
      f7 = fv2[2] * p->gr;
      y->AC_MW[2] = f7;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      y->SPEED_OUT[2] = f7;
      y->TORQUE_OUT[2] = y->TORQUE_LIM_POS[2];
      f7 = fv2[3] * p->gr;
      y->AC_MW[3] = f7;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      y->SPEED_OUT[3] = f7;
      y->TORQUE_OUT[3] = y->TORQUE_LIM_POS[3];
      /*  because of no speed control */
    } else if (y->VCU_MODE == 2.0F) {
      float SK_TO_DES_idx_0;
      float SK_TO_DES_idx_1;
      float SK_TO_DES_idx_2;
      float SK_TO_DES_idx_3;
      float b_LR;
      float c_ex;
      float control_force;
      float e_varargin_1_tmp;
      float f9;
      float f_varargin_1_tmp;
      float g_varargin_1_tmp;
      float h_varargin_1_tmp;
      int c_k;
      int i1;
      bool d_out;
      bool exitg1;
      /* 'vcu_step:43' elseif y.VCU_MODE == 2 */
      /*  skidpad event controller */
      /* 'vcu_step:44' y = get_SKID(p, y); */
      /*  calculate control force multiplier from steering angle */
      /*  at low steering angles, we don't want any TV */
      /*  Function Description */
      /*  Stead-state skid-pad controller */
      /*  Inputs */
      /*    p   vehicle paramater struct. constant */
      /*    y   Function input and output struct. contains all clipped and */
      /*            filtered variables, variable buffers, and output from */
      /*            this function. */
      /*  Outputs */
      /*    y   modified version of input y */
      /* 'get_SKID:15' ST_clipped = snip(abs(y.ST), p.SK_ST_ZERO_TV,
       * p.SK_ST_FULL_TV); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /* 'get_SKID:16' control_force = interp1([p.SK_ST_ZERO_TV,
       * p.SK_ST_FULL_TV], [0,1], ST_clipped); */
      b_p[0] = p->SK_ST_ZERO_TV;
      b_p[1] = p->SK_ST_FULL_TV;
      fv[0] = 0.0F;
      fv[1] = 1.0F;
      control_force = interp1(
          b_p, fv,
          fmaxf(fminf(fabsf(x->ST_RAW), p->SK_ST_FULL_TV), p->SK_ST_ZERO_TV));
      /*  calculate yaw rate error; positive = slower yaw than desired */
      /* 'get_SKID:19' yaw = y.AV(3); */
      /* 'get_SKID:20' err = sign(y.ST) * p.SK_YAW_des - yaw; */
      /*  proportional control on LR split based on error */
      /*  multiply yaw rate error by gain and control force */
      /* 'get_SKID:24' LR_split_raw = p.SK_LR_split_des + err * y.SK_LR_gain; */
      /* 'get_SKID:25' LR_split_snipped = snip(LR_split_raw, p.SK_LR_gain_lb,
       * p.SK_LR_gain_lb); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /*  limit split to reasonable level */
      /* 'get_SKID:26' LR_split = (1 - control_force) * 0.5 + (control_force) *
       * LR_split_snipped; */
      if (x->ST_RAW < 0.0F) {
        i1 = -1;
      } else {
        i1 = (x->ST_RAW > 0.0F);
      }
      b_LR = (1.0F - control_force) * 0.5F +
             control_force *
                 fmaxf(fminf(p->SK_LR_split_des +
                                 ((float)i1 * p->SK_YAW_des - (-x->AV_RAW[2])) *
                                     y->SK_LR_gain,
                             p->SK_LR_gain_lb),
                       p->SK_LR_gain_lb);
      /*  convert FR, LR split to torques */
      /* 'get_SKID:30' SK_TO_DES = split2torque(y.SK_FR_split, LR_split) .*
       * y.TH_PO .* p.MAX_TO_ABS_PO; */
      /*  Function Description */
      /*  This function computes the four individual torques given a Front:Rear
       */
      /*  torque split and a Left:Right torque split */
      /*  */
      /*  Input      :  FR - front:rear torque split; 1 = 100% front, 0 = 100%
       * rear */
      /*                LR - left:right torque split; 1 = 100% left, 0 = 100%
       * right */
      /*   */
      /*  Return     :  torques - individual torques, highest torque wheel is 1,
       */
      /*                    all others are equal or lower */
      /*                Size: [1 4] Order: [FL FR RL RR] */
      /* 'split2torque:14' m = max([FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)]); */
      e_varargin_1_tmp = y->SK_FR_split * b_LR;
      f_varargin_1_tmp = y->SK_FR_split * (1.0F - b_LR);
      g_varargin_1_tmp = (1.0F - y->SK_FR_split) * b_LR;
      h_varargin_1_tmp = (1.0F - y->SK_FR_split) * (1.0F - b_LR);
      c_ex = e_varargin_1_tmp;
      if (e_varargin_1_tmp < f_varargin_1_tmp) {
        c_ex = f_varargin_1_tmp;
      }
      if (c_ex < g_varargin_1_tmp) {
        c_ex = g_varargin_1_tmp;
      }
      if (c_ex < h_varargin_1_tmp) {
        c_ex = h_varargin_1_tmp;
      }
      /* 'split2torque:15' torques = [FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)] ./ m; */
      SK_TO_DES_idx_0 = e_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_1 = f_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_2 = g_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      SK_TO_DES_idx_3 = h_varargin_1_tmp / c_ex * value_tmp * p->MAX_TO_ABS_PO;
      /*  make sure torques do not violate rules or safety derating */
      /* 'get_SKID:33' y.SK_TO = rescale_torque(SK_TO_DES, y.TO_BL_PO); */
      /*  if any torque limit is 0, return 0 torque */
      /*  Function Description */
      /*  Takes a set of torques and torque limits, rescales given torques */
      /*  (maintaining relative torque split) so that no torque exceeds torque
       * limit */
      /*  Inputs */
      /*    TO_in      Set of torques to be rescaled to be below limits Units:
       * [Nm] Order: FL FR RL RR] */
      /*    TO_lim     Set of torque limits Units: [Nm] Order: FL FR RL RR] */
      /*  */
      /*  Outputs */
      /*    TO_maxed   TO_split rescaled by a constant such that no torque in
       * TO_split */
      /*        exceeds its corresponding torque in TO_lim, and at least one */
      /*        torque in TO_split is equal to its corresponding TO_lim */
      /* 'rescale_torque:16' if any(TO_max == 0) */
      d_out = false;
      c_k = 0;
      exitg1 = false;
      while ((!exitg1) && (c_k < 4)) {
        if (y->TORQUE_LIM_POS[c_k] == 0.0F) {
          d_out = true;
          exitg1 = true;
        } else {
          c_k++;
        }
      }
      if (d_out) {
        /* 'rescale_torque:17' TO_maxed = [0 0 0 0]; */
        y->SK_TO[0] = 0.0F;
        y->SK_TO[1] = 0.0F;
        y->SK_TO[2] = 0.0F;
        y->SK_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float b_best_scale;
        float b_scale;
        float c_value;
        /*  for each wheel, scale up TO_in so that wheel matches corresponding
         */
        /*  TO_max, then check if any other wheels exceed limit */
        /* 'rescale_torque:23' best_scale = 0; */
        b_best_scale = 0.0F;
        /*  FL */
        /* 'rescale_torque:26' if TO_in(1) ~= 0 */
        if (SK_TO_DES_idx_0 != 0.0F) {
          int h_k;
          bool i_out;
          /* 'rescale_torque:27' scale = TO_max(1) / TO_in(1); */
          b_scale = y->TORQUE_LIM_POS[0] / SK_TO_DES_idx_0;
          /* 'rescale_torque:28' TO_scaled = TO_in * scale; */
          TO_scaled[0] = SK_TO_DES_idx_0 * b_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * b_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * b_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * b_scale;
          /* 'rescale_torque:29' if all(TO_scaled <= TO_max) */
          i_out = true;
          h_k = 0;
          exitg1 = false;
          while ((!exitg1) && (h_k < 4)) {
            if (TO_scaled[h_k] > y->TORQUE_LIM_POS[h_k]) {
              i_out = false;
              exitg1 = true;
            } else {
              h_k++;
            }
          }
          if (i_out) {
            /* 'rescale_torque:30' best_scale = max(best_scale, scale); */
            b_best_scale = fmaxf(0.0F, b_scale);
          }
        }
        /*  FR */
        /* 'rescale_torque:34' if TO_in(2) ~= 0 */
        if (SK_TO_DES_idx_1 != 0.0F) {
          int j_k;
          bool k_out;
          /* 'rescale_torque:35' scale = TO_max(2) / TO_in(2); */
          b_scale = y->TORQUE_LIM_POS[1] / SK_TO_DES_idx_1;
          /* 'rescale_torque:36' TO_scaled = TO_in * scale; */
          TO_scaled[0] = SK_TO_DES_idx_0 * b_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * b_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * b_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * b_scale;
          /* 'rescale_torque:37' if all(TO_scaled <= TO_max) */
          k_out = true;
          j_k = 0;
          exitg1 = false;
          while ((!exitg1) && (j_k < 4)) {
            if (TO_scaled[j_k] > y->TORQUE_LIM_POS[j_k]) {
              k_out = false;
              exitg1 = true;
            } else {
              j_k++;
            }
          }
          if (k_out) {
            /* 'rescale_torque:38' best_scale = max(best_scale, scale); */
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        /*  RL */
        /* 'rescale_torque:42' if TO_in(3) ~= 0 */
        if (SK_TO_DES_idx_2 != 0.0F) {
          int k_k;
          bool l_out;
          /* 'rescale_torque:43' scale = TO_max(3) / TO_in(3); */
          b_scale = y->TORQUE_LIM_POS[2] / SK_TO_DES_idx_2;
          /* 'rescale_torque:44' TO_scaled = TO_in * scale; */
          TO_scaled[0] = SK_TO_DES_idx_0 * b_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * b_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * b_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * b_scale;
          /* 'rescale_torque:45' if all(TO_scaled <= TO_max) */
          l_out = true;
          k_k = 0;
          exitg1 = false;
          while ((!exitg1) && (k_k < 4)) {
            if (TO_scaled[k_k] > y->TORQUE_LIM_POS[k_k]) {
              l_out = false;
              exitg1 = true;
            } else {
              k_k++;
            }
          }
          if (l_out) {
            /* 'rescale_torque:46' best_scale = max(best_scale, scale); */
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        /*  RR */
        /* 'rescale_torque:50' if TO_in(4) ~= 0 */
        if (SK_TO_DES_idx_3 != 0.0F) {
          int m_k;
          bool n_out;
          /* 'rescale_torque:51' scale = TO_max(4) / TO_in(4); */
          b_scale = y->TORQUE_LIM_POS[3] / SK_TO_DES_idx_3;
          /* 'rescale_torque:52' TO_scaled = TO_in * scale; */
          TO_scaled[0] = SK_TO_DES_idx_0 * b_scale;
          TO_scaled[1] = SK_TO_DES_idx_1 * b_scale;
          TO_scaled[2] = SK_TO_DES_idx_2 * b_scale;
          TO_scaled[3] = SK_TO_DES_idx_3 * b_scale;
          /* 'rescale_torque:53' if all(TO_scaled <= TO_max) */
          n_out = true;
          m_k = 0;
          exitg1 = false;
          while ((!exitg1) && (m_k < 4)) {
            if (TO_scaled[m_k] > y->TORQUE_LIM_POS[m_k]) {
              n_out = false;
              exitg1 = true;
            } else {
              m_k++;
            }
          }
          if (n_out) {
            /* 'rescale_torque:54' best_scale = max(best_scale, scale); */
            b_best_scale = fmaxf(b_best_scale, b_scale);
          }
        }
        /*  make sure we don't increase torque */
        /* 'rescale_torque:58' best_scale_snipped = snip(best_scale, 0, 1); */
        /* SNIP code generation compatible version of 'clip()' */
        /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB'
         */
        /* 'snip:4' clipped = max(min(value, UB), LB); */
        c_value = fminf(b_best_scale, 1.0F);
        /*  output */
        /* 'rescale_torque:60' TO_maxed = TO_in * best_scale_snipped; */
        y->SK_TO[0] = SK_TO_DES_idx_0 * c_value;
        y->SK_TO[1] = SK_TO_DES_idx_1 * c_value;
        y->SK_TO[2] = SK_TO_DES_idx_2 * c_value;
        y->SK_TO[3] = SK_TO_DES_idx_3 * c_value;
      }
      /*  y.SK_TO = min(SK_TO_DES, y.TO_BL_PO); */
      /* 'vcu_step:45' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:46' y.TORQUE_LIM_POS = y.SK_TO; */
      /* 'vcu_step:47' y.SPEED_OUT = p.MAX_ABS_WM * [1 1 1 1]; */
      /* 'vcu_step:48' y.TORQUE_OUT = y.SK_TO; */
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
      /*  because of no speed control */
    } else if (y->VCU_MODE == 3.0F) {
      float AX_TO_DES_idx_0;
      float AX_TO_DES_idx_1;
      float AX_TO_DES_idx_2;
      float AX_TO_DES_idx_3;
      float ST_lookup_yaw_tmp;
      float c_LR;
      float d_ex;
      float f10;
      float i_varargin_1_tmp;
      float j_varargin_1_tmp;
      float k_varargin_1_tmp;
      float l_varargin_1_tmp;
      int AX_YAW_des_tmp;
      int e_k;
      bool exitg1;
      bool f_out;
      /* 'vcu_step:50' elseif y.VCU_MODE == 3 */
      /*  auto-x event controller */
      /* 'vcu_step:51' y = get_AUTOX(p, y); */
      /*  snip steering angle to positive and in LUT range, snip groundspeed to
       * LUT range */
      /*  ST_clipped = snip(abs(y.ST), p.AX_TV_yaw_ST_brkpt(1),
       * p.AX_TV_yaw_ST_brkpt(end)); */
      /*  GS_clipped = snip(y.GS, p.AX_TV_yaw_GS_brkpt(1),
       * p.AX_TV_yaw_GS_brkpt(end)); */
      /*  control_force = interp1([p.AX_ST_ZERO_TV, p.AX_ST_FULL_TV], [0,1],
       * ST_clipped);  */
      /*  Function Description */
      /*  Test controller */
      /*  Inputs */
      /*    p   vehicle paramater struct. constant */
      /*    y   Function input and output struct. contains all clipped and */
      /*            filtered variables, variable buffers, and output from */
      /*            this function. */
      /*  Outputs */
      /*    y   modified version of input y */
      /* 'get_AUTOX:17' control_force = y.AX_LR_control_force; */
      /*  calculate yaw rate error; positive = need to turn right faster,
       * negative = need to turn right slower */
      /* 'get_AUTOX:20' yaw = y.AV(3); */
      /* 'get_AUTOX:21' ST_lookup_yaw = snip(abs(y.ST), p.AX_TV_yaw_ST_brkpt(1),
       * p.AX_TV_yaw_ST_brkpt(end)); */
      ST_lookup_yaw_tmp = fabsf(x->ST_RAW);
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /* 'get_AUTOX:22' GS_lookup_yaw = snip(y.GS, p.AX_TV_yaw_GS_brkpt(1),
       * p.AX_TV_yaw_GS_brkpt(end)); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /* 'get_AUTOX:23' AX_YAW_des = sign(y.ST) .* interp2(p.AX_TV_yaw_ST_brkpt,
       * p.AX_TV_yaw_GS_brkpt, p.AX_TV_yaw_table, ST_lookup_yaw, GS_lookup_yaw);
       */
      if (x->ST_RAW < 0.0F) {
        AX_YAW_des_tmp = -1;
      } else {
        AX_YAW_des_tmp = (x->ST_RAW > 0.0F);
      }
      /* 'get_AUTOX:24' err = AX_YAW_des - yaw; */
      /*  calculate desired LR split based on calculated desired yaw */
      /* 'get_AUTOX:27' ST_lookup_split = snip(abs(y.ST),
       * p.AX_TV_split_ST_brkpt(1), p.AX_TV_split_ST_brkpt(end)); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /* 'get_AUTOX:28' GS_lookup_split = snip(abs(y.GS),
       * p.AX_TV_split_GS_brkpt(1), p.AX_TV_split_GS_brkpt(end)); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /* 'get_AUTOX:29' AX_LR_split_des = sign(y.ST) .*
       * interp2(p.AX_TV_split_ST_brkpt, p.AX_TV_split_GS_brkpt,
       * p.AX_TV_split_table, ST_lookup_split, GS_lookup_split); */
      /*  proportional control on LR split based on error */
      /*  multiply yaw rate error by gain and control force */
      /* 'get_AUTOX:33' LR_split_raw = AX_LR_split_des + err * p.AX_LR_gain; */
      /* 'get_AUTOX:34' LR_split_snipped = snip(LR_split_raw, p.AX_LR_split_lb,
       * p.AX_LR_split_lb); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /*  limit split to reasonable level */
      /* 'get_AUTOX:35' LR_split = (1 - control_force) * 0.5 + (control_force) *
       * LR_split_snipped; */
      c_LR =
          (1.0F - y->AX_LR_control_force) * 0.5F +
          y->AX_LR_control_force *
              fmaxf(
                  fminf(
                      (float)AX_YAW_des_tmp *
                              interp2(p->AX_TV_split_ST_brkpt,
                                      p->AX_TV_split_GS_brkpt,
                                      p->AX_TV_split_table,
                                      fmaxf(fminf(ST_lookup_yaw_tmp,
                                                  p->AX_TV_split_ST_brkpt[26]),
                                            p->AX_TV_split_ST_brkpt[0]),
                                      fmaxf(fminf(fabsf(x->GS_RAW),
                                                  p->AX_TV_split_GS_brkpt[50]),
                                            p->AX_TV_split_GS_brkpt[0])) +
                          ((float)AX_YAW_des_tmp *
                               interp2(p->AX_TV_yaw_ST_brkpt,
                                       p->AX_TV_yaw_GS_brkpt,
                                       p->AX_TV_yaw_table,
                                       fmaxf(fminf(ST_lookup_yaw_tmp,
                                                   p->AX_TV_yaw_ST_brkpt[26]),
                                             p->AX_TV_yaw_ST_brkpt[0]),
                                       fmaxf(fminf(x->GS_RAW,
                                                   p->AX_TV_yaw_GS_brkpt[50]),
                                             p->AX_TV_yaw_GS_brkpt[0])) -
                           (-x->AV_RAW[2])) *
                              p->AX_LR_gain,
                      p->AX_LR_split_lb),
                  p->AX_LR_split_lb);
      /*  convert FR, LR split to torques */
      /* 'get_AUTOX:39' AX_TO_DES = split2torque(y.AX_FR_split, LR_split) .*
       * y.TH_PO .* p.MAX_TO_ABS_PO; */
      /*  Function Description */
      /*  This function computes the four individual torques given a Front:Rear
       */
      /*  torque split and a Left:Right torque split */
      /*  */
      /*  Input      :  FR - front:rear torque split; 1 = 100% front, 0 = 100%
       * rear */
      /*                LR - left:right torque split; 1 = 100% left, 0 = 100%
       * right */
      /*   */
      /*  Return     :  torques - individual torques, highest torque wheel is 1,
       */
      /*                    all others are equal or lower */
      /*                Size: [1 4] Order: [FL FR RL RR] */
      /* 'split2torque:14' m = max([FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)]); */
      i_varargin_1_tmp = y->AX_FR_split * c_LR;
      j_varargin_1_tmp = y->AX_FR_split * (1.0F - c_LR);
      k_varargin_1_tmp = (1.0F - y->AX_FR_split) * c_LR;
      l_varargin_1_tmp = (1.0F - y->AX_FR_split) * (1.0F - c_LR);
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
      /* 'split2torque:15' torques = [FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)] ./ m; */
      AX_TO_DES_idx_0 = i_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_1 = j_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_2 = k_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      AX_TO_DES_idx_3 = l_varargin_1_tmp / d_ex * value_tmp * p->MAX_TO_ABS_PO;
      /*  make sure torques do not violate rules or safety derating */
      /* 'get_AUTOX:42' y.AX_TO = rescale_torque(AX_TO_DES, y.TO_BL_PO); */
      /*  if any torque limit is 0, return 0 torque */
      /*  Function Description */
      /*  Takes a set of torques and torque limits, rescales given torques */
      /*  (maintaining relative torque split) so that no torque exceeds torque
       * limit */
      /*  Inputs */
      /*    TO_in      Set of torques to be rescaled to be below limits Units:
       * [Nm] Order: FL FR RL RR] */
      /*    TO_lim     Set of torque limits Units: [Nm] Order: FL FR RL RR] */
      /*  */
      /*  Outputs */
      /*    TO_maxed   TO_split rescaled by a constant such that no torque in
       * TO_split */
      /*        exceeds its corresponding torque in TO_lim, and at least one */
      /*        torque in TO_split is equal to its corresponding TO_lim */
      /* 'rescale_torque:16' if any(TO_max == 0) */
      f_out = false;
      e_k = 0;
      exitg1 = false;
      while ((!exitg1) && (e_k < 4)) {
        if (y->TORQUE_LIM_POS[e_k] == 0.0F) {
          f_out = true;
          exitg1 = true;
        } else {
          e_k++;
        }
      }
      if (f_out) {
        /* 'rescale_torque:17' TO_maxed = [0 0 0 0]; */
        y->AX_TO[0] = 0.0F;
        y->AX_TO[1] = 0.0F;
        y->AX_TO[2] = 0.0F;
        y->AX_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float c_best_scale;
        float c_scale;
        float d_value;
        /*  for each wheel, scale up TO_in so that wheel matches corresponding
         */
        /*  TO_max, then check if any other wheels exceed limit */
        /* 'rescale_torque:23' best_scale = 0; */
        c_best_scale = 0.0F;
        /*  FL */
        /* 'rescale_torque:26' if TO_in(1) ~= 0 */
        if (AX_TO_DES_idx_0 != 0.0F) {
          int l_k;
          bool m_out;
          /* 'rescale_torque:27' scale = TO_max(1) / TO_in(1); */
          c_scale = y->TORQUE_LIM_POS[0] / AX_TO_DES_idx_0;
          /* 'rescale_torque:28' TO_scaled = TO_in * scale; */
          TO_scaled[0] = AX_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * c_scale;
          /* 'rescale_torque:29' if all(TO_scaled <= TO_max) */
          m_out = true;
          l_k = 0;
          exitg1 = false;
          while ((!exitg1) && (l_k < 4)) {
            if (TO_scaled[l_k] > y->TORQUE_LIM_POS[l_k]) {
              m_out = false;
              exitg1 = true;
            } else {
              l_k++;
            }
          }
          if (m_out) {
            /* 'rescale_torque:30' best_scale = max(best_scale, scale); */
            c_best_scale = fmaxf(0.0F, c_scale);
          }
        }
        /*  FR */
        /* 'rescale_torque:34' if TO_in(2) ~= 0 */
        if (AX_TO_DES_idx_1 != 0.0F) {
          int n_k;
          bool o_out;
          /* 'rescale_torque:35' scale = TO_max(2) / TO_in(2); */
          c_scale = y->TORQUE_LIM_POS[1] / AX_TO_DES_idx_1;
          /* 'rescale_torque:36' TO_scaled = TO_in * scale; */
          TO_scaled[0] = AX_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * c_scale;
          /* 'rescale_torque:37' if all(TO_scaled <= TO_max) */
          o_out = true;
          n_k = 0;
          exitg1 = false;
          while ((!exitg1) && (n_k < 4)) {
            if (TO_scaled[n_k] > y->TORQUE_LIM_POS[n_k]) {
              o_out = false;
              exitg1 = true;
            } else {
              n_k++;
            }
          }
          if (o_out) {
            /* 'rescale_torque:38' best_scale = max(best_scale, scale); */
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        /*  RL */
        /* 'rescale_torque:42' if TO_in(3) ~= 0 */
        if (AX_TO_DES_idx_2 != 0.0F) {
          int o_k;
          bool p_out;
          /* 'rescale_torque:43' scale = TO_max(3) / TO_in(3); */
          c_scale = y->TORQUE_LIM_POS[2] / AX_TO_DES_idx_2;
          /* 'rescale_torque:44' TO_scaled = TO_in * scale; */
          TO_scaled[0] = AX_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * c_scale;
          /* 'rescale_torque:45' if all(TO_scaled <= TO_max) */
          p_out = true;
          o_k = 0;
          exitg1 = false;
          while ((!exitg1) && (o_k < 4)) {
            if (TO_scaled[o_k] > y->TORQUE_LIM_POS[o_k]) {
              p_out = false;
              exitg1 = true;
            } else {
              o_k++;
            }
          }
          if (p_out) {
            /* 'rescale_torque:46' best_scale = max(best_scale, scale); */
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        /*  RR */
        /* 'rescale_torque:50' if TO_in(4) ~= 0 */
        if (AX_TO_DES_idx_3 != 0.0F) {
          int p_k;
          bool q_out;
          /* 'rescale_torque:51' scale = TO_max(4) / TO_in(4); */
          c_scale = y->TORQUE_LIM_POS[3] / AX_TO_DES_idx_3;
          /* 'rescale_torque:52' TO_scaled = TO_in * scale; */
          TO_scaled[0] = AX_TO_DES_idx_0 * c_scale;
          TO_scaled[1] = AX_TO_DES_idx_1 * c_scale;
          TO_scaled[2] = AX_TO_DES_idx_2 * c_scale;
          TO_scaled[3] = AX_TO_DES_idx_3 * c_scale;
          /* 'rescale_torque:53' if all(TO_scaled <= TO_max) */
          q_out = true;
          p_k = 0;
          exitg1 = false;
          while ((!exitg1) && (p_k < 4)) {
            if (TO_scaled[p_k] > y->TORQUE_LIM_POS[p_k]) {
              q_out = false;
              exitg1 = true;
            } else {
              p_k++;
            }
          }
          if (q_out) {
            /* 'rescale_torque:54' best_scale = max(best_scale, scale); */
            c_best_scale = fmaxf(c_best_scale, c_scale);
          }
        }
        /*  make sure we don't increase torque */
        /* 'rescale_torque:58' best_scale_snipped = snip(best_scale, 0, 1); */
        /* SNIP code generation compatible version of 'clip()' */
        /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB'
         */
        /* 'snip:4' clipped = max(min(value, UB), LB); */
        d_value = fminf(c_best_scale, 1.0F);
        /*  output */
        /* 'rescale_torque:60' TO_maxed = TO_in * best_scale_snipped; */
        y->AX_TO[0] = AX_TO_DES_idx_0 * d_value;
        y->AX_TO[1] = AX_TO_DES_idx_1 * d_value;
        y->AX_TO[2] = AX_TO_DES_idx_2 * d_value;
        y->AX_TO[3] = AX_TO_DES_idx_3 * d_value;
      }
      /*  y.AX_TO = min(AX_TO_DES, y.TO_BL_PO); */
      /* 'vcu_step:52' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:53' y.TORQUE_LIM_POS = y.AX_TO; */
      /* 'vcu_step:54' y.SPEED_OUT = p.MAX_ABS_WM * [1 1 1 1]; */
      /* 'vcu_step:55' y.TORQUE_OUT = y.AX_TO; */
      y->TORQUE_LIM_NEG[0] = 0.0F;
      f10 = y->AX_TO[0];
      y->TORQUE_LIM_POS[0] = f10;
      y->SPEED_OUT[0] = p->MAX_ABS_WM;
      y->TORQUE_OUT[0] = f10;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      f10 = y->AX_TO[1];
      y->TORQUE_LIM_POS[1] = f10;
      y->SPEED_OUT[1] = p->MAX_ABS_WM;
      y->TORQUE_OUT[1] = f10;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      f10 = y->AX_TO[2];
      y->TORQUE_LIM_POS[2] = f10;
      y->SPEED_OUT[2] = p->MAX_ABS_WM;
      y->TORQUE_OUT[2] = f10;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      f10 = y->AX_TO[3];
      y->TORQUE_LIM_POS[3] = f10;
      y->SPEED_OUT[3] = p->MAX_ABS_WM;
      y->TORQUE_OUT[3] = f10;
      /*  because of no speed control */
    } else if (y->VCU_MODE == 4.0F) {
      /* 'vcu_step:57' elseif y.VCU_MODE == 4 */
      /*  endurance event controller */
      /* 'vcu_step:58' y = get_ENDUR(p, y); */
      /*  Function Description */
      /*  Endurance controller */
      /*  Inputs */
      /*    p   vehicle paramater struct. constant */
      /*    y   Function input and output struct. contains all clipped and */
      /*            filtered variables, variable buffers, and output from */
      /*            this function. */
      /*  Outputs */
      /*    y   modified version of input y */
      /* 'vcu_step:59' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:60' y.TORQUE_LIM_POS = [0 0 0 0]; */
      /* 'vcu_step:61' y.SPEED_OUT = [0 0 0 0]; */
      y->TORQUE_LIM_NEG[0] = 0.0F;
      y->TORQUE_LIM_POS[0] = 0.0F;
      y->SPEED_OUT[0] = 0.0F;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      y->TORQUE_LIM_POS[1] = 0.0F;
      y->SPEED_OUT[1] = 0.0F;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      y->TORQUE_LIM_POS[2] = 0.0F;
      y->SPEED_OUT[2] = 0.0F;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      y->TORQUE_LIM_POS[3] = 0.0F;
      y->SPEED_OUT[3] = 0.0F;
    } else {
      float LR;
      float TS_TO_des_idx_0;
      float TS_TO_des_idx_1;
      float TS_TO_des_idx_2;
      float TS_TO_des_idx_3;
      float b_ex;
      float b_varargin_1_tmp;
      float c_varargin_1_tmp;
      float d_varargin_1_tmp;
      float f8;
      float varargin_1_tmp;
      int b_k;
      bool c_out;
      bool exitg1;
      /* 'vcu_step:63' elseif y.VCU_MODE == 5 */
      /*  testing/tuning controller */
      /* 'vcu_step:64' y = get_TEST(p, y); */
      /*  Function Description */
      /*  Testing controller, driver sets LR and FR split directly */
      /*  Inputs */
      /*    p   vehicle paramater struct. constant */
      /*    y   Function input and output struct. contains all clipped and */
      /*            filtered variables, variable buffers, and output from */
      /*            this function. */
      /*  Outputs */
      /*    y   modified version of input y */
      /* 'get_TEST:12' ST_clipped = snip(y.ST, -p.TS_LR_max_ST, p.TS_LR_max_ST);
       */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      /*  TS_LR_split_clipped = snip(y.TS_LR_split, 0, .25); */
      /* 'get_TEST:14' LR_split_raw = interp1([-p.TS_LR_max_ST, p.TS_LR_max_ST],
       * [.5 - y.TS_LR_split, .5 + y.TS_LR_split], ST_clipped); */
      /* 'get_TEST:15' LR_split = snip(LR_split_raw, p.TS_LR_split_lb,
       * p.TS_LR_split_ub); */
      /* SNIP code generation compatible version of 'clip()' */
      /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
      /* 'snip:4' clipped = max(min(value, UB), LB); */
      b_p[0] = -p->TS_LR_max_ST;
      b_p[1] = p->TS_LR_max_ST;
      fv[0] = 0.5F - y->TS_LR_split;
      fv[1] = y->TS_LR_split + 0.5F;
      LR = fmaxf(fminf(interp1(b_p, fv,
                               fmaxf(fminf(x->ST_RAW, p->TS_LR_max_ST),
                                     -p->TS_LR_max_ST)),
                       p->TS_LR_split_ub),
                 p->TS_LR_split_lb);
      /*  LR_split_raw = 0.5 + y.ST * p.TS_LR_gain; */
      /*  split_diff = snip(y.TS_LR_split, 0, .25); */
      /*  LR_split_clipped = snip(LR_split_raw, 0.5 - split_diff, 0.5 +
       * split_diff); */
      /* 'get_TEST:20' TS_TO_des = split2torque(y.TS_FR_split, LR_split) .*
       * y.TH_PO .* p.MAX_TO_ABS_PO; */
      /*  Function Description */
      /*  This function computes the four individual torques given a Front:Rear
       */
      /*  torque split and a Left:Right torque split */
      /*  */
      /*  Input      :  FR - front:rear torque split; 1 = 100% front, 0 = 100%
       * rear */
      /*                LR - left:right torque split; 1 = 100% left, 0 = 100%
       * right */
      /*   */
      /*  Return     :  torques - individual torques, highest torque wheel is 1,
       */
      /*                    all others are equal or lower */
      /*                Size: [1 4] Order: [FL FR RL RR] */
      /* 'split2torque:14' m = max([FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)]); */
      varargin_1_tmp = y->TS_FR_split * LR;
      b_varargin_1_tmp = y->TS_FR_split * (1.0F - LR);
      c_varargin_1_tmp = (1.0F - y->TS_FR_split) * LR;
      d_varargin_1_tmp = (1.0F - y->TS_FR_split) * (1.0F - LR);
      b_ex = varargin_1_tmp;
      if (varargin_1_tmp < b_varargin_1_tmp) {
        b_ex = b_varargin_1_tmp;
      }
      if (b_ex < c_varargin_1_tmp) {
        b_ex = c_varargin_1_tmp;
      }
      if (b_ex < d_varargin_1_tmp) {
        b_ex = d_varargin_1_tmp;
      }
      /* 'split2torque:15' torques = [FR*LR, FR*(1-LR), (1-FR)*LR,
       * (1-FR)*(1-LR)] ./ m; */
      TS_TO_des_idx_0 = varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_1 = b_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_2 = c_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      TS_TO_des_idx_3 = d_varargin_1_tmp / b_ex * value_tmp * p->MAX_TO_ABS_PO;
      /* 'get_TEST:21' y.TS_TO = rescale_torque(TS_TO_des, y.TO_BL_PO); */
      /*  if any torque limit is 0, return 0 torque */
      /*  Function Description */
      /*  Takes a set of torques and torque limits, rescales given torques */
      /*  (maintaining relative torque split) so that no torque exceeds torque
       * limit */
      /*  Inputs */
      /*    TO_in      Set of torques to be rescaled to be below limits Units:
       * [Nm] Order: FL FR RL RR] */
      /*    TO_lim     Set of torque limits Units: [Nm] Order: FL FR RL RR] */
      /*  */
      /*  Outputs */
      /*    TO_maxed   TO_split rescaled by a constant such that no torque in
       * TO_split */
      /*        exceeds its corresponding torque in TO_lim, and at least one */
      /*        torque in TO_split is equal to its corresponding TO_lim */
      /* 'rescale_torque:16' if any(TO_max == 0) */
      c_out = false;
      b_k = 0;
      exitg1 = false;
      while ((!exitg1) && (b_k < 4)) {
        if (y->TORQUE_LIM_POS[b_k] == 0.0F) {
          c_out = true;
          exitg1 = true;
        } else {
          b_k++;
        }
      }
      if (c_out) {
        /* 'rescale_torque:17' TO_maxed = [0 0 0 0]; */
        y->TS_TO[0] = 0.0F;
        y->TS_TO[1] = 0.0F;
        y->TS_TO[2] = 0.0F;
        y->TS_TO[3] = 0.0F;
      } else {
        float TO_scaled[4];
        float b_value;
        float best_scale;
        float scale;
        /*  for each wheel, scale up TO_in so that wheel matches corresponding
         */
        /*  TO_max, then check if any other wheels exceed limit */
        /* 'rescale_torque:23' best_scale = 0; */
        best_scale = 0.0F;
        /*  FL */
        /* 'rescale_torque:26' if TO_in(1) ~= 0 */
        if (TS_TO_des_idx_0 != 0.0F) {
          int d_k;
          bool e_out;
          /* 'rescale_torque:27' scale = TO_max(1) / TO_in(1); */
          scale = y->TORQUE_LIM_POS[0] / TS_TO_des_idx_0;
          /* 'rescale_torque:28' TO_scaled = TO_in * scale; */
          TO_scaled[0] = TS_TO_des_idx_0 * scale;
          TO_scaled[1] = TS_TO_des_idx_1 * scale;
          TO_scaled[2] = TS_TO_des_idx_2 * scale;
          TO_scaled[3] = TS_TO_des_idx_3 * scale;
          /* 'rescale_torque:29' if all(TO_scaled <= TO_max) */
          e_out = true;
          d_k = 0;
          exitg1 = false;
          while ((!exitg1) && (d_k < 4)) {
            if (TO_scaled[d_k] > y->TORQUE_LIM_POS[d_k]) {
              e_out = false;
              exitg1 = true;
            } else {
              d_k++;
            }
          }
          if (e_out) {
            /* 'rescale_torque:30' best_scale = max(best_scale, scale); */
            best_scale = fmaxf(0.0F, scale);
          }
        }
        /*  FR */
        /* 'rescale_torque:34' if TO_in(2) ~= 0 */
        if (TS_TO_des_idx_1 != 0.0F) {
          int f_k;
          bool g_out;
          /* 'rescale_torque:35' scale = TO_max(2) / TO_in(2); */
          scale = y->TORQUE_LIM_POS[1] / TS_TO_des_idx_1;
          /* 'rescale_torque:36' TO_scaled = TO_in * scale; */
          TO_scaled[0] = TS_TO_des_idx_0 * scale;
          TO_scaled[1] = TS_TO_des_idx_1 * scale;
          TO_scaled[2] = TS_TO_des_idx_2 * scale;
          TO_scaled[3] = TS_TO_des_idx_3 * scale;
          /* 'rescale_torque:37' if all(TO_scaled <= TO_max) */
          g_out = true;
          f_k = 0;
          exitg1 = false;
          while ((!exitg1) && (f_k < 4)) {
            if (TO_scaled[f_k] > y->TORQUE_LIM_POS[f_k]) {
              g_out = false;
              exitg1 = true;
            } else {
              f_k++;
            }
          }
          if (g_out) {
            /* 'rescale_torque:38' best_scale = max(best_scale, scale); */
            best_scale = fmaxf(best_scale, scale);
          }
        }
        /*  RL */
        /* 'rescale_torque:42' if TO_in(3) ~= 0 */
        if (TS_TO_des_idx_2 != 0.0F) {
          int g_k;
          bool h_out;
          /* 'rescale_torque:43' scale = TO_max(3) / TO_in(3); */
          scale = y->TORQUE_LIM_POS[2] / TS_TO_des_idx_2;
          /* 'rescale_torque:44' TO_scaled = TO_in * scale; */
          TO_scaled[0] = TS_TO_des_idx_0 * scale;
          TO_scaled[1] = TS_TO_des_idx_1 * scale;
          TO_scaled[2] = TS_TO_des_idx_2 * scale;
          TO_scaled[3] = TS_TO_des_idx_3 * scale;
          /* 'rescale_torque:45' if all(TO_scaled <= TO_max) */
          h_out = true;
          g_k = 0;
          exitg1 = false;
          while ((!exitg1) && (g_k < 4)) {
            if (TO_scaled[g_k] > y->TORQUE_LIM_POS[g_k]) {
              h_out = false;
              exitg1 = true;
            } else {
              g_k++;
            }
          }
          if (h_out) {
            /* 'rescale_torque:46' best_scale = max(best_scale, scale); */
            best_scale = fmaxf(best_scale, scale);
          }
        }
        /*  RR */
        /* 'rescale_torque:50' if TO_in(4) ~= 0 */
        if (TS_TO_des_idx_3 != 0.0F) {
          int i_k;
          bool j_out;
          /* 'rescale_torque:51' scale = TO_max(4) / TO_in(4); */
          scale = y->TORQUE_LIM_POS[3] / TS_TO_des_idx_3;
          /* 'rescale_torque:52' TO_scaled = TO_in * scale; */
          TO_scaled[0] = TS_TO_des_idx_0 * scale;
          TO_scaled[1] = TS_TO_des_idx_1 * scale;
          TO_scaled[2] = TS_TO_des_idx_2 * scale;
          TO_scaled[3] = TS_TO_des_idx_3 * scale;
          /* 'rescale_torque:53' if all(TO_scaled <= TO_max) */
          j_out = true;
          i_k = 0;
          exitg1 = false;
          while ((!exitg1) && (i_k < 4)) {
            if (TO_scaled[i_k] > y->TORQUE_LIM_POS[i_k]) {
              j_out = false;
              exitg1 = true;
            } else {
              i_k++;
            }
          }
          if (j_out) {
            /* 'rescale_torque:54' best_scale = max(best_scale, scale); */
            best_scale = fmaxf(best_scale, scale);
          }
        }
        /*  make sure we don't increase torque */
        /* 'rescale_torque:58' best_scale_snipped = snip(best_scale, 0, 1); */
        /* SNIP code generation compatible version of 'clip()' */
        /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB'
         */
        /* 'snip:4' clipped = max(min(value, UB), LB); */
        b_value = fminf(best_scale, 1.0F);
        /*  output */
        /* 'rescale_torque:60' TO_maxed = TO_in * best_scale_snipped; */
        y->TS_TO[0] = TS_TO_des_idx_0 * b_value;
        y->TS_TO[1] = TS_TO_des_idx_1 * b_value;
        y->TS_TO[2] = TS_TO_des_idx_2 * b_value;
        y->TS_TO[3] = TS_TO_des_idx_3 * b_value;
      }
      /* 'vcu_step:65' y.TORQUE_LIM_NEG = [0 0 0 0]; */
      /* 'vcu_step:66' y.TORQUE_LIM_POS = y.TS_TO; */
      /* 'vcu_step:67' y.TORQUE_OUT = y.TS_TO; */
      y->TORQUE_LIM_NEG[0] = 0.0F;
      f8 = y->TS_TO[0];
      y->TORQUE_LIM_POS[0] = f8;
      y->TORQUE_OUT[0] = f8;
      y->TORQUE_LIM_NEG[1] = 0.0F;
      f8 = y->TS_TO[1];
      y->TORQUE_LIM_POS[1] = f8;
      y->TORQUE_OUT[1] = f8;
      y->TORQUE_LIM_NEG[2] = 0.0F;
      f8 = y->TS_TO[2];
      y->TORQUE_LIM_POS[2] = f8;
      y->TORQUE_OUT[2] = f8;
      y->TORQUE_LIM_NEG[3] = 0.0F;
      f8 = y->TS_TO[3];
      y->TORQUE_LIM_POS[3] = f8;
      y->TORQUE_OUT[3] = f8;
    }
  } else if ((y->TH < 0.0F) && (y->REGEN_EN == 1.0F)) {
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
    /* 'vcu_step:70' elseif y.TH < 0 && y.REGEN_EN == 1 */
    /*  baseline regen torque */
    /*  torque limit after speed, current, voltage, thermal derating */
    /* 'vcu_step:73' y = get_BL_RG(p, y); */
    /*  max torque regen allowed by throttle position, subject to F:R balance */
    /*  Function Description */
    /*  vcu_step runs every loop on the TV board */
    /*  */
    /*  Inputs */
    /*    p   vehicle paramater struct. constant */
    /*    x   Raw sensor data struct. filled with data read from CAN */
    /*            in main.c */
    /*    y   Function input and output struct. contains all clipped and */
    /*            filtered variables, variable buffers, and output from */
    /*            this function. */
    /*  Outputs */
    /*    y   modified version of input y */
    /* 'get_BL_RG:18' m = max(y.RG_FR_split, 1-y.RG_FR_split); */
    m = fmaxf(y->RG_FR_split, 1.0F - y->RG_FR_split);
    /* 'get_BL_RG:19' split_FR = [y.RG_FR_split / m, y.RG_FR_split / m,
     * (1-y.RG_FR_split) / m, (1-y.RG_FR_split) / m]; */
    /* 'get_BL_RG:20' TO_ET_RG = y.TH_RG * p.MAX_TO_ABS_RG .* split_FR; */
    b_out = y->TH_RG * p->MAX_TO_ABS_RG;
    TO_ET_RG_tmp = b_out * (y->RG_FR_split / m);
    TO_ET_RG[0] = TO_ET_RG_tmp;
    TO_ET_RG[1] = TO_ET_RG_tmp;
    b_TO_ET_RG_tmp = b_out * ((1.0F - y->RG_FR_split) / m);
    TO_ET_RG[2] = b_TO_ET_RG_tmp;
    TO_ET_RG[3] = b_TO_ET_RG_tmp;
    /*  derating due to speed regulations (no regen below 5 km/hr) */
    /* 'get_BL_RG:23' GS_from_WW = min(y.WW * p.r); */
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
    /*  estimate groundspeed from wheelspeed, take slowest tire Units: [m/s] */
    /* 'get_BL_RG:24' GS_from_WW_snipped = snip(GS_from_WW,
     * p.GS_RG_derating_full, p.GS_RG_derating_zero); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:25' GS_RG_derate = [1 1 1 1] * interp1([p.GS_RG_derating_full,
     * p.GS_RG_derating_zero], [1,0], GS_from_WW_snipped); */
    b_p[0] = p->GS_RG_derating_full;
    b_p[1] = p->GS_RG_derating_zero;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    b_b = interp1(
        b_p, fv,
        fmaxf(fminf(ex, p->GS_RG_derating_zero), p->GS_RG_derating_full));
    /*  Inverter temp safetey derating - derate all motors based on highest
     * inverter temp */
    /* 'get_BL_RG:28' INV_T_snipped = snip(y.INV_T, p.INV_T_derating_full_T,
     * p.INV_T_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:29' INV_T_derate = [1 1 1 1] *
     * interp1([p.INV_T_derating_full_T, p.INV_T_derating_zero_T], [1,0],
     * INV_T_snipped); */
    b_p[0] = p->INV_T_derating_full_T;
    b_p[1] = p->INV_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    d_b = interp1(b_p, fv,
                  fmaxf(fminf(x->INV_T_RAW, p->INV_T_derating_zero_T),
                        p->INV_T_derating_full_T));
    /*  IGBT temp safety derating - derate all motors based on highest IGBT temp
     */
    /* 'get_BL_RG:32' IGBT_T_snipped = snip(y.IGBT_T, p.IGBT_T_derating_full_T,
     * p.IGBT_T_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:33' IGBT_T_derate = [1 1 1 1] *
     * interp1([p.IGBT_T_derating_full_T, p.IGBT_T_derating_zero_T], [1,0],
     * IGBT_T_snipped); */
    b_p[0] = p->IGBT_T_derating_full_T;
    b_p[1] = p->IGBT_T_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    f_b = interp1(b_p, fv,
                  fmaxf(fminf(x->IGBT_T_RAW, p->IGBT_T_derating_zero_T),
                        p->IGBT_T_derating_full_T));
    /*  Motor temp safetey derating - derate all motors based on highest motor
     * temp */
    /* 'get_BL_RG:36' MT_snipped = snip(y.MT, p.MT_derating_full_T,
     * p.MT_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:37' MT_derate = [1 1 1 1] * interp1([p.MT_derating_full_T,
     * p.MT_derating_zero_T], [1,0], MT_snipped); */
    b_p[0] = p->MT_derating_full_T;
    b_p[1] = p->MT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    h_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->MT_RAW, p->MT_derating_zero_T), p->MT_derating_full_T));
    /*  Battery temp safety derating - derate all motors based on highest cell
     * temp */
    /* 'get_BL_RG:40' BT_snipped = snip(y.BT, p.BT_derating_full_T,
     * p.BT_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:41' BT_derate = [1 1 1 1] * interp1([p.BT_derating_full_T,
     * p.BT_derating_zero_T], [1,0], BT_snipped); */
    b_p[0] = p->BT_derating_full_T;
    b_p[1] = p->BT_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    j_b = interp1(
        b_p, fv,
        fmaxf(fminf(x->BT_RAW, p->BT_derating_zero_T), p->BT_derating_full_T));
    /*  Battery overvoltage safety derating */
    /* 'get_BL_RG:44' VB_RG_snipped = snip(y.VB, p.VB_RG_derating_full_T,
     * p.VB_RG_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:45' VB_RG_derate = [1 1 1 1] *
     * interp1([p.VB_RG_derating_full_T, p.VB_RG_derating_zero_T], [1,0],
     * VB_RG_snipped); */
    b_p[0] = p->VB_RG_derating_full_T;
    b_p[1] = p->VB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    l_b = interp1(b_p, fv,
                  fmaxf(fminf(x->VB_RAW, p->VB_RG_derating_zero_T),
                        p->VB_RG_derating_full_T));
    /*  Battery current safety derating */
    /* 'get_BL_RG:48' IB_RG_snipped = snip(y.IB_AVG, p.IB_RG_derating_full_T,
     * p.IB_RG_derating_zero_T); */
    /* SNIP code generation compatible version of 'clip()' */
    /*    clips 'value' to be between lower bound 'LB' and upper bound 'UB' */
    /* 'snip:4' clipped = max(min(value, UB), LB); */
    /* 'get_BL_RG:49' IB_RG_derate = [1 1 1 1] *
     * interp1([p.IB_RG_derating_full_T, p.IB_RG_derating_zero_T], [1,0],
     * IB_RG_snipped); */
    b_p[0] = p->IB_RG_derating_full_T;
    b_p[1] = p->IB_RG_derating_zero_T;
    fv[0] = 1.0F;
    fv[1] = 0.0F;
    m_b = interp1(b_p, fv,
                  fmaxf(fminf(y->IB_AVG, p->IB_RG_derating_zero_T),
                        p->IB_RG_derating_full_T));
    /*  combine derating, multiply by abs max torque to get maximum torque
     * allowed */
    /* 'get_BL_RG:52' TO_RG_MAX = p.MAX_TO_ABS_RG * min([INV_T_derate;
     * IGBT_T_derate; MT_derate; BT_derate; VB_RG_derate; IB_RG_derate;
     * GS_RG_derate], [], 1); */
    /*  compute overall maximum torque */
    /* 'get_BL_RG:55' TO_BL_RG_positive = min(TO_ET_RG, TO_RG_MAX); */
    /* 'get_BL_RG:56' y.TO_BL_RG = -1 * TO_BL_RG_positive; */
    /* 'vcu_step:74' y.TORQUE_LIM_NEG = y.TO_BL_RG; */
    /* 'vcu_step:75' y.TORQUE_LIM_POS = [0 0 0 0]; */
    /* 'vcu_step:76' y.TORQUE_OUT = y.TO_BL_RG; */
    for (j = 0; j < 4; j++) {
      float f1;
      varargin_1[7 * j] = d_b;
      varargin_1[7 * j + 1] = f_b;
      varargin_1[7 * j + 2] = h_b;
      varargin_1[7 * j + 3] = j_b;
      varargin_1[7 * j + 4] = l_b;
      varargin_1[7 * j + 5] = m_b;
      varargin_1[7 * j + 6] = b_b;
      f1 = d_b;
      for (c_i = 0; c_i < 6; c_i++) {
        float f5;
        f5 = varargin_1[(c_i + 7 * j) + 1];
        if (f1 > f5) {
          f1 = f5;
        }
      }
      float f4;
      f4 = fminf(TO_ET_RG[j], p->MAX_TO_ABS_RG * f1);
      y->TO_BL_RG[j] = -f4;
      y->TORQUE_LIM_NEG[j] = -f4;
      y->TORQUE_LIM_POS[j] = 0.0F;
      y->TORQUE_OUT[j] = -f4;
    }
    /*  because of no speed control */
  } else {
    /* 'vcu_step:78' else */
    /*  throttle == 0; throttle < 0 & no regen; or fallback */
    /* 'vcu_step:80' y.TORQUE_LIM_NEG = [0 0 0 0]; */
    /* 'vcu_step:81' y.TORQUE_LIM_POS = [0 0 0 0]; */
    /* 'vcu_step:82' y.SPEED_OUT = [0 0 0 0]; */
    /* 'vcu_step:83' y.TORQUE_OUT = [0 0 0 0]; */
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
    /*  because of no speed control */
  }
}

/*
 * File trailer for vcu_step.c
 *
 * [EOF]
 */
