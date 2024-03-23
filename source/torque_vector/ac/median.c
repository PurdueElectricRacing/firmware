#include "median.h"
#include "quickselect.h"
#include <string.h>

float median(const float x[151])
{
  float a__4[151];
  int ia;
  int ib;
  int ilast;
  int ipiv;
  int k;
  int oldnv;
  bool checkspeed;
  bool exitg1;
  bool isslow;
  memcpy(&a__4[0], &x[0], 151U * sizeof(float));
  ipiv = 75;
  ia = 0;
  ib = 150;
  ilast = 150;
  oldnv = 151;
  checkspeed = false;
  isslow = false;
  exitg1 = false;
  while ((!exitg1) && (ia + 1 < ib + 1)) {
    float vref;
    bool guard1;
    vref = a__4[ipiv];
    a__4[ipiv] = a__4[ib];
    a__4[ib] = vref;
    ilast = ia;
    ipiv = -1;
    for (k = ia + 1; k <= ib; k++) {
      float vk_tmp;
      vk_tmp = a__4[k - 1];
      if (vk_tmp == vref) {
        a__4[k - 1] = a__4[ilast];
        a__4[ilast] = vk_tmp;
        ipiv++;
        ilast++;
      } else if (vk_tmp < vref) {
        a__4[k - 1] = a__4[ilast];
        a__4[ilast] = vk_tmp;
        ilast++;
      }
    }
    a__4[ib] = a__4[ilast];
    a__4[ilast] = vref;
    guard1 = false;
    if (ilast + 1 >= 76) {
      if (ilast - ipiv <= 76) {
        exitg1 = true;
      } else {
        ib = ilast - 1;
        guard1 = true;
      }
    } else {
      ia = ilast + 1;
      guard1 = true;
    }
    if (guard1) {
      int c;
      c = (ib - ia) + 1;
      if (checkspeed) {
        isslow = (c > oldnv / 2);
        oldnv = c;
      }
      checkspeed = !checkspeed;
      if (isslow) {
        while (c > 1) {
          int i;
          int ngroupsof5;
          int nlast;
          ngroupsof5 = c / 5;
          nlast = c - ngroupsof5 * 5;
          c = ngroupsof5;
          i = (unsigned char)ngroupsof5;
          for (k = 0; k < i; k++) {
            ipiv = (ia + k * 5) + 1;
            ipiv = thirdOfFive(a__4, ipiv, ipiv + 4) - 1;
            ilast = ia + k;
            vref = a__4[ilast];
            a__4[ilast] = a__4[ipiv];
            a__4[ipiv] = vref;
          }
          if (nlast > 0) {
            ipiv = (ia + ngroupsof5 * 5) + 1;
            ipiv = thirdOfFive(a__4, ipiv, (ipiv + nlast) - 1) - 1;
            ilast = ia + ngroupsof5;
            vref = a__4[ilast];
            a__4[ilast] = a__4[ipiv];
            a__4[ipiv] = vref;
            c = ngroupsof5 + 1;
          }
        }
      } else if (c >= 3) {
        ipiv = ia + (int)((unsigned int)(c - 1) >> 1);
        if (a__4[ia] < a__4[ipiv]) {
          if (a__4[ipiv] >= a__4[ib]) {
            if (a__4[ia] < a__4[ib]) {
              ipiv = ib;
            } else {
              ipiv = ia;
            }
          }
        } else if (a__4[ia] < a__4[ib]) {
          ipiv = ia;
        } else if (a__4[ipiv] < a__4[ib]) {
          ipiv = ib;
        }
        if (ipiv + 1 > ia + 1) {
          vref = a__4[ia];
          a__4[ia] = a__4[ipiv];
          a__4[ipiv] = vref;
        }
      }
      ipiv = ia;
      ilast = ib;
    }
  }
  return a__4[ilast];
}
