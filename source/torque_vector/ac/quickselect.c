#include "quickselect.h"

int thirdOfFive(const float v[151], int ia, int ib)
{
  int im;
  if ((ia == ib) || (ia + 1 == ib)) {
    im = ia;
  } else if ((ia + 2 == ib) || (ia + 3 == ib)) {
    float v4;
    v4 = v[ia - 1];
    if (v4 < v[ia]) {
      float v5;
      v5 = v[ia + 1];
      if (v[ia] < v5) {
        im = ia + 1;
      } else if (v4 < v5) {
        im = ia + 2;
      } else {
        im = ia;
      }
    } else {
      float v5;
      v5 = v[ia + 1];
      if (v4 < v5) {
        im = ia;
      } else if (v[ia] < v5) {
        im = ia + 2;
      } else {
        im = ia + 1;
      }
    }
  } else {
    float v4;
    float v5;
    int j2;
    int j3;
    int j4;
    int j5;
    v4 = v[ia - 1];
    if (v4 < v[ia]) {
      v5 = v[ia + 1];
      if (v[ia] < v5) {
        im = ia;
        j2 = ia;
        j3 = ia + 2;
      } else if (v4 < v5) {
        im = ia;
        j2 = ia + 1;
        j3 = ia + 1;
      } else {
        im = ia + 2;
        j2 = ia - 1;
        j3 = ia + 1;
      }
    } else {
      v5 = v[ia + 1];
      if (v4 < v5) {
        im = ia + 1;
        j2 = ia - 1;
        j3 = ia + 2;
      } else if (v[ia] < v5) {
        im = ia + 1;
        j2 = ia + 1;
        j3 = ia;
      } else {
        im = ia + 2;
        j2 = ia;
        j3 = ia;
      }
    }
    j4 = ia;
    j5 = ia + 1;
    v4 = v[ia + 2];
    v5 = v[ia + 3];
    if (v5 < v4) {
      j4 = ia + 1;
      j5 = ia;
      v5 = v4;
      v4 = v[ia + 3];
    }
    if (v5 >= v[im - 1]) {
      if (v5 < v[j2]) {
        im = j5 + 3;
      } else if (v4 < v[j2]) {
        im = j2 + 1;
      } else if (v4 < v[j3 - 1]) {
        im = j4 + 3;
      } else {
        im = j3;
      }
    }
  }
  return im;
}
