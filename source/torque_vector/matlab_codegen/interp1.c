#include "interp1.h"

float interp1(const float varargin_1[2], const float varargin_2[2],
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
