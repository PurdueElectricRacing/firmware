#include "vcu.h"

xVCU_struct init_xVCU(void) {
    xVCU_struct xVCU = {
        .TH_RAW = 0,
        .ST_RAW = 0,
        .VB_RAW = 600,
        .WM_RAW = {0, 0, 0, 0},
        .GS_RAW = 0,
        .AV_RAW = {0, 0, 0},
        .IB_RAW = 0,
        .MT_RAW = 0,
        .IGBT_T_RAW = 0,
        .INV_T_RAW = 0,
        .MC_RAW = 0,
        .IC_RAW = 0,
        .BT_RAW = 0,
        .TO_RAW = {0, 0, 0, 0}
    };
}

yVCU_struct init_yVCU(void) {
    yVCU_struct yVCU = {
        .TH = 0,
        .TH_PO = 0,
        .TH_RG = 0,
        .ST = 0,
        .VB = 500,
        .WM = {0, 0, 0, 0},
        .GS = 0,
        .AV = {0, 0, 0},
        .IB = 0,
        .MT = 0,
        .IGBT_T = 0,
        .INV_T = 0,
        .MC = 0,
        .IC = 0,
        .BT = 0,
        .TO = {0, 0, 0, 0},
        .PB = 0,
        .TO_BL_PO = {0, 0, 0, 0},
        .TORQUE_OUT = {0, 0, 0, 0}
    };
}

pVCU_struct init_pVCU(void) {
    pVCU_struct pVCU = {
        .r = 0.2,
        .ht = {0.649, 0.621},
        .wb = 2,
        .gr = 12.51,
        .MAX_TO_ABS_PO = 21,
        .PB_derating_full_T = 70,
        .PB_derating_half_T = 75,
        .PB_derating_FR = 0.75,
        .INV_T_derating_full_T = 50,
        .INV_T_derating_zero_T = 60,
        .IGBT_T_derating_full_T = 115,
        .IGBT_T_derating_zero_T = 125,
        .MT_derating_full_T = 125,
        .MT_derating_zero_T = 140,
        .BT_derating_full_T = 55,
        .BT_derating_zero_T = 60,
        .VB_derating_full_T = 400,
        .VB_derating_zero_T = 340,
        .IB_derating_full_T = 200,
        .IB_derating_zero_T = 230
    };
}