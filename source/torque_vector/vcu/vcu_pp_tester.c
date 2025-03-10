#include "can_parse.h"
#include "common_defs.h"
#include "vcu.h"

// Manually sets xVCU and fVCU structs to test values
void vcu_pp_tester(fVCU_struct *fVCU, xVCU_struct *xVCU, yVCU_struct *yVCU) {
    xVCU->TH_RAW = 0.5;
    xVCU->ST_RAW = 25;
    xVCU->VB_RAW = 470;
    xVCU->WT_RAW[0] = 10;
    xVCU->WT_RAW[1] = 11;
    xVCU->WM_RAW[0] = 113.4;
    xVCU->WM_RAW[1] = 124.74;
    xVCU->GS_RAW = 2;
    xVCU->AV_RAW[0] = 0;
    xVCU->AV_RAW[1] = 0;
    xVCU->AV_RAW[2] = 9.81;
    xVCU->IB_RAW = 10;
    xVCU->MT_RAW = 31;
    xVCU->CT_RAW = 37;
    xVCU->IT_RAW = 41;
    xVCU->MC_RAW = 0;
    xVCU->IC_RAW = 0;
    xVCU->BT_RAW = 31;
    xVCU->AG_RAW[0] = 2;
    xVCU->AG_RAW[1] = 1;
    xVCU->AG_RAW[2] = 0;
    xVCU->TO_RAW[0] = 15;
    xVCU->TO_RAW[1] = 16;
    xVCU->VT_DB_RAW = 0;
    xVCU->TV_PP_RAW = 0.4;
    xVCU->TC_TR_RAW = 0.5;
    xVCU->VS_MAX_SR_RAW = 0.1;
    
    fVCU->CS_SFLAG = 0;
    fVCU->TB_SFLAG = 0;
    fVCU->SS_SFLAG = 0;
    fVCU->WT_SFLAG = 0;
    fVCU->IV_SFLAG = 0;
    fVCU->BT_SFLAG = 0;
    fVCU->IAC_SFLAG = 0;
    fVCU->IAT_SFLAG = 0;
    fVCU->IBC_SFLAG = 0;
    fVCU->IBT_SFLAG = 0;
    fVCU->SS_FFLAG = 1;
    fVCU->AV_FFLAG = 1;
    fVCU->GS_FFLAG = 3;
    fVCU->VCU_PFLAG = 2;
    fVCU->VCU_CFLAG = 2;

    yVCU->PT_permit_buffer[0] = 1;
    yVCU->PT_permit_buffer[1] = 1;
    yVCU->PT_permit_buffer[2] = 1;
    yVCU->PT_permit_buffer[3] = 1;
    yVCU->PT_permit_buffer[4] = 1;

    yVCU->VS_permit_buffer[0] = 1;
    yVCU->VS_permit_buffer[1] = 1;
    yVCU->VS_permit_buffer[2] = 1;
    yVCU->VS_permit_buffer[3] = 1;
    yVCU->VS_permit_buffer[4] = 1;

    yVCU->VT_permit_buffer[0] = 1;
    yVCU->VT_permit_buffer[1] = 1;
    yVCU->VT_permit_buffer[2] = 1;
    yVCU->VT_permit_buffer[3] = 1;
    yVCU->VT_permit_buffer[4] = 1;
}