#include "ac_compute_R.h"
#include "median.h"
#include <math.h>

void ac_compute_R(const float ax[151], const float ay[151], const float az[151],
                  float R[9])
{
  static const signed char b[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
  float b_u[9];
  float u[3];
  float XYZs_idx_0_tmp;
  float XYZs_idx_1_tmp;
  float XYZs_idx_2;
  float XYZv_idx_2;
  float a;
  float scale;
  float t;
  float theta;
  int i;
  XYZs_idx_0_tmp = median(ax);
  XYZs_idx_1_tmp = median(ay);
  XYZs_idx_2 = median(az);
  XYZv_idx_2 = sqrtf(
      (XYZs_idx_0_tmp * XYZs_idx_0_tmp + XYZs_idx_1_tmp * XYZs_idx_1_tmp) +
      XYZs_idx_2 * XYZs_idx_2);
  scale = 1.29246971E-26F;
  if (XYZv_idx_2 > 1.29246971E-26F) {
    a = 1.0F;
    scale = XYZv_idx_2;
  } else {
    t = XYZv_idx_2 / 1.29246971E-26F;
    a = t * t;
  }
  a = scale * sqrtf(a);
  theta = acosf(XYZs_idx_2 * XYZv_idx_2 / (a * a));
  u[0] = XYZs_idx_1_tmp * XYZv_idx_2;
  u[1] = 0.0F - XYZs_idx_0_tmp * XYZv_idx_2;
  scale = 1.29246971E-26F;
  XYZs_idx_2 = fabsf(u[0]);
  if (XYZs_idx_2 > 1.29246971E-26F) {
    a = 1.0F;
    scale = XYZs_idx_2;
  } else {
    t = XYZs_idx_2 / 1.29246971E-26F;
    a = t * t;
  }
  XYZs_idx_2 = fabsf(u[1]);
  if (XYZs_idx_2 > scale) {
    t = scale / XYZs_idx_2;
    a = a * t * t + 1.0F;
    scale = XYZs_idx_2;
  } else {
    t = XYZs_idx_2 / scale;
    a += t * t;
  }
  a = scale * sqrtf(a);
  u[0] /= a;
  u[1] /= a;
  u[2] = 0.0F / a;
  XYZs_idx_2 = cosf(theta);
  a = sinf(theta);
  R[0] = 0.0F;
  R[3] = a * -u[2];
  R[6] = a * u[1];
  R[1] = a * u[2];
  R[4] = 0.0F;
  R[7] = a * -u[0];
  R[2] = a * -u[1];
  R[5] = a * u[0];
  R[8] = 0.0F;
  for (i = 0; i < 3; i++) {
    b_u[3 * i] = u[0] * u[i];
    b_u[3 * i + 1] = u[1] * u[i];
    b_u[3 * i + 2] = u[2] * u[i];
  }
  for (i = 0; i < 9; i++) {
    R[i] = (XYZs_idx_2 * (float)b[i] + R[i]) + (1.0F - XYZs_idx_2) * b_u[i];
  }
}
