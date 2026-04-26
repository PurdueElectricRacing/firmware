#include "vcu.h"
xVCU_struct init_xVCU(void) {
    xVCU_struct xVCU = {
        .VCU_MODE_REQ = 0,
        .THROT_RAW = 0,
        .BRAKE_RAW = 0,
        .ST_RAW = 0,
        .VB_RAW = 600,
        .WM_RAW = {0, 0, 0, 0},
        .GS_RAW = 0,
        .AV_RAW = {0, 0, 0},
        .IB_RAW = 0,
        .MT_RAW = 0,
        .IGBT_T_RAW = 0,
        .INV_T_RAW = 0,
        .MC_RAW = {0, 0, 0, 0},
        .IC_RAW = {0, 0, 0, 0},
        .BT_RAW = 0,
        .TO_RAW = {0, 0, 0, 0},
        .RG_split_FR_RAW = 0.7
    };
    return xVCU;
}


yVCU_struct init_yVCU(void) {
    yVCU_struct yVCU = {
        .VCU_MODE = 0,
        .TH = 0,
        .TH_PO = 0,
        .TH_RG = 0,
        .ST = 0,
        .VB = 0,
        .WM = {0, 0, 0, 0},
        .GS = 0,
        .AV = {0, 0, 0},
        .IB = 0,
        .MT = 0,
        .IGBT_T = 0,
        .INV_T = 0,
        .OV_MOT = {0, 0, 0, 0},
        .OV_INV = {0, 0, 0, 0},
        .BT = 0,
        .TO = {0, 0, 0, 0},
        .IB_AVG_buffer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        .PB = 0,
        .WW = {0, 0, 0, 0},
        .IB_AVG = 0,
        .TO_BL_PO = {0, 0, 0, 0},
        .RG_split_FR = 0.7,
        .TO_BL_RG = {0, 0, 0, 0},
        .AC_MW = {0, 0, 0, 0},
        .SK_TO = {0, 0, 0, 0},
        .AX_TO = {0, 0, 0, 0},
        .TORQUE_LIM_NEG = {0, 0, 0, 0},
        .TORQUE_LIM_POS = {0, 0, 0, 0},
        .SPEED_OUT = {0, 0, 0, 0},
        .TORQUE_OUT = {0, 0, 0, 0}
    };
    return yVCU;
}


pVCU_struct init_pVCU(void) {
    pVCU_struct pVCU = {
        .r = 0.2,
        .ht = {0.649, 0.621},
        .wb = 2,
        .gr = 12.51,
        .MAX_ABS_WM = 3100,
        .IB_AVG_length = 10,
        .MAX_TO_ABS_PO = 21,
        .PB_derating_full_T = 75,
        .PB_derating_half_T = 80,
        .PB_derating_FR = 0.75,
        .VB_derating_full_T = 400,
        .VB_derating_zero_T = 340,
        .IB_derating_full_T = 200,
        .IB_derating_zero_T = 230,
        .OV_MOT_derating_full_T = 50,
        .OV_MOT_derating_zero_T = 100,
        .OV_INV_derating_full_T = 50,
        .OV_INV_derating_zero_T = 100,
        .MAX_TO_ABS_RG = 5,
        .VB_RG_derating_full_T = 340,
        .VB_RG_derating_zero_T = 400,
        .IB_RG_derating_full_T = -145,
        .IB_RG_derating_zero_T = -160,
        .GS_RG_derating_zero = 1.3888889,
        .GS_RG_derating_full = 2.7777778,
        .INV_T_derating_full_T = 50,
        .INV_T_derating_zero_T = 60,
        .IGBT_T_derating_full_T = 115,
        .IGBT_T_derating_zero_T = 125,
        .MT_derating_full_T = 125,
        .MT_derating_zero_T = 140,
        .BT_derating_full_T = 55,
        .BT_derating_zero_T = 60,
        .AC_speed_brkpt = {0, 1.598721, 49.5603517},
        .AC_speed_table = {22, 22, 272.5819345},
        .AC_brkpt_lb = 0,
        .AC_brkpt_ub = 49.5603517,
        .SK_YAW_des = 1.2493151,
        .SK_LR_split_des = 0.6,
        .SK_FR_split = 0.4,
        .SK_LR_gain = 1,
        .SK_ST_ZERO_TV = 10,
        .SK_ST_FULL_TV = 25
    };
    return pVCU;
}
