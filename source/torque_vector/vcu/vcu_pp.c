#include "can_parse.h"
#include "common_defs.h"
#include "vcu.h"
 
void vcu_pp(xVCU_struct *xVCU, fVCU_struct *fVCU, GPS_Handle_t *GPS)
{
    /*Raw X Data*/
    xVCU->TH_RAW = (can_data.filt_throttle_brake.throttle/4095.0); /* Incoming is a scalar in the range [0 4095] */
    xVCU->ST_RAW = (can_data.LWS_Standard.LWS_ANGLE*0.1); /* Incoming is 10*degree of CCSA  */
    xVCU->VB_RAW = (can_data.orion_currents_volts.pack_voltage*0.1); /* Incoming is 10*V of terminal*/
    xVCU->WT_RAW[1] = (can_data.rear_wheel_speeds.left_speed_sensor*0.01); /* Incoming is 100*rad/s of tire */
    xVCU->WT_RAW[2] = (can_data.rear_wheel_speeds.right_speed_sensor*0.01); /* Incoming is 100*rad/s of tire */
    xVCU->WM_RAW[1] = (can_data.actual_torque_speed.speed_left); /*Incoming is RPM of motor shaft */
    xVCU->WM_RAW[2] = (can_data.actual_torque_speed.speed_right); /*Incoming is RPM of motor shaft */
    xVCU->GS_RAW = (GPS->g_speed*0.001); /* Incoming data is 1000*m/s */
    xVCU->AV_RAW[1] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    xVCU->AV_RAW[2] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    xVCU->AV_RAW[3] = (GPS->gyroscope.z);  /* Incoming data is rad/s */
    xVCU->IB_RAW = (can_data.orion_currents_volts.pack_current*0.1); /* Incoming is 10*A out of battery */
    xVCU->MT_RAW = (MAX((can_data.rear_motor_temps.left_mot_temp),(can_data.rear_motor_temps.right_mot_temp))*0.1);/* incoming is 10*motor temp*/
    xVCU->CT_RAW = (MAX((can_data.rear_motor_temps.left_igbt_temp),(can_data.rear_motor_temps.right_igbt_temp))*0.1);/* incoming is 10*controller igbt temp*/
    xVCU->IT_RAW = (MAX((can_data.rear_motor_temps.left_inv_temp),(can_data.rear_motor_temps.right_inv_temp))*0.1);/* incoming is 10*inverter cold plate temp*/
    xVCU->MC_RAW = (MAX((can_data.INV_Overload.AMK_DisplayOverloadMotorA),(can_data.INV_Overload.AMK_DisplayOverloadMotorB))*0.1); /* incoming data is 10*% */
    xVCU->IC_RAW = (MAX((can_data.INV_Overload.AMK_DisplayOverloadInverterA),(can_data.INV_Overload.AMK_DisplayOverloadInverterB))*0.1); /* incoming data is 10*% */
    xVCU->BT_RAW = (can_data.max_cell_temp.max_temp*0.1); /* Incoming is 10*deg C */
    xVCU->AG_RAW[1] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[2] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[3] = (GPS->acceleration.z); /* Incoming data is m/s^2 */
    xVCU->TO_RAW[1] = (can_data.actual_torque_speed.torque_left)*0.1; /* incoming data is 10*Nm */
    xVCU->TO_RAW[2] = (can_data.actual_torque_speed.torque_right)*0.1; /* incoming data is 10*Nm */
    xVCU->DB_RAW = (can_data.dashboard_vcu_parameters.tv_deadband_val); /*Incoming is int16 value*/
    xVCU->PI_RAW = (can_data.dashboard_vcu_parameters.tv_intensity_val); /*Incoming is int16 value*/
    xVCU->PP_RAW = (can_data.dashboard_vcu_parameters.tv_p_val); /*Incoming is int16 value*/

    /*Raw F Data*/
    fVCU->CS_SFLAG = (can_data.main_hb.stale);
    fVCU->TB_SFLAG = (can_data.filt_throttle_brake.stale);
    fVCU->SS_SFLAG = (can_data.LWS_Standard.stale);
    fVCU->WT_SFLAG = (can_data.rear_wheel_speeds.stale);
    fVCU->IV_SFLAG = (can_data.orion_currents_volts.stale);
    fVCU->BT_SFLAG = (can_data.max_cell_temp.stale);
    fVCU->MT_SFLAG = (can_data.rear_motor_temps.stale);
    fVCU->CO_SFLAG = (can_data.INV_Overload.stale);
    fVCU->MO_SFLAG = (can_data.actual_torque_speed.stale);
    fVCU->SS_FFLAG = (can_data.LWS_Standard.Ok);
    fVCU->AV_FFLAG = (GPS->gyro_OK);
    fVCU->GS_FFLAG = (GPS->fix_type);
    fVCU->VCU_PFLAG = (can_data.dashboard_vcu_parameters.vcu_mode);
}