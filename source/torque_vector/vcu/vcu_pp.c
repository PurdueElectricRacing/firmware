#include "can_parse.h"
#include "common_defs.h"
#include "vcu.h"
#include <math.h>
 
void vcu_pp(fVCU_struct *fVCU, xVCU_struct *xVCU, GPS_Handle_t *GPS)
{
    /*Raw F Data*/
    fVCU->CS_SFLAG = (can_data.main_hb.stale);
    fVCU->TB_SFLAG = (can_data.filt_throttle_brake.stale);
    fVCU->SS_SFLAG = (can_data.LWS_Standard.stale);
    fVCU->WT_SFLAG = (can_data.rear_wheel_speeds.stale);
    fVCU->IV_SFLAG = (can_data.orion_currents_volts.stale);
    fVCU->BT_SFLAG = (can_data.max_cell_temp.stale);
    fVCU->IAC_SFLAG = (can_data.INVA_CRIT.stale);
    fVCU->IAT_SFLAG = (can_data.INVA_TEMPS.stale);
    fVCU->IBC_SFLAG = (can_data.INVB_CRIT.stale);
    fVCU->IBT_SFLAG = (can_data.INVB_TEMPS.stale);
    fVCU->SS_FFLAG = (can_data.LWS_Standard.Ok);
    fVCU->AV_FFLAG = (GPS->gyro_OK);
    fVCU->GS_FFLAG = (GPS->fix_type);
    fVCU->VCU_PFLAG = (4);
    fVCU->VCU_CFLAG = (2);

    /*Raw X Data*/
    xVCU->TH_RAW = (can_data.filt_throttle_brake.throttle/4095.0); /* Incoming is a scalar in the range [0 4095] */
    xVCU->ST_RAW = (can_data.LWS_Standard.LWS_ANGLE*0.1); /* Incoming is 10*degree of CCSA  */
    xVCU->VB_RAW = (can_data.orion_currents_volts.pack_voltage*0.1); /* Incoming is 10*V of terminal*/
    xVCU->WT_RAW[0] = 0;
    xVCU->WT_RAW[1] = 0;
    xVCU->WM_RAW[0] = (can_data.INVA_CRIT.AMK_ActualSpeed*M_PI/30); /*Incoming is RPM of motor shaft */
    xVCU->WM_RAW[1] = (can_data.INVB_CRIT.AMK_ActualSpeed*M_PI/30); /*Incoming is RPM of motor shaft */
    xVCU->GS_RAW = (GPS->g_speed*0.001); /* Incoming data is 1000*m/s */
    xVCU->AV_RAW[0] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    xVCU->AV_RAW[1] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    xVCU->AV_RAW[2] = (GPS->gyroscope.z);  /* Incoming data is rad/s */
    xVCU->IB_RAW = (can_data.orion_currents_volts.pack_current*0.1); /* Incoming is 10*A out of battery */
    xVCU->MT_RAW = (MAX((can_data.INVA_TEMPS.AMK_MotorTemp),(can_data.INVB_TEMPS.AMK_MotorTemp))*0.1);/* incoming is 10*motor temp*/
    xVCU->CT_RAW = (MAX((can_data.INVA_TEMPS.AMK_IGBTTemp),(can_data.INVB_TEMPS.AMK_IGBTTemp))*0.1);/* incoming is 10*controller igbt temp*/
    xVCU->IT_RAW = (MAX((can_data.INVA_TEMPS.AMK_InverterTemp),(can_data.INVB_TEMPS.AMK_InverterTemp))*0.1);/* incoming is 10*inverter cold plate temp*/
    xVCU->MC_RAW = (MAX((can_data.INVA_CRIT.AMK_DisplayOverloadMotor),(can_data.INVB_CRIT.AMK_DisplayOverloadMotor))*0.1); /* incoming data is 10*% */
    xVCU->IC_RAW = (MAX((can_data.INVA_CRIT.AMK_DisplayOverloadInverter),(can_data.INVB_CRIT.AMK_DisplayOverloadInverter))*0.1); /* incoming data is 10*% */
    xVCU->BT_RAW = (can_data.max_cell_temp.max_temp*0.1); /* Incoming is 10*deg C */
    xVCU->AG_RAW[0] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[1] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[2] = (GPS->acceleration.z); /* Incoming data is m/s^2 */
    xVCU->TO_RAW[0] = (can_data.INVA_CRIT.AMK_ActualTorque)*9.8/1000; /* incoming data is 10*%Mn (Mn=9.8Nm) */
    xVCU->TO_RAW[1] = (can_data.INVB_CRIT.AMK_ActualTorque)*9.8/1000; /* incoming data is 10*%Mn (Mn=9.8Nm) */
    xVCU->VT_DB_RAW = 12; /*Incoming is int8 value*/
    xVCU->TV_PP_RAW = 5; /*Incoming is 100*int8 value*/
    xVCU->TC_TR_RAW = 1; /*Incoming is 100*int8 value*/
    xVCU->VS_MAX_SR_RAW = 1; /*Incoming is 100*int8 value*/
}