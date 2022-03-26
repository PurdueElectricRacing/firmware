#ifndef __FORCE__
#define __FORCE__

#define a0          57/220
#define a1          437/2640
#define a2          3/176



#define MAX_V        250         // maximum velocity for which we're able to return velocity value, mm/sec
#define VEL_SIZE     26          // size of the velocity storage 

float damp_force (float v, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE]);
float pot_speed(int* x, float resolution, float delta_T, int n, int start);
float f(int* x, float error, float resolution, float zero, float delta, float k, int n, const float force_reb [VEL_SIZE], const float force_comp [VEL_SIZE], int start);
float f_rear(int* x, int start);
float f_front(int* x, int start);
void n_rear(int* xl, int* xr, float* n_l, float* n_r, int start);
void n_front(int* xl, int* xr, float* n_l, float* n_r, int start);

#define DELTA_REAR   1       // sampling rate of the microcontoller, s
#define ERROR_REAR   100       // error for the windowing algorithm, needs to be tuned.
#define N_REAR       10          // number of measurements passed, maximum depth of the algorithm

#define DELTA_FRONT   1       // sampling rate of the microcontoller, s
#define ERROR_FRONT   0.001       // error for the windowing algorithm, needs to be tuned.
#define N_FRONT       10          // number of measurements passed, maximum depth of the algorithm

#define D_D_FRONT     3.36        // diagonal wheel connection distance,
#define D_W_FRONT     3.36        // damper+spring distance
#define D_A_FRONT     1.46        // ARB distance
#define GAMMA_FRONT   2004.75     // torsion coefficient, m*N/rad
#define S_FRONT       5           // ARB leverage, m
#define COS_A_FRONT   0.74        // cos_alpha
#define K_FRONT       146.75      // spring constant, N/m

#define D_D_REAR    3.36          // diagonal wheel connection distance, in
#define D_W_REAR     3.36        // damper+spring distance, in
#define D_A_REAR     1.46        // ARB distance, in
#define GAMMA_REAR  2004.75     // torsion coefficient, lb*in/rad
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