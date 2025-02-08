#include "can_parse.h"
#include "common_defs.h"
#include "vcu_pp.h"
#include "gps.h"
 
void vcu_pp(xVCU_struct *xVCU, GPS_Handle_t *GPS)
{
    /*Raw X Data*/
    xVCU->TH_RAW = (can_data.filt_throttle_brake.throttle/4095.0); /* Incoming is a scalar in the range [0 4095] */
    xVCU->ST_RAW = (can_data.LWS_Standard.LWS_ANGLE*0.1); /* Incoming is 10*degree of CCSA  */
    xVCU->VB_RAW = (can_data.orion_currents_volts.pack_voltage*0.1); /* Incoming is 10*V of terminal*/
    xVCU->WT_RAW[1] = (can_data.rear_wheel_speeds.left_speed_sensor*0.01*8.75); /* Incoming is 100*rad/s of tire */
    xVCU->WT_RAW[2] = (can_data.rear_wheel_speeds.right_speed_sensor*0.01*8.75); /* Incoming is 100*rad/s of tire */
    xVCU->WM_RAW[1] = (can_data.rear_wheel_speeds.left_speed_mc*0.01*8.75); /*Incoming is 100*rad/s of tire*/
    xVCU->WM_RAW[2] = (can_data.rear_wheel_speeds.right_speed_mc*0.01*8.75); /*Incoming is 100*rad/s of tire*/
    xVCU->GS_RAW = (GPS->g_speed*0.001); /* Incoming data is 1000*m/s */
    xVCU->AV_RAW[1] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    xVCU->AV_RAW[2] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    xVCU->AV_RAW[3] = (GPS->gyroscope.z);  /* Incoming data is rad/s */
    xVCU->IB_RAW = (can_data.orion_currents_volts.pack_current*0.1); /* Incoming is 10*A out of battery */
    xVCU->MT_RAW = (MAX((can_data.rear_motor_temps.left_mot_temp),(can_data.rear_motor_temps.right_mot_temp))*0.1);/* incoming is 10*motor temp*/
    xVCU->CT_RAW = (MAX((can_data.rear_motor_temps.left_igbt_temp),(can_data.rear_motor_temps.right_igbt_temp))*0.1);/* incoming is 10*controller igbt temp*/
    xVCU->IT_RAW = (MAX((can_data.rear_motor_temps.left_inv_temp),(can_data.rear_motor_temps.right_inv_temp))*0.1);/* incoming is 10*inverter cold plate temp*/
    xVCU->MC_RAW = 0; /*DUMMY - torque percentage*/
    xVCU->IC_RAW = 0; /*DUMMY - torque percentage*/
    xVCU->BT_RAW = (can_data.max_cell_temp.max_temp*0.1); /* Incoming is deg C */
    xVCU->AG_RAW[1] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[2] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    xVCU->AG_RAW[3] = (GPS->acceleration.z); /* Incoming data is m/s^2 */
    xVCU->TO_RAW[1] = 0; /*DUMMY - Motor torque Left*/
    xVCU->TO_RAW[2] = 0; /*DUMMY - Motor torque right*/
    xVCU->DB_RAW = (can_data.dashboard_tv_parameters.tv_deadband_val); /*Incoming is int16 value*/
    xVCU->PI_RAW = (can_data.dashboard_tv_parameters.tv_intensity_val); /*Incoming is int16 value*/
    xVCU->PP_RAW = (can_data.dashboard_tv_parameters.tv_p_val); /*Incoming is int16 value*/


    /*Raw F Data*/
    

}