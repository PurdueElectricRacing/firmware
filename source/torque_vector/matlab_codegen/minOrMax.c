#include "minOrMax.h"

void minimum(const double x[28], double ex[4])
{
  int i;
  int j;
  for (j = 0; j < 4; j++) {
    double d;
    d = x[7 * j];
    for (i = 0; i < 6; i++) {
      double d1;
      d1 = x[(i + 7 * j) + 1];
      if (d > d1) {
        d = d1;
      }
    }
    ex[j] = d;
  }
}
