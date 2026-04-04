
// VCU structs
typedef struct {
    float TH_RAW;
    float ST_RAW;
    float VB_RAW;
    float WM_RAW[4];
    float GS_RAW;
    float AV_RAW[3];
    float IB_RAW;
    float MT_RAW;
    float IGBT_T_RAW;
    float INV_T_RAW;
    float MC_RAW;
    float IC_RAW;
    float BT_RAW;
    float TO_RAW[4];
} xVCU_struct;

typedef struct {
    float TH;
    float TH_PO;
    float TH_RG;
    float ST;
    float VB;
    float WM[4];
    float GS;
    float AV[3];
    float IB;
    float MT;
    float IGBT_T;
    float INV_T;
    float MC;
    float IC;
    float BT;
    float TO[4];
    float PB;
    float TO_BL_PO[4];
    float TORQUE_OUT[4];
} yVCU_struct;

typedef struct {
    float r;
    float ht[2];
    float wb;
    float gr;
    float MAX_TO_ABS;
    float PB_derating_full_T;
    float PB_derating_half_T;
    float PB_derating_FR;
    float INV_T_derating_full_T;
    float INV_derating_zero_T;
    float IGBT_derating_full_T;
    float IGBT_derating_zero_T;
    float MT_derating_full_T;
    float MT_derating_zero_T;
    float BT_derating_full_T;
    float BT_derating_zero_T;
    float VB_derating_full_T;
    float VB_derating_zero_T;
    float IB_derating_full_T;
    float IB_derating_zero_T;
} pVCU_struct;