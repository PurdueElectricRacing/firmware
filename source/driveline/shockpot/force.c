#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "force.h"

/*  
    Ver 1.0, 2/05/22
    
    Function foe determining the force based on a
    simple model in the PDF. Some of the values need to 
    be double checked before applying to the system.

    Ref. 1 F. Janabi-Sharifi, V. Hayward and C. . -S. J. Chen, "Discrete-time adaptive windowing for 
    velocity estimation," in IEEE Transactions on Control Systems Technology, 
    vol. 8, no. 6, pp. 1003-1009, Nov. 2000, doi: 10.1109/87.880606.
*/

const float FORCE_COMP_FRONT[] = {    // copy data from one of the csv files, gives the measured compression damping force for a 
          0,                     // particular configuration of valves
    266.907,
    347.769,
    391.315,
    428.215,
    464.746,
    488.642,
    511.53,
    534.269,
    557.547,
    579.819,
    591.2,
    600.593,
    610.17,
    620.067,
    629.761,
    638.914,
    646.8,
    654.686,
    662.572,
    670.458,
    678.211,
    686.843,
    696.301,
    705.759,
    715.218,
};

const float FORCE_REB_FRONT[] = {    // copy data from one of the csv files, gives the measured rebound damping force for a 
           0,                     // particular configuration of valves
    -232.487,
    -290.239,
    -324.172,
    -349.805,
    -374.158,
    -394.209,
    -413.159,
    -433.588,
    -453.067,
    -469.533,
    -485.999,
    -497.792,
    -509.725,
    -521.864,
    -534.617,
    -543.42,
    -550.862,
    -558.305,
    -566.015,
    -574.208,
    -582.04,
    -589.937,
    -598.443,
    -606.955,
    -615.468,
};

const float FORCE_COMP_REAR[] = {    // copy data from one of the csv files, gives the measured compression damping force for a 
          0,                     // particular configuration of valves
    266.907,
    347.769,
    391.315,
    428.215,
    464.746,
    488.642,
    511.53,
    534.269,
    557.547,
    579.819,
    591.2,
    600.593,
    610.17,
    620.067,
    629.761,
    638.914,
    646.8,
    654.686,
    662.572,
    670.458,
    678.211,
    686.843,
    696.301,
    705.759,
    715.218,
};

const float FORCE_REB_REAR[] = {    // copy data from one of the csv files, gives the measured rebound damping force for a 
           0,                     // particular configuration of valves
    -232.487,
    -290.239,
    -324.172,
    -349.805,
    -374.158,
    -394.209,
    -413.159,
    -433.588,
    -453.067,
    -469.533,
    -485.999,
    -497.792,
    -509.725,
    -521.864,
    -534.617,
    -543.42,
    -550.862,
    -558.305,
    -566.015,
    -574.208,
    -582.04,
    -589.937,
    -598.443,
    -606.955,
    -615.468,
};

/*
    Calculates damping force based on the data from dyno tests. The data was digitized and put into corresponding
    *.csv files where the force is interpolted for values 0, 10, 20, ..., 250. This basically implements further 
    first oreder interpolation.
*/

float damp_force (float v, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE]) {
    int val= (int)(floor(fabs(v/10)));
    if (v >= MAX_V) {
        return force_reb[VEL_SIZE-1];
        printf("Damper velocity overshoot (positive, rebound)\n");
    } else if (v <= -MAX_V) {
        return force_comp[VEL_SIZE-1];
        printf("Damper velocity overshoot (negative, compresion)\n");
    } else if (v > 0) {
        return (force_reb[val] * (val * 10 + 10 - v) + force_reb[val + 1] * (v - val * 10)) / 10;
    } else {
        return (force_comp[val] * (v + val * 10 + 10) + force_comp[val + 1] * (-val * 10 - v)) / 10;
    }
}

/*
    This is the main calculation function. It adjusts sampling window according to, 
    basically, change in speed. If the speed changes quickly, the estimator use only
    first few measurements to reduce the delay. If the speed chages slowly, 
    it adds more points t0 increase precision. The parameter ERROR that regulates this should be tuned 
    to achieve the best performance

    Input: sequence of the last N measurements of ADC.
    Return: suspension unit force

    front wheel: coming up...
*/

float f(int* x, float error, float resolution, float zero, float delta, float k, int n, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE], int start) {     // implements adaptive windowing as described in (1)

    float b = 0;
    float b_old = 0;
    float disp = x[0];                          // current displacement                    

    for (int i = 5; i < n; i++) {               // initialize the loop over the window size
        float s1 = 0;                           // variables for summation
        float s2 = 0;
        for (int j = 0; j<=i; j++) {
            s1 += x[(start + j) % n];
            s2 += j * x[(start + j) % n];
        }
        b = (i * s1 - 2 * s2) / (i + 1) / (i + 2) / i * 6;                  // implementing the formula from the paper.
        for (int j = 1; j<=i; j++) {
            if (fabs(x[(start + j) % n] - (x[start] - j * b)) > error) {
                return -k * (disp * resolution + zero) + damp_force(resolution * b_old / delta, force_reb, force_comp);        // returing the spring force + damping force
            }
        }
        b_old = b;
    }
    return -k * (disp * resolution + zero) - damp_force(resolution * b_old / delta, force_reb, force_comp);
}

float f_rear(int* x, int start) {
    return f(x, ERROR_REAR, RESOLUTION_REAR, ZERO_REAR, DELTA_REAR, K_REAR, N_REAR, FORCE_REB_REAR, FORCE_COMP_REAR, start);
}

float f_front(int* x, int start) {
    return f(x, ERROR_FRONT, RESOLUTION_FRONT, ZERO_FRONT, DELTA_FRONT, K_FRONT, N_FRONT, FORCE_REB_FRONT, FORCE_COMP_FRONT, start);
}

void n_rear(int* xl, int* xr, float* n_l, float* n_r, int start) {
    *n_l=(f_rear(xl, start) * D_D_REAR + ( GAMMA_REAR * (D_A_REAR / D_D_REAR) * (xr[start] - xl[start])) / (S_REAR * S_REAR) * D_A_REAR) / (D_W_REAR * COS_A_REAR);
    *n_r=(f_rear(xr, start) * D_D_REAR + ( GAMMA_REAR * (D_A_REAR / D_D_REAR) * (xl[start] - xr[start])) / (S_REAR * S_REAR) * D_A_REAR) / (D_W_REAR * COS_A_REAR);
}

void n_front(int* xl, int* xr, float* n_l, float* n_r, int start) {
    *n_l=(f_front(xl, start) * D_D_FRONT + ( GAMMA_FRONT * (D_A_FRONT / D_D_FRONT) * (xr[start] - xl[start])) / (S_FRONT * S_FRONT) * D_A_FRONT) / (D_W_FRONT * COS_A_FRONT);
    *n_r=(f_front(xr, start) * D_D_FRONT + ( GAMMA_FRONT * (D_A_FRONT / D_D_FRONT) * (xl[start] - xr[start])) / (S_FRONT * S_FRONT) * D_A_FRONT) / (D_W_FRONT * COS_A_FRONT);
}