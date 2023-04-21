#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
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

struct Wheel left_rear = {
    .geom = {
        // Length
        .oa = 1, 
        .ob = 2,
        .oc = 3,
        .od = 4,
        
        // Angles and their sines/cosines
        .fw_vert = 5,
        .od_vert = 6,
        .cob = 7,
        .coa = 8,
    }
};

struct Wheel right_rear = {
    .geom = {
        // Length
        .oa = 1, 
        .ob = 2,
        .oc = 3,
        .od = 4,

        // Angles and their sines/cosines
        .fw_vert = 5,
        .od_vert = 6,
        .cob = 7,
        .coa = 8,
    }
};

struct Wheel left_front = {
    .geom = {
        // Length
        .oa = 1, 
        .ob = 2,
        .oc = 3,
        .od = 4,

        // Angles and their sines/cosines
        .fw_vert = 5,
        .od_vert = 6,
        .cob = 7,
        .coa = 8,
    }
};

struct Wheel right_front = {
    .geom = {
        // Length
        .oa = 1, 
        .ob = 2,
        .oc = 3,
        .od = 4,

        // Angles and their sines/cosines
        .fw_vert = 5,
        .od_vert = 6,
        .cob = 7,
        .coa = 8,
    }
};

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
    *.csv files where the force is interpolted for values 0, 10, 20, ..., 250. This basically implements
    first oreder interpolation.
*/
void _get_pot_speed_pos(int* x, struct Wheel* w, float delta_T, int n, int start) {     // second order polynomial fitting
    // x - array of position sensor data, which is cyclically updated
    // start - index of the latest measurement
    // delta_T - time interval between measurements
    // resolution - conversion of ADC step to real length

    float b = 0;             
    float s0 = 0;
    float s1 = 0;
    float s2 = 0;

    for (int i = 0; i < n; i++) {
        s0 += x[(start + i) % n];
        s1 += i * x[(start + i) % n];
        s2 += i * i * x[(start + i) % n];
    }
    b = A0 * s0 - A1 * s1 + A2 * s2;   

    w->param.v = w->adc.resolution * b / delta_T;
    w->geom.cd = w->adc.resolution * x[start] +  w->adc.adc_0;
}

void _get_damp_force (float *f_damp, float v, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE]) {
    int val= (int)(floor(fabs(v/10)));
    if (v >= MAX_V) {
        *f_damp = force_reb[VEL_SIZE-1];
        printf("Damper velocity overshoot (positive, rebound)\n");
    } else if (v <= -MAX_V) {
        *f_damp = force_comp[VEL_SIZE-1];
        printf("Damper velocity overshoot (negative, compresion)\n");
    } else if (v > 0) {
        *f_damp = (force_reb[val] * (val * 10 + 10 - v) + force_reb[val + 1] * (v - val * 10)) / 10;
    } else {
        *f_damp = (force_comp[val] * (v + val * 10 + 10) + force_comp[val + 1] * (-val * 10 - v)) / 10;
    }
}


void _upadte_geometry (struct Geometry *g) {
    // update gometry of a suspension node, the comments correspond to the
    // equation numbers in the description document

    g->cos_ocd = (g->oc*g->oc + g->cd*g->cd - g->od*g->od)/(2 * g->oc * g->cd); // 1

    g->sin_ocd = sqrt(1 - g->cos_ocd*g->cos_ocd); // 2
    
    g->d_d = g->oc * g->sin_ocd; // 3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

    g->doc = fabs(asin(sin(g->ocd) * g->cd / g->od)); // 4

    g->fw_ob = g->cob + g->doc - g->od_vert + g->fw_vert; //5

    g->sin_ob_fw = sin(g->fw_ob); // 6

    g->d_w = g->ob * g->sin_ob_fw; // 7

    g->vert_ao = g->coa + g->doc + g->od_vert; // 8

    g->oa_fa = g->vert_fa - g->vert_ao; // 9

    g->d_a = g->oa * fabs(sin(g->oa_fa)); // 10

    g->x_a = g->oa * sin (g->vert_ao - M_PI_2); // 11
}

void _get_total_force (struct Wheel *w) {
    float f_spring_l = -w->param.k * (w->geom.cd - w->param.x_0);
    w->param.f_total = f_spring_l + w->param.f_damp;
}

void _get_normal_force (struct Wheel *w, struct Wheel *w_other) {

    w->param.n = (w->param.f_total * w->geom.d_d + 
                        w->param.gamma * (w_other->geom.x_a - w->geom.x_a) *
                        w->geom.d_a / w->param.s / w->param.s
                    ) / (w->geom.d_w * cos(w->geom.fw_vert));

}

void _calc_pipeline(int* x_l, int* x_r, struct Wheel *w_l, struct Wheel *w_r, int start) {
    _get_pot_speed_pos(x_l, w_l, DELTA_T, N_SAMPLE, start);
    _get_pot_speed_pos(x_r, w_r, DELTA_T, N_SAMPLE, start);

    _get_damp_force (&(w_l->param.f_damp), w_l->param.v, FORCE_REB_REAR, FORCE_COMP_REAR);
    _get_damp_force (&(w_r->param.f_damp), w_r->param.v, FORCE_REB_REAR, FORCE_COMP_REAR);

    _upadte_geometry(&(w_l->geom));
    _upadte_geometry(&(w_r->geom));

    _get_total_force (w_l);
    _get_total_force (w_r);

    _get_normal_force (w_l, w_r);
    _get_normal_force (w_r, w_l);
}

void force_rear(float* n_l, float* n_r, int* x_l, int* x_r, int start) {
    _calc_pipeline(x_l, x_r, &left_rear, &right_rear, start);
    *n_l = left_rear.param.n;
    *n_r = right_rear.param.n;
}

void force_front(float* n_l, float* n_r, int* x_l, int* x_r, int start) {
    _calc_pipeline(x_l, x_r, &left_front, &right_front, start);
    *n_l = left_front.param.n;
    *n_r = right_front.param.n;
}

// float f(int* x, float pot_speed, float resolution, float zero, float damp_force, float k, int start) { 
//     return -k * (x[start] * resolution + zero) + damp_force;
// }

// float f_rear(int* x, int start) {
//     return f(x, ERROR_REAR, RESOLUTION_REAR, ZERO_REAR, DELTA_REAR, K_REAR, N_REAR, FORCE_REB_REAR, FORCE_COMP_REAR, start);
// }

// float f_front(int* x, int start) {
//     return f(x, ERROR_FRONT, RESOLUTION_FRONT, ZERO_FRONT, DELTA_FRONT, K_FRONT, N_FRONT, FORCE_REB_FRONT, FORCE_COMP_FRONT, start);
// }

// void n_rear(int* xl, int* xr, float* n_l, float* n_r, int start) {
//     *n_l=(f_rear(xl, start) * D_D_REAR + ( GAMMA_REAR * (D_A_REAR / D_D_REAR) * (xr[start] - xl[start])) / (S_REAR * S_REAR) * D_A_REAR) / (D_W_REAR * COS_A_REAR);
//     *n_r=(f_rear(xr, start) * D_D_REAR + ( GAMMA_REAR * (D_A_REAR / D_D_REAR) * (xl[start] - xr[start])) / (S_REAR * S_REAR) * D_A_REAR) / (D_W_REAR * COS_A_REAR);
// }

// void n_front(int* xl, int* xr, float* n_l, float* n_r, int start) {
//     *n_l=(f_front(xl, start) * D_D_FRONT + ( GAMMA_FRONT * (D_A_FRONT / D_D_FRONT) * (xr[start] - xl[start])) / (S_FRONT * S_FRONT) * D_A_FRONT) / (D_W_FRONT * COS_A_FRONT);
//     *n_r=(f_front(xr, start) * D_D_FRONT + ( GAMMA_FRONT * (D_A_FRONT / D_D_FRONT) * (xl[start] - xr[start])) / (S_FRONT * S_FRONT) * D_A_FRONT) / (D_W_FRONT * COS_A_FRONT);
// }

/*
    This is the main calculation function. It adjusts sampling window according to, 
    basically, change in speed. If the speed changes quickly, the estimator use only
    first few measurements to reduce the delay. If the speed chages slowly, 
    it adds more points t0 increase precision. The parameter ERROR that regulates this should be tuned 
    to achieve the best performance

    Input: sequence of the last N measurements of ADC.
    Return: estimated speed of the shockpot

    front wheel: coming up...
*/

// float pot_speed(int* x, float error, float resolution, float delta, int n, int start) {     // implements adaptive windowing as described in (1)

//     float b = 0;
//     float b_old = 0;                 

//     for (int i = 5; i < n; i++) {               // initialize the loop over the window size
//         float s1 = 0;                           // variables for summation
//         float s2 = 0;
//         for (int j = 0; j<=i; j++) {
//             s1 += x[(start + j) % n];
//             s2 += j * x[(start + j) % n];
//         }
//         b = (i * s1 - 2 * s2) / (i + 1) / (i + 2) / i * 6;                  // implementing the formula from the paper.
//         for (int j = 1; j<=i; j++) {
//             if (fabs(x[(start + j) % n] - (x[start] - j * b)) > error) {
//                 return resolution * b_old / delta;      // returing the spring force + damping force
//             }
//         }
//         b_old = b;
//     }
//     return resolution * b_old / delta; 
// }