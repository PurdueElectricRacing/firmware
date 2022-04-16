#ifndef __FORCE__
#define __FORCE__

//Coefficients for second-oreder polynomial fit
#define A0          57/220
#define A1          437/2640
#define A2          3/176

#define MAX_V        250         // maximum velocity for which we're able to return damping force value, mm/sec
#define VEL_SIZE     26          // size of the velocity storage

//Geometry of the car
struct Geometry {
    //Shock pot positions
    // Length
    const float oa;
    const float ob;
    const float oc;
    const float od;
    float cd;           // Dampre length, updatable

    float d_d;
    float d_w;
    float d_a;

    // Angles and their sines/cosines
    const float fw_vert;
    const float od_vert;
    const float cob;
    const float coa;

    float fw_ob;
    float sin_ob_fw;
    float vert_ao;
    float oa_fa;
    float vert_fa;
    float ocd;
    float cos_ocd;
    float sin_ocd;
    float doc;

    // Displpacement for ARB
    float x_a;
};

struct ForceParam {
    float f_damp;       // Damping force
    float f_total;      // Total damper force
    float n;             // Normal force of the wheel
    float k;            // Spring coefficient
    float gamma;        // Torsion coefficient
    float v;            // Damper velocity
    float x_0;          // Rest position of the spring, force is determied as:
                        // F_spring = k * (cd - x_0)
    float s;            // Leverage of ARB
};

struct ADCconv {
    // Model for finding the length is CD = adc_0 + resolution*(ADC measurement)
    float adc_0;
    float resolution;
};
struct Wheel {
    struct Geometry geom;
    struct ForceParam param;
    struct ADCconv adc;
};

void _get_pot_speed_pos(int* x, struct Wheel* w, float delta_T, int n, int start);
void _get_damp_force (float *f_damp, float v, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE]);
void _upadte_geometry (struct Geometry *g);
void _get_total_force (struct Wheel *w);
void _get_normal_force (struct Wheel *w, struct Wheel *w_other);
void _calc_pipeline(int* x_l, int* x_r, struct Wheel *w_l, struct Wheel *w_r, int start);
void force_rear(float* n_l, float* n_r, int* x_l, int* x_r, int start);
void force_front(float* n_l, float* n_r, int* x_l, int* x_r, int start);


#define N_SAMPLE    10
#define DELTA_T     0.001

#define DELTA_REAR   1       // sampling rate of the microcontoller, s
#define ERROR_REAR   100       // error for the windowing algorithm, needs to be tuned.
#define N_REAR       10          // number of measurements passed, maximum depth of the algorithm

#define DELTA_FRONT   1       // sampling rate of the microcontoller, s
#define ERROR_FRONT   0.001       // error for the windowing algorithm, needs to be tuned.
#define N_FRONT       10          // number of measurements passed, maximum depth of the algorithm

#define GAMMA_FRONT   2004.75     // torsion coefficient, m*N/rad
#define S_FRONT       5           // ARB leverage, m
#define COS_A_FRONT   0.74        // cos_alpha
#define K_FRONT       146.75      // spring constant, N/m

#define GAMMA_REAR   2004.75     // torsion coefficient, lb*in/rad
#define S_REAR       5           // ARB leverage, 
#define COS_A_REAR   0.74        // cos_alpha
#define K_REAR       0           // spring constant, lb/in

#define RESOLUTION_REAR  1  // convert ADC value to real numbers, resolution of the ADC, 
#define ZERO_REAR        0   // actual displacement when ADC shows 0, in  

// ignore it, just calculations
// 1235 - 1 V - 0 deisplacement 2477 - 2V 1 inch

#define RESOLUTION_FRONT  0.0001  // convert ADC value to real numbers, resolution of the ADC, 
#define ZERO_FRONT        0.001   // actual displacement when ADC shows 0


#endif