/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: SFS.c
 *
 * Code generated for Simulink model 'SFS'.
 *
 * Model version                  : 2.298
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Mar  7 19:18:20 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "SFS.h"
#include "rtwtypes.h"
#include <string.h>
#include <math.h>
#include <float.h>
#include <stddef.h>
#define NumBitsPerChar 8U

extern real_T rt_remd_snf(real_T u0, real_T u1);
static void SFS_Init(DW_SFS *localDW);
static void SFS_d(const real_T rtu_sensor_raw[3], const real_T rtu_sensor_raw_p[3], const real_T rtu_sensor_raw_j[3], const real_T rtu_sensor_raw_d[3], const real_T rtu_sensor_raw_h[3], real_T rty_sensor_fused[3], real_T rty_sensor_fused_p[4], real_T rty_sensor_fused_j[3], real_T rty_sensor_fused_d[3], real_T rty_sensor_fused_h[3], real_T rty_sensor_fused_pa[3], real_T rty_sensor_fused_g[3], real_T rty_sensor_fused_c[3], real_T rty_sensor_fused_f[3], DW_SFS *localDW);

/* Forward declaration for local functions */
static void quaternionBase_rotmat(real_T q_a, real_T q_b, real_T q_c, real_T q_d,
                                  real_T r[9]);
static void BasicEKF_repairQuaternion(const insfilterAsync *obj, real_T x[28]);
static void AsyncMARGGPSFuserBase_predict(insfilterAsync *obj, real_T dt, DW_SFS *localDW);
static void BasicEKF_correctEqn(const insfilterAsync *obj, real_T x[28], real_T P_0[784], const real_T h[3], const real_T H[84], const real_T z[3], const real_T R[9], real_T innov[3], real_T innovCov[9], DW_SFS *localDW);
static void AsyncMARGGPSFuserBase_fuseaccel(insfilterAsync *obj, const real_T accel[3], real_T Raccel, DW_SFS *localDW);
static void cosd(real_T *x);
static void sind(real_T *x);
static void lla2ecef(const real_T llaPos[3], real_T ecefPos[3]);
static void AsyncMARGGPSFuserBase_fusegps(insfilterAsync *obj, const real_T lla[3], real_T Rpos, const real_T vel[3], real_T Rvel, DW_SFS *localDW);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);

#define NOT_USING_NONFINITE_LITERALS 1

extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static void rt_InitInfAndNaN(size_t realSize);
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
typedef struct
{
    struct
    {
        uint32_T wordH;
        uint32_T wordL;
    } words;
} BigEndianIEEEDouble;

typedef struct
{
    struct
    {
        uint32_T wordL;
        uint32_T wordH;
    } words;
} LittleEndianIEEEDouble;

typedef struct
{
    union
    {
        real32_T wordLreal;
        uint32_T wordLuint;
    } wordL;
} IEEESingle;

real_T rtInf;
real_T rtMinusInf;
real_T rtNaN;
real32_T rtInfF;
real32_T rtMinusInfF;
real32_T rtNaNF;
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);

/*
 * Initialize rtNaN needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetNaN(void)
{
    size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
    real_T nan = 0.0;
    if (bitsPerReal == 32U)
    {
        nan = rtGetNaNF();
    }
    else
    {
        union
        {
            LittleEndianIEEEDouble bitVal;
            real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0xFFF80000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        nan = tmpVal.fltVal;
    }

    return nan;
}

/*
 * Initialize rtNaNF needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetNaNF(void)
{
    IEEESingle nanF = {{0.0F}};

    nanF.wordL.wordLuint = 0xFFC00000U;
    return nanF.wordL.wordLreal;
}

/*
 * Initialize the rtInf, rtMinusInf, and rtNaN needed by the
 * generated code. NaN is initialized as non-signaling. Assumes IEEE.
 */
static void rt_InitInfAndNaN(size_t realSize)
{
    (void)(realSize);
    rtNaN = rtGetNaN();
    rtNaNF = rtGetNaNF();
    rtInf = rtGetInf();
    rtInfF = rtGetInfF();
    rtMinusInf = rtGetMinusInf();
    rtMinusInfF = rtGetMinusInfF();
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
    return (boolean_T)((value == rtInf || value == rtMinusInf) ? 1U : 0U);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
    return (boolean_T)(((value) == rtInfF || (value) == rtMinusInfF) ? 1U : 0U);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
    boolean_T result = (boolean_T)0;
    size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
    if (bitsPerReal == 32U)
    {
        result = rtIsNaNF((real32_T)value);
    }
    else
    {
        union
        {
            LittleEndianIEEEDouble bitVal;
            real_T fltVal;
        } tmpVal;

        tmpVal.fltVal = value;
        result = (boolean_T)((tmpVal.bitVal.words.wordH & 0x7FF00000) == 0x7FF00000 &&
                             ((tmpVal.bitVal.words.wordH & 0x000FFFFF) != 0 ||
                              (tmpVal.bitVal.words.wordL != 0)));
    }

    return result;
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
    IEEESingle tmp;
    tmp.wordL.wordLreal = value;
    return (boolean_T)((tmp.wordL.wordLuint & 0x7F800000) == 0x7F800000 &&
                       (tmp.wordL.wordLuint & 0x007FFFFF) != 0);
}

/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetInf(void)
{
    size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
    real_T inf = 0.0;
    if (bitsPerReal == 32U)
    {
        inf = rtGetInfF();
    }
    else
    {
        union
        {
            LittleEndianIEEEDouble bitVal;
            real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0x7FF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        inf = tmpVal.fltVal;
    }

    return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetInfF(void)
{
    IEEESingle infF;
    infF.wordL.wordLuint = 0x7F800000U;
    return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetMinusInf(void)
{
    size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
    real_T minf = 0.0;
    if (bitsPerReal == 32U)
    {
        minf = rtGetMinusInfF();
    }
    else
    {
        union
        {
            LittleEndianIEEEDouble bitVal;
            real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0xFFF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        minf = tmpVal.fltVal;
    }

    return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetMinusInfF(void)
{
    IEEESingle minfF;
    minfF.wordL.wordLuint = 0xFF800000U;
    return minfF.wordL.wordLreal;
}

/* Function for MATLAB Function: '<S2>/SNED to VNED' */
static void quaternionBase_rotmat(real_T q_a, real_T q_b, real_T q_c, real_T q_d,
                                  real_T r[9])
{
    real_T aasq;
    real_T ac2;
    real_T ad2;
    real_T bc2;
    real_T bd2;
    real_T cd2;
    real_T n;
    n = sqrt(((q_a * q_a + q_b * q_b) + q_c * q_c) + q_d * q_d);
    q_a /= n;
    q_b /= n;
    q_c /= n;
    q_d /= n;
    n = q_a * q_b * 2.0;
    ac2 = q_a * q_c * 2.0;
    ad2 = q_a * q_d * 2.0;
    bc2 = q_b * q_c * 2.0;
    bd2 = q_b * q_d * 2.0;
    cd2 = q_c * q_d * 2.0;
    aasq = q_a * q_a * 2.0 - 1.0;
    r[0] = q_b * q_b * 2.0 + aasq;
    r[3] = bc2 + ad2;
    r[6] = bd2 - ac2;
    r[1] = bc2 - ad2;
    r[4] = q_c * q_c * 2.0 + aasq;
    r[7] = cd2 + n;
    r[2] = bd2 + ac2;
    r[5] = cd2 - n;
    r[8] = q_d * q_d * 2.0 + aasq;
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void BasicEKF_repairQuaternion(const insfilterAsync *obj, real_T x[28])
{
    real_T n;
    real_T n_tmp;
    real_T n_tmp_0;
    real_T n_tmp_1;
    real_T qparts_idx_0;
    qparts_idx_0 = x[(int32_T)obj->OrientationIdx[0] - 1];
    n_tmp = x[(int32_T)obj->OrientationIdx[1] - 1];
    n_tmp_0 = x[(int32_T)obj->OrientationIdx[2] - 1];
    n_tmp_1 = x[(int32_T)obj->OrientationIdx[3] - 1];
    n = sqrt(((qparts_idx_0 * qparts_idx_0 + n_tmp * n_tmp) + n_tmp_0 * n_tmp_0) +
             n_tmp_1 * n_tmp_1);
    qparts_idx_0 /= n;
    if (qparts_idx_0 < 0.0)
    {
        x[(int32_T)obj->OrientationIdx[0] - 1] = -qparts_idx_0;
        x[(int32_T)obj->OrientationIdx[1] - 1] = -(n_tmp / n);
        x[(int32_T)obj->OrientationIdx[2] - 1] = -(n_tmp_0 / n);
        x[(int32_T)obj->OrientationIdx[3] - 1] = -(n_tmp_1 / n);
    }
    else
    {
        x[(int32_T)obj->OrientationIdx[0] - 1] = qparts_idx_0;
        x[(int32_T)obj->OrientationIdx[1] - 1] = n_tmp / n;
        x[(int32_T)obj->OrientationIdx[2] - 1] = n_tmp_0 / n;
        x[(int32_T)obj->OrientationIdx[3] - 1] = n_tmp_1 / n;
    }
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void AsyncMARGGPSFuserBase_predict(insfilterAsync *obj, real_T dt, DW_SFS *localDW)
{
    real_T v[28];
    real_T dfdx_tmp;
    real_T dfdx_tmp_0;
    real_T dfdx_tmp_1;
    real_T dfdx_tmp_2;
    real_T dfdx_tmp_3;
    real_T dfdx_tmp_4;
    int32_T i;
    int32_T i_0;
    int32_T j;
    static const int8_T c[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T d[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T e[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T f[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T g[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T h[28] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    v[0] = obj->QuaternionNoise[0];
    v[1] = obj->QuaternionNoise[1];
    v[2] = obj->QuaternionNoise[2];
    v[3] = obj->QuaternionNoise[3];
    v[4] = obj->AngularVelocityNoise[0];
    v[7] = obj->PositionNoise[0];
    v[10] = obj->VelocityNoise[0];
    v[13] = obj->AccelerationNoise[0];
    v[16] = obj->AccelerometerBiasNoise[0];
    v[19] = obj->GyroscopeBiasNoise[0];
    v[22] = obj->GeomagneticVectorNoise[0];
    v[25] = obj->MagnetometerBiasNoise[0];
    v[5] = obj->AngularVelocityNoise[1];
    v[8] = obj->PositionNoise[1];
    v[11] = obj->VelocityNoise[1];
    v[14] = obj->AccelerationNoise[1];
    v[17] = obj->AccelerometerBiasNoise[1];
    v[20] = obj->GyroscopeBiasNoise[1];
    v[23] = obj->GeomagneticVectorNoise[1];
    v[26] = obj->MagnetometerBiasNoise[1];
    v[6] = obj->AngularVelocityNoise[2];
    v[9] = obj->PositionNoise[2];
    v[12] = obj->VelocityNoise[2];
    v[15] = obj->AccelerationNoise[2];
    v[18] = obj->AccelerometerBiasNoise[2];
    v[21] = obj->GyroscopeBiasNoise[2];
    v[24] = obj->GeomagneticVectorNoise[2];
    v[27] = obj->MagnetometerBiasNoise[2];
    memset(&localDW->procNoise[0], 0, 784U * sizeof(real_T));
    localDW->Pdot[0] = 0.0;
    dfdx_tmp_3 = -obj->pState[4] / 2.0;
    localDW->Pdot[28] = dfdx_tmp_3;
    dfdx_tmp = -obj->pState[5] / 2.0;
    localDW->Pdot[56] = dfdx_tmp;
    dfdx_tmp_1 = -obj->pState[6] / 2.0;
    localDW->Pdot[84] = dfdx_tmp_1;
    dfdx_tmp_2 = -obj->pState[1] / 2.0;
    localDW->Pdot[112] = dfdx_tmp_2;
    dfdx_tmp_4 = -obj->pState[2] / 2.0;
    localDW->Pdot[140] = dfdx_tmp_4;
    dfdx_tmp_0 = -obj->pState[3] / 2.0;
    localDW->Pdot[168] = dfdx_tmp_0;
    localDW->Pdot[196] = 0.0;
    localDW->Pdot[224] = 0.0;
    localDW->Pdot[252] = 0.0;
    localDW->Pdot[280] = 0.0;
    localDW->Pdot[308] = 0.0;
    localDW->Pdot[336] = 0.0;
    localDW->Pdot[364] = 0.0;
    localDW->Pdot[392] = 0.0;
    localDW->Pdot[420] = 0.0;
    localDW->Pdot[448] = 0.0;
    localDW->Pdot[476] = 0.0;
    localDW->Pdot[504] = 0.0;
    localDW->Pdot[532] = 0.0;
    localDW->Pdot[560] = 0.0;
    localDW->Pdot[588] = 0.0;
    localDW->Pdot[616] = 0.0;
    localDW->Pdot[644] = 0.0;
    localDW->Pdot[672] = 0.0;
    localDW->Pdot[700] = 0.0;
    localDW->Pdot[728] = 0.0;
    localDW->Pdot[756] = 0.0;
    localDW->Pdot[1] = obj->pState[4] / 2.0;
    localDW->Pdot[29] = 0.0;
    localDW->Pdot[57] = obj->pState[6] / 2.0;
    localDW->Pdot[85] = dfdx_tmp;
    localDW->Pdot[113] = obj->pState[0] / 2.0;
    localDW->Pdot[141] = dfdx_tmp_0;
    localDW->Pdot[169] = obj->pState[2] / 2.0;
    localDW->Pdot[197] = 0.0;
    localDW->Pdot[225] = 0.0;
    localDW->Pdot[253] = 0.0;
    localDW->Pdot[281] = 0.0;
    localDW->Pdot[309] = 0.0;
    localDW->Pdot[337] = 0.0;
    localDW->Pdot[365] = 0.0;
    localDW->Pdot[393] = 0.0;
    localDW->Pdot[421] = 0.0;
    localDW->Pdot[449] = 0.0;
    localDW->Pdot[477] = 0.0;
    localDW->Pdot[505] = 0.0;
    localDW->Pdot[533] = 0.0;
    localDW->Pdot[561] = 0.0;
    localDW->Pdot[589] = 0.0;
    localDW->Pdot[617] = 0.0;
    localDW->Pdot[645] = 0.0;
    localDW->Pdot[673] = 0.0;
    localDW->Pdot[701] = 0.0;
    localDW->Pdot[729] = 0.0;
    localDW->Pdot[757] = 0.0;
    localDW->Pdot[2] = obj->pState[5] / 2.0;
    localDW->Pdot[30] = dfdx_tmp_1;
    localDW->Pdot[58] = 0.0;
    localDW->Pdot[86] = obj->pState[4] / 2.0;
    localDW->Pdot[114] = obj->pState[3] / 2.0;
    localDW->Pdot[142] = obj->pState[0] / 2.0;
    localDW->Pdot[170] = dfdx_tmp_2;
    localDW->Pdot[198] = 0.0;
    localDW->Pdot[226] = 0.0;
    localDW->Pdot[254] = 0.0;
    localDW->Pdot[282] = 0.0;
    localDW->Pdot[310] = 0.0;
    localDW->Pdot[338] = 0.0;
    localDW->Pdot[366] = 0.0;
    localDW->Pdot[394] = 0.0;
    localDW->Pdot[422] = 0.0;
    localDW->Pdot[450] = 0.0;
    localDW->Pdot[478] = 0.0;
    localDW->Pdot[506] = 0.0;
    localDW->Pdot[534] = 0.0;
    localDW->Pdot[562] = 0.0;
    localDW->Pdot[590] = 0.0;
    localDW->Pdot[618] = 0.0;
    localDW->Pdot[646] = 0.0;
    localDW->Pdot[674] = 0.0;
    localDW->Pdot[702] = 0.0;
    localDW->Pdot[730] = 0.0;
    localDW->Pdot[758] = 0.0;
    localDW->Pdot[3] = obj->pState[6] / 2.0;
    localDW->Pdot[31] = obj->pState[5] / 2.0;
    localDW->Pdot[59] = dfdx_tmp_3;
    localDW->Pdot[87] = 0.0;
    localDW->Pdot[115] = dfdx_tmp_4;
    localDW->Pdot[143] = obj->pState[1] / 2.0;
    localDW->Pdot[171] = obj->pState[0] / 2.0;
    localDW->Pdot[199] = 0.0;
    localDW->Pdot[227] = 0.0;
    localDW->Pdot[255] = 0.0;
    localDW->Pdot[283] = 0.0;
    localDW->Pdot[311] = 0.0;
    localDW->Pdot[339] = 0.0;
    localDW->Pdot[367] = 0.0;
    localDW->Pdot[395] = 0.0;
    localDW->Pdot[423] = 0.0;
    localDW->Pdot[451] = 0.0;
    localDW->Pdot[479] = 0.0;
    localDW->Pdot[507] = 0.0;
    localDW->Pdot[535] = 0.0;
    localDW->Pdot[563] = 0.0;
    localDW->Pdot[591] = 0.0;
    localDW->Pdot[619] = 0.0;
    localDW->Pdot[647] = 0.0;
    localDW->Pdot[675] = 0.0;
    localDW->Pdot[703] = 0.0;
    localDW->Pdot[731] = 0.0;
    localDW->Pdot[759] = 0.0;
    for (j = 0; j < 28; j++)
    {
        localDW->procNoise[j + 28 * j] = v[j];
        localDW->Pdot[28 * j + 4] = 0.0;
        localDW->Pdot[28 * j + 5] = 0.0;
        localDW->Pdot[28 * j + 6] = 0.0;
        localDW->Pdot[28 * j + 7] = c[j];
        localDW->Pdot[28 * j + 8] = d[j];
        localDW->Pdot[28 * j + 9] = e[j];
        localDW->Pdot[28 * j + 10] = f[j];
        localDW->Pdot[28 * j + 11] = g[j];
        localDW->Pdot[28 * j + 12] = h[j];
        localDW->Pdot[28 * j + 13] = 0.0;
        localDW->Pdot[28 * j + 14] = 0.0;
        localDW->Pdot[28 * j + 15] = 0.0;
        localDW->Pdot[28 * j + 16] = 0.0;
        localDW->Pdot[28 * j + 17] = 0.0;
        localDW->Pdot[28 * j + 18] = 0.0;
        localDW->Pdot[28 * j + 19] = 0.0;
        localDW->Pdot[28 * j + 20] = 0.0;
        localDW->Pdot[28 * j + 21] = 0.0;
        localDW->Pdot[28 * j + 22] = 0.0;
        localDW->Pdot[28 * j + 23] = 0.0;
        localDW->Pdot[28 * j + 24] = 0.0;
        localDW->Pdot[28 * j + 25] = 0.0;
        localDW->Pdot[28 * j + 26] = 0.0;
        localDW->Pdot[28 * j + 27] = 0.0;
    }

    for (j = 0; j < 28; j++)
    {
        for (i = 0; i < 28; i++)
        {
            int32_T dfdx_tmp_5;
            dfdx_tmp_5 = 28 * i + j;
            localDW->dfdx[dfdx_tmp_5] = 0.0;
            localDW->obj[dfdx_tmp_5] = 0.0;
            for (i_0 = 0; i_0 < 28; i_0++)
            {
                int32_T dfdx_tmp_6;
                dfdx_tmp_6 = 28 * i_0 + j;
                localDW->dfdx[dfdx_tmp_5] += obj->pStateCovariance[28 * i + i_0] *
                                             localDW->Pdot[dfdx_tmp_6];
                localDW->obj[dfdx_tmp_5] += localDW->Pdot[28 * i_0 + i] *
                                            obj->pStateCovariance[dfdx_tmp_6];
            }
        }
    }

    for (j = 0; j < 784; j++)
    {
        localDW->Pdot[j] = (localDW->dfdx[j] + localDW->obj[j]) + localDW->procNoise[j];
    }

    v[0] = ((-(obj->pState[1] * obj->pState[4]) / 2.0 - obj->pState[2] *
                                                            obj->pState[5] / 2.0) -
            obj->pState[3] * obj->pState[6] / 2.0) *
               dt +
           obj->pState[0];
    v[1] = ((obj->pState[0] * obj->pState[4] / 2.0 - obj->pState[3] * obj->pState[5] / 2.0) + obj->pState[2] * obj->pState[6] / 2.0) * dt +
           obj->pState[1];
    v[2] = ((obj->pState[3] * obj->pState[4] / 2.0 + obj->pState[0] * obj->pState[5] / 2.0) - obj->pState[1] * obj->pState[6] / 2.0) * dt +
           obj->pState[2];
    v[3] = ((obj->pState[1] * obj->pState[5] / 2.0 - obj->pState[2] * obj->pState[4] / 2.0) + obj->pState[0] * obj->pState[6] / 2.0) * dt +
           obj->pState[3];
    v[4] = dt * 0.0 + obj->pState[4];
    v[5] = dt * 0.0 + obj->pState[5];
    v[6] = dt * 0.0 + obj->pState[6];
    v[7] = dt * obj->pState[10] + obj->pState[7];
    v[8] = dt * obj->pState[11] + obj->pState[8];
    v[9] = dt * obj->pState[12] + obj->pState[9];
    v[10] = dt * obj->pState[13] + obj->pState[10];
    v[11] = dt * obj->pState[14] + obj->pState[11];
    v[12] = dt * obj->pState[15] + obj->pState[12];
    v[13] = dt * 0.0 + obj->pState[13];
    v[14] = dt * 0.0 + obj->pState[14];
    v[15] = dt * 0.0 + obj->pState[15];
    v[16] = dt * 0.0 + obj->pState[16];
    v[17] = dt * 0.0 + obj->pState[17];
    v[18] = dt * 0.0 + obj->pState[18];
    v[19] = dt * 0.0 + obj->pState[19];
    v[20] = dt * 0.0 + obj->pState[20];
    v[21] = dt * 0.0 + obj->pState[21];
    v[22] = dt * 0.0 + obj->pState[22];
    v[23] = dt * 0.0 + obj->pState[23];
    v[24] = dt * 0.0 + obj->pState[24];
    v[25] = dt * 0.0 + obj->pState[25];
    v[26] = dt * 0.0 + obj->pState[26];
    v[27] = dt * 0.0 + obj->pState[27];
    BasicEKF_repairQuaternion(obj, v);
    for (j = 0; j < 28; j++)
    {
        for (i = 0; i < 28; i++)
        {
            i_0 = 28 * j + i;
            obj->pStateCovariance[i_0] += (localDW->Pdot[28 * i + j] + localDW->Pdot[i_0]) * 0.5 * dt;
        }

        obj->pState[j] = v[j];
    }
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void BasicEKF_correctEqn(const insfilterAsync *obj, real_T x[28], real_T P_0[784], const real_T h[3], const real_T H[84], const real_T z[3], const real_T R[9], real_T innov[3], real_T innovCov[9], DW_SFS *localDW)
{
    real_T B_0[84];
    real_T H_0[84];
    real_T W[84];
    real_T A[9];
    real_T a21;
    real_T maxval;
    int32_T W_tmp;
    int32_T W_tmp_0;
    int32_T W_tmp_1;
    int32_T r1;
    int32_T r2;
    int32_T r3;
    int32_T rtemp;
    for (W_tmp_0 = 0; W_tmp_0 < 3; W_tmp_0++)
    {
        for (W_tmp_1 = 0; W_tmp_1 < 28; W_tmp_1++)
        {
            W_tmp = 3 * W_tmp_1 + W_tmp_0;
            W[W_tmp_1 + 28 * W_tmp_0] = H[W_tmp];
            H_0[W_tmp] = 0.0;
            for (rtemp = 0; rtemp < 28; rtemp++)
            {
                H_0[W_tmp] += H[3 * rtemp + W_tmp_0] * P_0[28 * W_tmp_1 + rtemp];
            }
        }
    }

    for (W_tmp_0 = 0; W_tmp_0 < 3; W_tmp_0++)
    {
        for (W_tmp_1 = 0; W_tmp_1 < 3; W_tmp_1++)
        {
            maxval = 0.0;
            for (rtemp = 0; rtemp < 28; rtemp++)
            {
                maxval += H_0[3 * rtemp + W_tmp_0] * W[28 * W_tmp_1 + rtemp];
            }

            rtemp = 3 * W_tmp_1 + W_tmp_0;
            innovCov[rtemp] = R[rtemp] + maxval;
        }

        innov[W_tmp_0] = z[W_tmp_0] - h[W_tmp_0];
        for (W_tmp_1 = 0; W_tmp_1 < 28; W_tmp_1++)
        {
            W_tmp = 28 * W_tmp_0 + W_tmp_1;
            B_0[W_tmp] = 0.0;
            for (rtemp = 0; rtemp < 28; rtemp++)
            {
                B_0[W_tmp] += P_0[28 * rtemp + W_tmp_1] * W[28 * W_tmp_0 + rtemp];
            }
        }
    }

    memcpy(&A[0], &innovCov[0], 9U * sizeof(real_T));
    r1 = 0;
    r2 = 1;
    r3 = 2;
    maxval = fabs(innovCov[0]);
    a21 = fabs(innovCov[1]);
    if (a21 > maxval)
    {
        maxval = a21;
        r1 = 1;
        r2 = 0;
    }

    if (fabs(innovCov[2]) > maxval)
    {
        r1 = 2;
        r2 = 1;
        r3 = 0;
    }

    A[r2] = innovCov[r2] / innovCov[r1];
    A[r3] /= A[r1];
    A[r2 + 3] -= A[r1 + 3] * A[r2];
    A[r3 + 3] -= A[r1 + 3] * A[r3];
    A[r2 + 6] -= A[r1 + 6] * A[r2];
    A[r3 + 6] -= A[r1 + 6] * A[r3];
    if (fabs(A[r3 + 3]) > fabs(A[r2 + 3]))
    {
        rtemp = r2;
        r2 = r3;
        r3 = rtemp;
    }

    A[r3 + 3] /= A[r2 + 3];
    A[r3 + 6] -= A[r3 + 3] * A[r2 + 6];
    for (rtemp = 0; rtemp < 28; rtemp++)
    {
        W_tmp = 28 * r1 + rtemp;
        W[W_tmp] = B_0[rtemp] / A[r1];
        W_tmp_0 = 28 * r2 + rtemp;
        W[W_tmp_0] = B_0[rtemp + 28] - A[r1 + 3] * W[W_tmp];
        W_tmp_1 = 28 * r3 + rtemp;
        W[W_tmp_1] = B_0[rtemp + 56] - A[r1 + 6] * W[W_tmp];
        W[W_tmp_0] /= A[r2 + 3];
        W[W_tmp_1] -= A[r2 + 6] * W[W_tmp_0];
        W[W_tmp_1] /= A[r3 + 6];
        W[W_tmp_0] -= A[r3 + 3] * W[W_tmp_1];
        W[W_tmp] -= W[W_tmp_1] * A[r3];
        W[W_tmp] -= W[W_tmp_0] * A[r2];
        for (W_tmp_0 = 0; W_tmp_0 < 28; W_tmp_0++)
        {
            W_tmp = 28 * W_tmp_0 + rtemp;
            localDW->W[W_tmp] = 0.0;
            localDW->W[W_tmp] += H[3 * W_tmp_0] * W[rtemp];
            localDW->W[W_tmp] += H[3 * W_tmp_0 + 1] * W[rtemp + 28];
            localDW->W[W_tmp] += H[3 * W_tmp_0 + 2] * W[rtemp + 56];
        }

        for (W_tmp_0 = 0; W_tmp_0 < 28; W_tmp_0++)
        {
            maxval = 0.0;
            for (W_tmp_1 = 0; W_tmp_1 < 28; W_tmp_1++)
            {
                maxval += localDW->W[28 * W_tmp_1 + rtemp] * P_0[28 * W_tmp_0 + W_tmp_1];
            }

            W_tmp_1 = 28 * W_tmp_0 + rtemp;
            localDW->P_c[W_tmp_1] = P_0[W_tmp_1] - maxval;
        }
    }

    memcpy(&P_0[0], &localDW->P_c[0], 784U * sizeof(real_T));
    for (W_tmp_0 = 0; W_tmp_0 < 28; W_tmp_0++)
    {
        x[W_tmp_0] += (W[W_tmp_0 + 28] * innov[1] + W[W_tmp_0] * innov[0]) +
                      W[W_tmp_0 + 56] * innov[2];
    }

    BasicEKF_repairQuaternion(obj, x);
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void AsyncMARGGPSFuserBase_fuseaccel(insfilterAsync *obj, const real_T accel[3], real_T Raccel, DW_SFS *localDW)
{
    real_T tmp[84];
    real_T val[28];
    real_T measNoise[9];
    real_T resCov[9];
    real_T obj_0[3];
    real_T res[3];
    real_T obj_tmp;
    real_T obj_tmp_0;
    real_T obj_tmp_1;
    real_T obj_tmp_2;
    real_T obj_tmp_3;
    real_T obj_tmp_4;
    real_T obj_tmp_5;
    real_T obj_tmp_6;
    real_T obj_tmp_7;
    real_T obj_tmp_8;
    real_T obj_tmp_9;
    real_T obj_tmp_a;
    real_T tmp_0;
    real_T tmp_1;
    real_T tmp_2;
    real_T tmp_3;
    real_T tmp_4;
    real_T tmp_5;
    real_T tmp_6;
    real_T tmp_7;
    real_T tmp_8;
    real_T tmp_9;
    memset(&measNoise[0], 0, 9U * sizeof(real_T));
    measNoise[0] = Raccel;
    measNoise[4] = Raccel;
    measNoise[8] = Raccel;
    memcpy(&localDW->P_m[0], &obj->pStateCovariance[0], 784U * sizeof(real_T));
    memcpy(&val[0], &obj->pState[0], 28U * sizeof(real_T));
    obj_tmp = obj->pState[0] * obj->pState[0];
    obj_tmp_0 = obj->pState[1] * obj->pState[1];
    obj_tmp_1 = obj->pState[2] * obj->pState[2];
    obj_tmp_2 = obj->pState[3] * obj->pState[3];
    obj_tmp_3 = 2.0 * obj->pState[0] * obj->pState[3];
    obj_tmp_4 = 2.0 * obj->pState[1] * obj->pState[2];
    obj_tmp_8 = 2.0 * obj->pState[0] * obj->pState[2];
    obj_tmp_9 = 2.0 * obj->pState[1] * obj->pState[3];
    obj_tmp_a = obj_tmp_8 - obj_tmp_9;
    obj_0[0] = ((obj->pState[16] - (((obj_tmp + obj_tmp_0) - obj_tmp_1) -
                                    obj_tmp_2) *
                                       obj->pState[13]) +
                (obj->pState[15] - 9.81) * obj_tmp_a) -
               (obj_tmp_3 + obj_tmp_4) * obj->pState[14];
    obj_tmp_5 = obj_tmp - obj_tmp_0;
    obj_tmp_6 = 2.0 * obj->pState[0] * obj->pState[1];
    obj_tmp_7 = 2.0 * obj->pState[2] * obj->pState[3];
    obj_tmp_3 -= obj_tmp_4;
    obj_0[1] = ((obj->pState[17] - ((obj_tmp_5 + obj_tmp_1) - obj_tmp_2) *
                                       obj->pState[14]) -
                (obj_tmp_6 + obj_tmp_7) * (obj->pState[15] -
                                           9.81)) +
               obj_tmp_3 * obj->pState[13];
    obj_tmp_6 -= obj_tmp_7;
    obj_0[2] = ((obj->pState[18] - ((obj_tmp_5 - obj_tmp_1) + obj_tmp_2) *
                                       (obj->pState[15] - 9.81)) +
                obj_tmp_6 * obj->pState[14]) -
               (obj_tmp_8 + obj_tmp_9) * obj->pState[13];
    obj_tmp_8 = 2.0 * obj->pState[3] * obj->pState[14];
    obj_tmp_5 = 2.0 * obj->pState[2] * (obj->pState[15] - 9.81);
    tmp_5 = 2.0 * obj->pState[0] * obj->pState[13];
    tmp_9 = (obj_tmp_5 - obj_tmp_8) - tmp_5;
    tmp[0] = tmp_9;
    tmp_4 = (-2.0 * obj->pState[3] * (obj->pState[15] - 9.81) - 2.0 * obj->pState[2] * obj->pState[14]) - 2.0 * obj->pState[1] * obj->pState[13];
    tmp[3] = tmp_4;
    tmp_1 = 2.0 * obj->pState[1] * obj->pState[14];
    tmp_2 = 2.0 * obj->pState[0] * (obj->pState[15] - 9.81);
    tmp_3 = 2.0 * obj->pState[2] * obj->pState[13];
    tmp[6] = (tmp_2 - tmp_1) + tmp_3;
    tmp_6 = 2.0 * obj->pState[1] * (obj->pState[15] - 9.81);
    tmp_7 = 2.0 * obj->pState[0] * obj->pState[14];
    tmp_8 = 2.0 * obj->pState[3] * obj->pState[13];
    tmp_0 = (tmp_8 - tmp_7) - tmp_6;
    tmp[9] = tmp_0;
    tmp[12] = 0.0;
    tmp[15] = 0.0;
    tmp[18] = 0.0;
    tmp[21] = 0.0;
    tmp[24] = 0.0;
    tmp[27] = 0.0;
    tmp[30] = 0.0;
    tmp[33] = 0.0;
    tmp[36] = 0.0;
    tmp[39] = ((-obj_tmp - obj_tmp_0) + obj_tmp_1) + obj_tmp_2;
    tmp[42] = -2.0 * obj->pState[0] * obj->pState[3] - obj_tmp_4;
    tmp[45] = obj_tmp_a;
    tmp[48] = 1.0;
    tmp[51] = 0.0;
    tmp[54] = 0.0;
    tmp[57] = 0.0;
    tmp[60] = 0.0;
    tmp[63] = 0.0;
    tmp[66] = 0.0;
    tmp[69] = 0.0;
    tmp[72] = 0.0;
    tmp[75] = 0.0;
    tmp[78] = 0.0;
    tmp[81] = 0.0;
    tmp[1] = tmp_0;
    tmp_1 = (tmp_1 - tmp_2) - tmp_3;
    tmp[4] = tmp_1;
    tmp[7] = tmp_4;
    tmp[10] = (obj_tmp_8 - obj_tmp_5) + tmp_5;
    tmp[13] = 0.0;
    tmp[16] = 0.0;
    tmp[19] = 0.0;
    tmp[22] = 0.0;
    tmp[25] = 0.0;
    tmp[28] = 0.0;
    tmp[31] = 0.0;
    tmp[34] = 0.0;
    tmp[37] = 0.0;
    tmp[40] = obj_tmp_3;
    obj_tmp_8 = -obj_tmp + obj_tmp_0;
    tmp[43] = (obj_tmp_8 - obj_tmp_1) + obj_tmp_2;
    tmp[46] = -2.0 * obj->pState[0] * obj->pState[1] - obj_tmp_7;
    tmp[49] = 0.0;
    tmp[52] = 1.0;
    tmp[55] = 0.0;
    tmp[58] = 0.0;
    tmp[61] = 0.0;
    tmp[64] = 0.0;
    tmp[67] = 0.0;
    tmp[70] = 0.0;
    tmp[73] = 0.0;
    tmp[76] = 0.0;
    tmp[79] = 0.0;
    tmp[82] = 0.0;
    tmp[2] = tmp_1;
    tmp[5] = (tmp_6 + tmp_7) - tmp_8;
    tmp[8] = tmp_9;
    tmp[11] = tmp_4;
    tmp[14] = 0.0;
    tmp[17] = 0.0;
    tmp[20] = 0.0;
    tmp[23] = 0.0;
    tmp[26] = 0.0;
    tmp[29] = 0.0;
    tmp[32] = 0.0;
    tmp[35] = 0.0;
    tmp[38] = 0.0;
    tmp[41] = -2.0 * obj->pState[0] * obj->pState[2] - obj_tmp_9;
    tmp[44] = obj_tmp_6;
    tmp[47] = (obj_tmp_8 + obj_tmp_1) - obj_tmp_2;
    tmp[50] = 0.0;
    tmp[53] = 0.0;
    tmp[56] = 1.0;
    tmp[59] = 0.0;
    tmp[62] = 0.0;
    tmp[65] = 0.0;
    tmp[68] = 0.0;
    tmp[71] = 0.0;
    tmp[74] = 0.0;
    tmp[77] = 0.0;
    tmp[80] = 0.0;
    tmp[83] = 0.0;
    BasicEKF_correctEqn(obj, val, localDW->P_m, obj_0, tmp, accel, measNoise, res,
                        resCov, localDW);
    memcpy(&obj->pStateCovariance[0], &localDW->P_m[0], 784U * sizeof(real_T));
    memcpy(&obj->pState[0], &val[0], 28U * sizeof(real_T));
}

real_T rt_remd_snf(real_T u0, real_T u1)
{
    real_T y;
    if (rtIsNaN(u0) || rtIsNaN(u1) || rtIsInf(u0))
    {
        y = (rtNaN);
    }
    else if (rtIsInf(u1))
    {
        y = u0;
    }
    else if ((u1 != 0.0) && (u1 != trunc(u1)))
    {
        real_T q;
        q = fabs(u0 / u1);
        if (!(fabs(q - floor(q + 0.5)) > DBL_EPSILON * q))
        {
            y = 0.0 * u0;
        }
        else
        {
            y = fmod(u0, u1);
        }
    }
    else
    {
        y = fmod(u0, u1);
    }

    return y;
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void cosd(real_T *x)
{
    if (rtIsInf(*x) || rtIsNaN(*x))
    {
        *x = (rtNaN);
    }
    else
    {
        real_T absx;
        real_T b_x;
        int8_T n;
        b_x = rt_remd_snf(*x, 360.0);
        absx = fabs(b_x);
        if (absx > 180.0)
        {
            if (b_x > 0.0)
            {
                b_x -= 360.0;
            }
            else
            {
                b_x += 360.0;
            }

            absx = fabs(b_x);
        }

        if (absx <= 45.0)
        {
            b_x *= 0.017453292519943295;
            n = 0;
        }
        else if (absx <= 135.0)
        {
            if (b_x > 0.0)
            {
                b_x = (b_x - 90.0) * 0.017453292519943295;
                n = 1;
            }
            else
            {
                b_x = (b_x + 90.0) * 0.017453292519943295;
                n = -1;
            }
        }
        else if (b_x > 0.0)
        {
            b_x = (b_x - 180.0) * 0.017453292519943295;
            n = 2;
        }
        else
        {
            b_x = (b_x + 180.0) * 0.017453292519943295;
            n = -2;
        }

        switch (n)
        {
        case 0:
            *x = cos(b_x);
            break;

        case 1:
            *x = -sin(b_x);
            break;

        case -1:
            *x = sin(b_x);
            break;

        default:
            *x = -cos(b_x);
            break;
        }
    }
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void sind(real_T *x)
{
    if (rtIsInf(*x) || rtIsNaN(*x))
    {
        *x = (rtNaN);
    }
    else
    {
        real_T absx;
        real_T b_x;
        int8_T n;
        b_x = rt_remd_snf(*x, 360.0);
        absx = fabs(b_x);
        if (absx > 180.0)
        {
            if (b_x > 0.0)
            {
                b_x -= 360.0;
            }
            else
            {
                b_x += 360.0;
            }

            absx = fabs(b_x);
        }

        if (absx <= 45.0)
        {
            b_x *= 0.017453292519943295;
            n = 0;
        }
        else if (absx <= 135.0)
        {
            if (b_x > 0.0)
            {
                b_x = (b_x - 90.0) * 0.017453292519943295;
                n = 1;
            }
            else
            {
                b_x = (b_x + 90.0) * 0.017453292519943295;
                n = -1;
            }
        }
        else if (b_x > 0.0)
        {
            b_x = (b_x - 180.0) * 0.017453292519943295;
            n = 2;
        }
        else
        {
            b_x = (b_x + 180.0) * 0.017453292519943295;
            n = -2;
        }

        switch (n)
        {
        case 0:
            *x = sin(b_x);
            break;

        case 1:
            *x = cos(b_x);
            break;

        case -1:
            *x = -cos(b_x);
            break;

        default:
            *x = -sin(b_x);
            break;
        }
    }
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void lla2ecef(const real_T llaPos[3], real_T ecefPos[3])
{
    real_T N;
    real_T c;
    real_T d;
    real_T e;
    real_T sinphi;
    sinphi = llaPos[0];
    sind(&sinphi);
    N = 6.378137E+6 / sqrt(1.0 - sinphi * sinphi * 0.0066943799901413165);
    c = llaPos[0];
    cosd(&c);
    c *= N + llaPos[2];
    d = llaPos[1];
    cosd(&d);
    e = llaPos[1];
    sind(&e);
    ecefPos[0] = c * d;
    ecefPos[1] = c * e;
    ecefPos[2] = (N * 0.99330562000985867 + llaPos[2]) * sinphi;
}

/* Function for MATLAB Function: '<S2>/fusion' */
static void AsyncMARGGPSFuserBase_fusegps(insfilterAsync *obj, const real_T lla[3], real_T Rpos, const real_T vel[3], real_T Rvel, DW_SFS *localDW)
{
    real_T A_tmp[168];
    real_T X_0[168];
    real_T A[36];
    real_T R[36];
    real_T val[28];
    real_T varargin_1[9];
    real_T varargin_2[9];
    real_T obj_0[6];
    real_T pos[6];
    real_T pos_0[6];
    real_T tmp_0[3];
    real_T tmp_1[3];
    real_T coslambda;
    real_T cosphi;
    real_T ecefPosWithENUOrigin_idx_0;
    real_T ecefPosWithENUOrigin_idx_1;
    real_T ecefPosWithENUOrigin_idx_2;
    real_T sinlambda;
    real_T sinphi;
    int32_T b_ix;
    int32_T i;
    int32_T ix;
    int32_T iy;
    int32_T j;
    int32_T jj;
    int32_T jp;
    int8_T ipiv[6];
    static const int8_T f[168] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const int8_T H[168] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                                  0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const int8_T *A_tmp_0;
    const int8_T *X_tmp;
    memset(&varargin_1[0], 0, 9U * sizeof(real_T));
    varargin_1[0] = Rpos;
    varargin_1[4] = Rpos;
    varargin_1[8] = Rpos;
    memset(&varargin_2[0], 0, 9U * sizeof(real_T));
    varargin_2[0] = Rvel;
    varargin_2[4] = Rvel;
    varargin_2[8] = Rvel;
    memset(&R[0], 0, 36U * sizeof(real_T));
    for (j = 0; j < 3; j++)
    {
        R[6 * j] = varargin_1[3 * j];
        i = (j + 3) * 6;
        R[i + 3] = varargin_2[3 * j];
        jj = 3 * j + 1;
        R[6 * j + 1] = varargin_1[jj];
        R[i + 4] = varargin_2[jj];
        jj = 3 * j + 2;
        R[6 * j + 2] = varargin_1[jj];
        R[i + 5] = varargin_2[jj];
    }

    real_T tmp;
    cosphi = obj->ReferenceLocation[0];
    cosd(&cosphi);
    sinphi = obj->ReferenceLocation[0];
    sind(&sinphi);
    coslambda = obj->ReferenceLocation[1];
    cosd(&coslambda);
    sinlambda = obj->ReferenceLocation[1];
    sind(&sinlambda);
    lla2ecef(lla, tmp_0);
    lla2ecef(obj->ReferenceLocation, tmp_1);
    ecefPosWithENUOrigin_idx_0 = tmp_0[0] - tmp_1[0];
    ecefPosWithENUOrigin_idx_1 = tmp_0[1] - tmp_1[1];
    ecefPosWithENUOrigin_idx_2 = tmp_0[2] - tmp_1[2];
    tmp = coslambda * ecefPosWithENUOrigin_idx_0 + sinlambda *
                                                       ecefPosWithENUOrigin_idx_1;
    pos[0] = -sinphi * tmp + cosphi * ecefPosWithENUOrigin_idx_2;
    ecefPosWithENUOrigin_idx_2 = cosphi * tmp + sinphi *
                                                    ecefPosWithENUOrigin_idx_2;
    X_tmp = &f[0];
    for (j = 0; j < 6; j++)
    {
        for (jj = 0; jj < 28; jj++)
        {
            i = 28 * j + jj;
            X_0[i] = 0.0;
            for (iy = 0; iy < 28; iy++)
            {
                X_0[i] += obj->pStateCovariance[28 * iy + jj] * (real_T)X_tmp[28 * j +
                                                                              iy];
            }
        }
    }

    A_tmp_0 = &H[0];
    for (j = 0; j < 6; j++)
    {
        for (jj = 0; jj < 28; jj++)
        {
            i = 6 * jj + j;
            A_tmp[i] = 0.0;
            for (iy = 0; iy < 28; iy++)
            {
                A_tmp[i] += (real_T)A_tmp_0[6 * iy + j] * obj->pStateCovariance[28 * jj + iy];
            }
        }

        for (jj = 0; jj < 6; jj++)
        {
            cosphi = 0.0;
            for (iy = 0; iy < 28; iy++)
            {
                cosphi += A_tmp[6 * iy + j] * (real_T)X_tmp[28 * jj + iy];
            }

            i = 6 * jj + j;
            A[i] = R[i] + cosphi;
        }

        ipiv[j] = (int8_T)(j + 1);
    }

    for (j = 0; j < 5; j++)
    {
        jj = j * 7;
        iy = 6 - j;
        b_ix = 0;
        cosphi = fabs(A[jj]);
        for (jp = 2; jp <= iy; jp++)
        {
            sinphi = fabs(A[(jj + jp) - 1]);
            if (sinphi > cosphi)
            {
                b_ix = jp - 1;
                cosphi = sinphi;
            }
        }

        if (A[jj + b_ix] != 0.0)
        {
            if (b_ix != 0)
            {
                iy = j + b_ix;
                ipiv[j] = (int8_T)(iy + 1);
                b_ix = j;
                for (jp = 0; jp < 6; jp++)
                {
                    cosphi = A[b_ix];
                    A[b_ix] = A[iy];
                    A[iy] = cosphi;
                    b_ix += 6;
                    iy += 6;
                }
            }

            iy = (jj - j) + 6;
            for (jp = jj + 2; jp <= iy; jp++)
            {
                A[jp - 1] /= A[jj];
            }
        }

        b_ix = 4 - j;
        ix = jj + 8;
        for (jp = 0; jp <= b_ix; jp++)
        {
            cosphi = A[(jp * 6 + jj) + 6];
            if (cosphi != 0.0)
            {
                i = (ix - j) + 4;
                for (iy = ix; iy <= i; iy++)
                {
                    A[iy - 1] += A[((jj + iy) - ix) + 1] * -cosphi;
                }
            }

            ix += 6;
        }
    }

    for (j = 0; j < 6; j++)
    {
        jj = 28 * j;
        b_ix = 6 * j;
        for (jp = 0; jp < j; jp++)
        {
            ix = 28 * jp;
            cosphi = A[jp + b_ix];
            if (cosphi != 0.0)
            {
                for (iy = 0; iy < 28; iy++)
                {
                    i = iy + jj;
                    X_0[i] -= X_0[iy + ix] * cosphi;
                }
            }
        }

        cosphi = 1.0 / A[j + b_ix];
        for (jp = 0; jp < 28; jp++)
        {
            i = jp + jj;
            X_0[i] *= cosphi;
        }
    }

    for (j = 5; j >= 0; j--)
    {
        iy = 28 * j;
        b_ix = 6 * j - 1;
        for (jj = j + 2; jj < 7; jj++)
        {
            ix = (jj - 1) * 28;
            if (A[jj + b_ix] != 0.0)
            {
                for (jp = 0; jp < 28; jp++)
                {
                    i = jp + iy;
                    X_0[i] -= A[jj + b_ix] * X_0[jp + ix];
                }
            }
        }
    }

    for (j = 4; j >= 0; j--)
    {
        int8_T ipiv_0;
        ipiv_0 = ipiv[j];
        if (j + 1 != ipiv_0)
        {
            for (jj = 0; jj < 28; jj++)
            {
                iy = 28 * j + jj;
                cosphi = X_0[iy];
                i = (ipiv_0 - 1) * 28 + jj;
                X_0[iy] = X_0[i];
                X_0[i] = cosphi;
            }
        }
    }

    pos[3] = vel[0];
    pos[1] = -sinlambda * ecefPosWithENUOrigin_idx_0 + coslambda *
                                                           ecefPosWithENUOrigin_idx_1;
    pos[4] = vel[1];
    pos[2] = -ecefPosWithENUOrigin_idx_2;
    pos[5] = vel[2];
    obj_0[0] = obj->pState[7];
    obj_0[1] = obj->pState[8];
    obj_0[2] = obj->pState[9];
    obj_0[3] = obj->pState[10];
    obj_0[4] = obj->pState[11];
    obj_0[5] = obj->pState[12];
    for (j = 0; j < 6; j++)
    {
        pos_0[j] = pos[j] - obj_0[j];
    }

    for (j = 0; j < 28; j++)
    {
        cosphi = 0.0;
        for (jj = 0; jj < 6; jj++)
        {
            cosphi += X_0[28 * jj + j] * pos_0[jj];
        }

        val[j] = obj->pState[j] + cosphi;
    }

    BasicEKF_repairQuaternion(obj, val);
    for (j = 0; j < 28; j++)
    {
        for (jj = 0; jj < 28; jj++)
        {
            i = 28 * jj + j;
            localDW->X_c[i] = 0.0;
            for (iy = 0; iy < 6; iy++)
            {
                localDW->X_c[i] += X_0[28 * iy + j] * (real_T)A_tmp_0[6 * jj + iy];
            }
        }

        for (jj = 0; jj < 28; jj++)
        {
            cosphi = 0.0;
            for (iy = 0; iy < 28; iy++)
            {
                cosphi += localDW->X_c[28 * iy + j] * obj->pStateCovariance[28 * jj + iy];
            }

            i = 28 * jj + j;
            localDW->obj_k[i] = obj->pStateCovariance[i] - cosphi;
        }
    }

    memcpy(&obj->pStateCovariance[0], &localDW->obj_k[0], 784U * sizeof(real_T));
    memcpy(&obj->pState[0], &val[0], 28U * sizeof(real_T));
}

/* System initialize for atomic system: '<Root>/SFS' */
static void SFS_Init(DW_SFS *localDW)
{
    /* InitializeConditions for UnitDelay: '<S2>/Unit Delay3' */
    memcpy(&localDW->UnitDelay3_DSTATE[0], &rtConstP.UnitDelay3_InitialCondition[0],
           784U * sizeof(real_T));

    /* InitializeConditions for UnitDelay: '<S2>/Unit Delay2' */
    memcpy(&localDW->UnitDelay2_DSTATE[0], &rtConstP.UnitDelay2_InitialCondition[0],
           28U * sizeof(real_T));
}

/* Output and update for atomic system: '<Root>/SFS' */
static void SFS_d(const real_T rtu_sensor_raw[3], const real_T rtu_sensor_raw_p[3], const real_T rtu_sensor_raw_j[3], const real_T rtu_sensor_raw_d[3], const real_T rtu_sensor_raw_h[3], real_T rty_sensor_fused[3], real_T rty_sensor_fused_p[4], real_T rty_sensor_fused_j[3], real_T rty_sensor_fused_d[3], real_T rty_sensor_fused_h[3], real_T rty_sensor_fused_pa[3], real_T rty_sensor_fused_g[3], real_T rty_sensor_fused_c[3], real_T rty_sensor_fused_f[3], DW_SFS *localDW)
{
    real_T tmp[84];
    real_T xk[28];
    real_T IMUNED2VNED[9];
    real_T measNoise[9];
    real_T IMUNED2VNED_0[3];
    real_T filter[3];
    real_T res[3];
    real_T IMUNED2VNED_tmp;
    real_T aasq;
    real_T ab2;
    real_T ac2;
    real_T ad2;
    real_T bc2;
    real_T bd2;
    real_T cd2;
    real_T n;
    real_T q;
    real_T xk_tmp_1;
    int32_T i;
    static const real_T c[84] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    /* MATLAB Function: '<S2>/SNED to VNED' */
    n = (rtP.phi - 180.0) * 0.017453292519943295 / 2.0;
    IMUNED2VNED_tmp = cos(n);
    q = sin(n);
    quaternionBase_rotmat(IMUNED2VNED_tmp - 0.0 * q * 0.0, IMUNED2VNED_tmp * 0.0 + sin(n) * 0.0, q - cos(n) * 0.0 * 0.0, cos(n) * 0.0 + 0.0 * sin(n), IMUNED2VNED);
    n = (180.0 - rtP.phi) * 0.017453292519943295 / 2.0;

    /* MATLAB Function: '<S2>/fusion' incorporates:
     *  UnitDelay: '<S2>/Unit Delay2'
     *  UnitDelay: '<S2>/Unit Delay3'
     */
    memcpy(&xk[0], &localDW->UnitDelay2_DSTATE[0], 28U * sizeof(real_T));
    localDW->filter.OrientationIdx[0] = 1.0;
    localDW->filter.OrientationIdx[1] = 2.0;
    localDW->filter.OrientationIdx[2] = 3.0;
    localDW->filter.OrientationIdx[3] = 4.0;
    localDW->filter.ReferenceLocation[0] = rtP.location_lla_IC[0];
    localDW->filter.ReferenceLocation[1] = rtP.location_lla_IC[1];
    localDW->filter.ReferenceLocation[2] = rtP.location_lla_IC[2];
    xk[24] = localDW->UnitDelay2_DSTATE[24];
    xk[23] = localDW->UnitDelay2_DSTATE[23];
    xk[22] = localDW->UnitDelay2_DSTATE[22];
    memcpy(&localDW->filter.pState[0], &xk[0], 28U * sizeof(real_T));
    memcpy(&localDW->filter.pStateCovariance[0], &localDW->UnitDelay3_DSTATE[0],
           784U * sizeof(real_T));
    localDW->filter.QuaternionNoise[0] = rtP.noise_state[0];
    localDW->filter.QuaternionNoise[1] = rtP.noise_state[1];
    localDW->filter.QuaternionNoise[2] = rtP.noise_state[2];
    localDW->filter.QuaternionNoise[3] = rtP.noise_state[3];
    localDW->filter.AngularVelocityNoise[0] = rtP.noise_state[4];
    localDW->filter.PositionNoise[0] = rtP.noise_state[7];
    localDW->filter.VelocityNoise[0] = rtP.noise_state[10];
    localDW->filter.AccelerationNoise[0] = rtP.noise_state[13];
    localDW->filter.GyroscopeBiasNoise[0] = rtP.noise_state[16];
    localDW->filter.AccelerometerBiasNoise[0] = rtP.noise_state[19];
    localDW->filter.GeomagneticVectorNoise[0] = rtP.noise_state[22];
    localDW->filter.MagnetometerBiasNoise[0] = rtP.noise_state[25];
    localDW->filter.AngularVelocityNoise[1] = rtP.noise_state[5];
    localDW->filter.PositionNoise[1] = rtP.noise_state[8];
    localDW->filter.VelocityNoise[1] = rtP.noise_state[11];
    localDW->filter.AccelerationNoise[1] = rtP.noise_state[14];
    localDW->filter.GyroscopeBiasNoise[1] = rtP.noise_state[17];
    localDW->filter.AccelerometerBiasNoise[1] = rtP.noise_state[20];
    localDW->filter.GeomagneticVectorNoise[1] = rtP.noise_state[23];
    localDW->filter.MagnetometerBiasNoise[1] = rtP.noise_state[26];
    localDW->filter.AngularVelocityNoise[2] = rtP.noise_state[6];
    localDW->filter.PositionNoise[2] = rtP.noise_state[9];
    localDW->filter.VelocityNoise[2] = rtP.noise_state[12];
    localDW->filter.AccelerationNoise[2] = rtP.noise_state[15];
    localDW->filter.GyroscopeBiasNoise[2] = rtP.noise_state[18];
    localDW->filter.AccelerometerBiasNoise[2] = rtP.noise_state[21];
    localDW->filter.GeomagneticVectorNoise[2] = rtP.noise_state[24];
    localDW->filter.MagnetometerBiasNoise[2] = rtP.noise_state[27];
    AsyncMARGGPSFuserBase_predict(&localDW->filter, rtP.fusion_t, localDW);

    /* MATLAB Function: '<S2>/SNED to VNED' */
    for (i = 0; i < 3; i++)
    {
        IMUNED2VNED_0[i] = (IMUNED2VNED[i + 3] * rtu_sensor_raw[1] + IMUNED2VNED[i] *
                                                                         rtu_sensor_raw[0]) +
                           IMUNED2VNED[i + 6] *
                               rtu_sensor_raw[2];
    }

    /* MATLAB Function: '<S2>/fusion' */
    AsyncMARGGPSFuserBase_fuseaccel(&localDW->filter, IMUNED2VNED_0, rtP.covA,
                                    localDW);
    memset(&measNoise[0], 0, 9U * sizeof(real_T));
    measNoise[0] = rtP.covG;
    measNoise[4] = rtP.covG;
    measNoise[8] = rtP.covG;
    memcpy(&xk[0], &localDW->filter.pState[0], 28U * sizeof(real_T));
    xk[24] = localDW->filter.pState[24];
    xk[23] = localDW->filter.pState[23];
    xk[22] = localDW->filter.pState[22];
    memcpy(&localDW->val[0], &localDW->filter.pStateCovariance[0], 784U * sizeof(real_T));
    filter[0] = localDW->filter.pState[4] + localDW->filter.pState[19];
    filter[1] = localDW->filter.pState[5] + localDW->filter.pState[20];
    filter[2] = localDW->filter.pState[6] + localDW->filter.pState[21];

    /* MATLAB Function: '<S2>/SNED to VNED' */
    for (i = 0; i < 3; i++)
    {
        IMUNED2VNED_0[i] = (IMUNED2VNED[i + 3] * rtu_sensor_raw_p[1] + IMUNED2VNED[i] * rtu_sensor_raw_p[0]) + IMUNED2VNED[i + 6] *
                                                                                                                   rtu_sensor_raw_p[2];
    }

    real_T tmp_0;
    real_T tmp_1;
    real_T tmp_2;
    real_T tmp_3;
    real_T tmp_4;
    real_T tmp_5;
    real_T tmp_6;
    real_T tmp_7;
    real_T tmp_8;
    real_T tmp_9;
    real_T xk_tmp;
    real_T xk_tmp_0;
    real_T xk_tmp_2;

    /* MATLAB Function: '<S2>/fusion' */
    BasicEKF_correctEqn(&localDW->filter, xk, localDW->val, filter, c,
                        IMUNED2VNED_0, measNoise, res, IMUNED2VNED, localDW);
    memcpy(&localDW->filter.pStateCovariance[0], &localDW->val[0], 784U * sizeof(real_T));
    memcpy(&localDW->filter.pState[0], &xk[0], 28U * sizeof(real_T));
    memset(&measNoise[0], 0, 9U * sizeof(real_T));

    /* MATLAB Function: '<S2>/SNED to VNED' */
    xk_tmp = sin(n);
    xk_tmp_0 = cos(n);
    tmp_0 = 6.123233995736766E-17 * xk_tmp;
    tmp_1 = 6.123233995736766E-17 * xk_tmp_0;
    quaternionBase_rotmat(tmp_1 - xk_tmp * 0.0, tmp_0 * 0.0 + xk_tmp_0, tmp_0 - xk_tmp_0 * 0.0, tmp_1 * 0.0 + xk_tmp, IMUNED2VNED);

    /* MATLAB Function: '<S2>/fusion' incorporates:
     *  MATLAB Function: '<S2>/SNED to VNED'
     *  UnitDelay: '<S2>/Unit Delay1'
     */
    IMUNED2VNED_tmp = xk[0] * xk[0];
    q = xk[1] * xk[1];
    xk_tmp = xk[2] * xk[2];
    xk_tmp_0 = xk[3] * xk[3];
    n = 2.0 * xk[0] * xk[3];
    aasq = 2.0 * xk[1] * xk[2];
    xk_tmp_1 = 2.0 * xk[0] * xk[2];
    ab2 = 2.0 * xk[1] * xk[3];
    ac2 = ((IMUNED2VNED_tmp + q) - xk_tmp) - xk_tmp_0;
    ad2 = n + aasq;
    IMUNED2VNED_0[0] = ((ac2 * xk[22] + xk[25]) - (xk_tmp_1 - ab2) * xk[24]) + ad2 * xk[23];
    IMUNED2VNED_tmp -= q;
    q = 2.0 * xk[0] * xk[1];
    bc2 = 2.0 * xk[2] * xk[3];
    bd2 = (IMUNED2VNED_tmp + xk_tmp) - xk_tmp_0;
    cd2 = q + bc2;
    IMUNED2VNED_0[1] = ((bd2 * xk[23] + xk[26]) + cd2 * xk[24]) - (n - aasq) * xk
                                                                                   [22];
    xk_tmp_2 = xk_tmp_1 + ab2;
    IMUNED2VNED_tmp = (IMUNED2VNED_tmp - xk_tmp) + xk_tmp_0;
    IMUNED2VNED_0[2] = ((IMUNED2VNED_tmp * xk[24] + xk[27]) - (q - bc2) * xk[23]) + xk_tmp_2 * xk[22];
    xk_tmp = 2.0 * xk[24] * xk[2];
    xk_tmp_0 = 2.0 * xk[23] * xk[3];
    tmp_0 = 2.0 * xk[22] * xk[0];
    tmp_1 = (xk_tmp_0 - xk_tmp) + tmp_0;
    tmp[0] = tmp_1;
    tmp_6 = (2.0 * xk[24] * xk[3] + 2.0 * xk[23] * xk[2]) + 2.0 * xk[22] * xk[1];
    tmp[3] = tmp_6;
    tmp_3 = 2.0 * xk[24] * xk[0];
    tmp_4 = 2.0 * xk[23] * xk[1];
    tmp_5 = 2.0 * xk[22] * xk[2];
    tmp[6] = (tmp_4 - tmp_3) - tmp_5;
    tmp_7 = 2.0 * xk[22] * xk[3];
    tmp_8 = 2.0 * xk[23] * xk[0];
    tmp_9 = 2.0 * xk[24] * xk[1];
    tmp_2 = (tmp_9 + tmp_8) - tmp_7;
    tmp[9] = tmp_2;
    tmp[12] = 0.0;
    tmp[15] = 0.0;
    tmp[18] = 0.0;
    tmp[21] = 0.0;
    tmp[24] = 0.0;
    tmp[27] = 0.0;
    tmp[30] = 0.0;
    tmp[33] = 0.0;
    tmp[36] = 0.0;
    tmp[39] = 0.0;
    tmp[42] = 0.0;
    tmp[45] = 0.0;
    tmp[48] = 0.0;
    tmp[51] = 0.0;
    tmp[54] = 0.0;
    tmp[57] = 0.0;
    tmp[60] = 0.0;
    tmp[63] = 0.0;
    tmp[66] = ac2;
    tmp[69] = ad2;
    tmp[72] = ab2 - xk_tmp_1;
    tmp[75] = 1.0;
    tmp[78] = 0.0;
    tmp[81] = 0.0;
    tmp[1] = tmp_2;
    tmp_3 = (tmp_3 - tmp_4) + tmp_5;
    tmp[4] = tmp_3;
    tmp[7] = tmp_6;
    tmp[10] = (xk_tmp - xk_tmp_0) - tmp_0;
    tmp[13] = 0.0;
    tmp[16] = 0.0;
    tmp[19] = 0.0;
    tmp[22] = 0.0;
    tmp[25] = 0.0;
    tmp[28] = 0.0;
    tmp[31] = 0.0;
    tmp[34] = 0.0;
    tmp[37] = 0.0;
    tmp[40] = 0.0;
    tmp[43] = 0.0;
    tmp[46] = 0.0;
    tmp[49] = 0.0;
    tmp[52] = 0.0;
    tmp[55] = 0.0;
    tmp[58] = 0.0;
    tmp[61] = 0.0;
    tmp[64] = 0.0;
    tmp[67] = aasq - n;
    tmp[70] = bd2;
    tmp[73] = cd2;
    tmp[76] = 0.0;
    tmp[79] = 1.0;
    tmp[82] = 0.0;
    tmp[2] = tmp_3;
    tmp[5] = (tmp_7 - tmp_8) - tmp_9;
    tmp[8] = tmp_1;
    tmp[11] = tmp_6;
    tmp[14] = 0.0;
    tmp[17] = 0.0;
    tmp[20] = 0.0;
    tmp[23] = 0.0;
    tmp[26] = 0.0;
    tmp[29] = 0.0;
    tmp[32] = 0.0;
    tmp[35] = 0.0;
    tmp[38] = 0.0;
    tmp[41] = 0.0;
    tmp[44] = 0.0;
    tmp[47] = 0.0;
    tmp[50] = 0.0;
    tmp[53] = 0.0;
    tmp[56] = 0.0;
    tmp[59] = 0.0;
    tmp[62] = 0.0;
    tmp[65] = 0.0;
    tmp[68] = xk_tmp_2;
    tmp[71] = bc2 - q;
    tmp[74] = IMUNED2VNED_tmp;
    tmp[77] = 0.0;
    tmp[80] = 0.0;
    tmp[83] = 1.0;
    for (i = 0; i < 3; i++)
    {
        measNoise[i + 3 * i] = rtP.covM;
        filter[i] = (IMUNED2VNED[i + 3] * rtu_sensor_raw_j[1] + IMUNED2VNED[i] *
                                                                    rtu_sensor_raw_j[0]) +
                    IMUNED2VNED[i + 6] * rtu_sensor_raw_j[2];
    }

    BasicEKF_correctEqn(&localDW->filter, xk, localDW->val, IMUNED2VNED_0, tmp,
                        filter, measNoise, res, IMUNED2VNED, localDW);
    memcpy(&localDW->filter.pStateCovariance[0], &localDW->val[0], 784U * sizeof(real_T));
    memcpy(&localDW->filter.pState[0], &xk[0], 28U * sizeof(real_T));
    if (!(localDW->UnitDelay1_DSTATE != 0.0))
    {
        AsyncMARGGPSFuserBase_fusegps(&localDW->filter, rtu_sensor_raw_h, rtP.covP,
                                      rtu_sensor_raw_d, rtP.covV, localDW);
    }

    IMUNED2VNED_tmp = localDW->UnitDelay1_DSTATE + 1.0;
    if (rtP.gps_ratio == 0.0)
    {
        if (localDW->UnitDelay1_DSTATE + 1.0 == 0.0)
        {
            IMUNED2VNED_tmp = rtP.gps_ratio;
        }
    }
    else if (rtIsNaN(localDW->UnitDelay1_DSTATE + 1.0))
    {
        IMUNED2VNED_tmp = (rtNaN);
    }
    else if (rtIsNaN(rtP.gps_ratio))
    {
        IMUNED2VNED_tmp = (rtNaN);
    }
    else if (rtIsInf(localDW->UnitDelay1_DSTATE + 1.0))
    {
        IMUNED2VNED_tmp = (rtNaN);
    }
    else if (localDW->UnitDelay1_DSTATE + 1.0 == 0.0)
    {
        IMUNED2VNED_tmp = 0.0 / rtP.gps_ratio;
    }
    else if (rtIsInf(rtP.gps_ratio))
    {
        if ((localDW->UnitDelay1_DSTATE + 1.0 < 0.0) != (rtP.gps_ratio < 0.0))
        {
            IMUNED2VNED_tmp = rtP.gps_ratio;
        }
    }
    else
    {
        boolean_T rEQ0;
        IMUNED2VNED_tmp = fmod(localDW->UnitDelay1_DSTATE + 1.0, rtP.gps_ratio);
        rEQ0 = (IMUNED2VNED_tmp == 0.0);
        if ((!rEQ0) && (rtP.gps_ratio > floor(rtP.gps_ratio)))
        {
            q = fabs((localDW->UnitDelay1_DSTATE + 1.0) / rtP.gps_ratio);
            rEQ0 = !(fabs(q - floor(q + 0.5)) > 2.2204460492503131E-16 * q);
        }

        if (rEQ0)
        {
            IMUNED2VNED_tmp = rtP.gps_ratio * 0.0;
        }
        else if ((localDW->UnitDelay1_DSTATE + 1.0 < 0.0) != (rtP.gps_ratio < 0.0))
        {
            IMUNED2VNED_tmp += rtP.gps_ratio;
        }
    }

    memcpy(&xk[0], &localDW->filter.pState[0], 28U * sizeof(real_T));
    n = sqrt(((xk[0] * xk[0] + xk[1] * xk[1]) + xk[2] * xk[2]) + xk[3] * xk[3]);
    aasq = xk[0] / n;
    q = xk[1] / n;
    xk_tmp_1 = xk[2] / n;
    n = xk[3] / n;
    ab2 = aasq * q * 2.0;
    ac2 = aasq * xk_tmp_1 * 2.0;
    ad2 = aasq * n * 2.0;
    bc2 = q * xk_tmp_1 * 2.0;
    bd2 = q * n * 2.0;
    cd2 = xk_tmp_1 * n * 2.0;
    aasq = aasq * aasq * 2.0 - 1.0;
    IMUNED2VNED[0] = q * q * 2.0 + aasq;
    IMUNED2VNED[3] = bc2 + ad2;
    IMUNED2VNED[6] = bd2 - ac2;
    IMUNED2VNED[1] = bc2 - ad2;
    IMUNED2VNED[4] = xk_tmp_1 * xk_tmp_1 * 2.0 + aasq;
    IMUNED2VNED[7] = cd2 + ab2;
    IMUNED2VNED[2] = bd2 + ac2;
    IMUNED2VNED[5] = cd2 - ab2;
    IMUNED2VNED[8] = n * n * 2.0 + aasq;

    /* Math: '<S2>/Transpose11' incorporates:
     *  MATLAB Function: '<S2>/NED2VNED'
     */
    rty_sensor_fused_p[0] = localDW->filter.pState[0];
    rty_sensor_fused_p[1] = localDW->filter.pState[1];
    rty_sensor_fused_p[2] = localDW->filter.pState[2];
    rty_sensor_fused_p[3] = localDW->filter.pState[3];
    for (i = 0; i < 3; i++)
    {
        /* Math: '<S2>/Transpose10' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused[i] = localDW->filter.pState[i + 4];

        /* Math: '<S2>/Transpose3' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_f[i] = localDW->filter.pState[i + 25];

        /* Math: '<S2>/Transpose4' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_c[i] = 0.0;

        /* Math: '<S2>/Transpose5' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_d[i] = 0.0;

        /* Math: '<S2>/Transpose6' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_h[i] = 0.0;

        /* Math: '<S2>/Transpose7' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_pa[i] = localDW->filter.pState[i + 22];

        /* Math: '<S2>/Transpose8' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_g[i] = 0.0;

        /* MATLAB Function: '<S2>/NED2VNED' */
        q = IMUNED2VNED[i];

        /* Math: '<S2>/Transpose4' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_c[i] += q * localDW->filter.pState[19];

        /* Math: '<S2>/Transpose5' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_d[i] += q * localDW->filter.pState[10];

        /* Math: '<S2>/Transpose6' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_h[i] += q * localDW->filter.pState[13];

        /* Math: '<S2>/Transpose8' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_g[i] += q * localDW->filter.pState[16];

        /* MATLAB Function: '<S2>/NED2VNED' */
        q = IMUNED2VNED[i + 3];

        /* Math: '<S2>/Transpose4' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_c[i] += q * localDW->filter.pState[20];

        /* Math: '<S2>/Transpose5' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_d[i] += q * localDW->filter.pState[11];

        /* Math: '<S2>/Transpose6' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_h[i] += q * localDW->filter.pState[14];

        /* Math: '<S2>/Transpose8' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_g[i] += q * localDW->filter.pState[17];

        /* MATLAB Function: '<S2>/NED2VNED' */
        q = IMUNED2VNED[i + 6];

        /* Math: '<S2>/Transpose4' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_c[i] += q * localDW->filter.pState[21];

        /* Math: '<S2>/Transpose5' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_d[i] += q * localDW->filter.pState[12];

        /* Math: '<S2>/Transpose6' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_h[i] += q * localDW->filter.pState[15];

        /* Math: '<S2>/Transpose8' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_g[i] += q * localDW->filter.pState[18];

        /* Math: '<S2>/Transpose9' incorporates:
         *  MATLAB Function: '<S2>/NED2VNED'
         */
        rty_sensor_fused_j[i] = localDW->filter.pState[i + 7];
    }

    /* Update for UnitDelay: '<S2>/Unit Delay3' incorporates:
     *  MATLAB Function: '<S2>/fusion'
     */
    memcpy(&localDW->UnitDelay3_DSTATE[0], &localDW->filter.pStateCovariance[0],
           784U * sizeof(real_T));

    /* Update for UnitDelay: '<S2>/Unit Delay1' incorporates:
     *  MATLAB Function: '<S2>/fusion'
     */
    localDW->UnitDelay1_DSTATE = IMUNED2VNED_tmp;

    /* Update for UnitDelay: '<S2>/Unit Delay2' */
    memcpy(&localDW->UnitDelay2_DSTATE[0], &localDW->filter.pState[0], 28U * sizeof(real_T));
}

/* Model step function */
void SFS_step(RT_MODEL *const rtM, ExtU *rtU, ExtY *rtY)
{
    DW *rtDW = rtM->dwork;

    /* Outputs for Atomic SubSystem: '<Root>/SFS' */
    SFS_d(rtU->acc, rtU->gyro, rtU->mag, rtU->vel, rtU->pos, rtY->angvel_VNED,
          rtY->ang_NED, rtY->pos_VNED, rtY->vel_VNED, rtY->acc_VNED, rtY->mag_VNED,
          rtY->acc_bias_VNED, rtY->gyro_bias_VNED, rtY->mag_bias_VNED,
          &rtDW->SFS_ds);

    /* End of Outputs for SubSystem: '<Root>/SFS' */
}

/* Model initialize function */
void SFS_initialize(RT_MODEL *const rtM)
{
    DW *rtDW = rtM->dwork;

    /* Registration code */

    /* initialize non-finites */
    rt_InitInfAndNaN(sizeof(real_T));

    /* SystemInitialize for Atomic SubSystem: '<Root>/SFS' */
    SFS_Init(&rtDW->SFS_ds);

    /* End of SystemInitialize for SubSystem: '<Root>/SFS' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */