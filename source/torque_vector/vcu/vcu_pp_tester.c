#include "can_parse.h"
#include "common_defs.h"
#include "vcu.h"

// Manually sets xVCU and fVCU structs to test values
void vcu_pp_tester(fVCU_struct* fVCU, xVCU_struct* xVCU) {
    xVCU->TH_RAW        = 0;
    xVCU->ST_RAW        = 0;
    xVCU->VB_RAW        = 0;
    xVCU->WT_RAW[0]     = 0;
    xVCU->WT_RAW[1]     = 0;
    xVCU->WM_RAW[0]     = 0;
    xVCU->WM_RAW[1]     = 0;
    xVCU->GS_RAW        = 0;
    xVCU->AV_RAW[0]     = 0;
    xVCU->AV_RAW[1]     = 0;
    xVCU->AV_RAW[2]     = 0;
    xVCU->IB_RAW        = 0;
    xVCU->MT_RAW        = 0;
    xVCU->CT_RAW        = 0;
    xVCU->IT_RAW        = 0;
    xVCU->MC_RAW        = 0;
    xVCU->IC_RAW        = 0;
    xVCU->BT_RAW        = 0;
    xVCU->AG_RAW[0]     = 0;
    xVCU->AG_RAW[1]     = 0;
    xVCU->AG_RAW[2]     = 0;
    xVCU->TO_RAW[0]     = 0;
    xVCU->TO_RAW[1]     = 0;
    xVCU->VT_DB_RAW     = 0;
    xVCU->TV_PP_RAW     = 0;
    xVCU->TC_TR_RAW     = 0;
    xVCU->VS_MAX_SR_RAW = 0;

    fVCU->CS_SFLAG  = 0;
    fVCU->TB_SFLAG  = 0;
    fVCU->SS_SFLAG  = 0;
    fVCU->WT_SFLAG  = 0;
    fVCU->IV_SFLAG  = 0;
    fVCU->BT_SFLAG  = 0;
    fVCU->IAC_SFLAG = 0;
    fVCU->IAT_SFLAG = 0;
    fVCU->IBC_SFLAG = 0;
    fVCU->IBT_SFLAG = 0;
    fVCU->SS_FFLAG  = 1;
    fVCU->AV_FFLAG  = 1;
    fVCU->GS_FFLAG  = 3;
    fVCU->VCU_PFLAG = 1;
    fVCU->VCU_CFLAG = 1;
}