#include "tv_pp.h"
#include "tv.h"
#include "can_parse.h"
#include "common_defs.h"

void tv_pp(ExtU_tv *rtU_tv, GPS_Handle_t *GPS)
{
    /* Flags */
    rtU_tv->F_raw[0] = can_data.main_hb.car_state == 4;
    rtU_tv->F_raw[1] = !can_data.main_hb.stale;
    rtU_tv->F_raw[2] = !can_data.filt_throttle_brake.stale;
    rtU_tv->F_raw[3] = !can_data.LWS_Standard.stale;
    rtU_tv->F_raw[4] = !can_data.rear_wheel_speeds.stale;
    rtU_tv->F_raw[5] = !can_data.orion_currents_volts.stale;
    rtU_tv->F_raw[6] = can_data.LWS_Standard.Ok;
    rtU_tv->F_raw[7] = GPS->gyro_OK;
    rtU_tv->F_raw[8] = GPS->fix_type == 3;
    rtU_tv->F_raw[9] = 1.0;
    rtU_tv->F_raw[10] = 1.0;
    rtU_tv->F_raw[11] = 1.0;

    /* Raw Data */
    rtU_tv->D_raw[0] = (can_data.filt_throttle_brake.throttle/4095.0); /* Incoming is a scalar in the range [0 4095] */
    rtU_tv->D_raw[1] = (can_data.LWS_Standard.LWS_ANGLE*0.1); /* Incoming is degree of CCSA  */
    rtU_tv->D_raw[2] = (can_data.orion_currents_volts.pack_voltage*0.1); /* Incoming is V of terminal*/
    rtU_tv->D_raw[3] = (can_data.rear_wheel_speeds.left_speed_sensor*0.01*8.75); /* Incoming is rad/s of tire */
    rtU_tv->D_raw[4] = (can_data.rear_wheel_speeds.right_speed_sensor*0.01*8.75); /* Incoming is rad/s of tire */
    rtU_tv->D_raw[5] = (GPS->n_vel*0.001); /* Incoming data is mm/s, uint16_t */
    rtU_tv->D_raw[6] = (GPS->e_vel*0.001); /* Incoming data is mm/s */
    rtU_tv->D_raw[7] = (GPS->d_vel*0.001); /* Incoming data is mm/s */
    rtU_tv->D_raw[8] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    rtU_tv->D_raw[9] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    rtU_tv->D_raw[10] = (GPS->gyroscope.z); /* Incoming data is rad/s */
    rtU_tv->D_raw[11] = (can_data.orion_currents_volts.pack_current*0.1); /* Incoming is A out of battery */
    rtU_tv->D_raw[12] = (can_data.rear_controller_temps.left_temp); /* Incoming is degree C */
    rtU_tv->D_raw[13] = (can_data.rear_controller_temps.right_temp); /* Incoming is degree C */
    rtU_tv->D_raw[14] = (can_data.rear_motor_currents_temps.left_temp); /* Incoming is degree C */
    rtU_tv->D_raw[15] = (can_data.rear_motor_currents_temps.right_temp); /* Incoming is degree C */
    rtU_tv->D_raw[16] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    rtU_tv->D_raw[17] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    rtU_tv->D_raw[18] = (GPS->acceleration.z); /* Incoming data is m/s^2 */

    /* Driver Tunable Parameters */
    /* If Can is up to date, update driver parameters */
    if (!can_data.dashboard_tv_parameters.stale) {
        rtU_tv->dphi = can_data.dashboard_tv_parameters.tv_deadband_val;
        rtU_tv->TVS_P = can_data.dashboard_tv_parameters.tv_p_val;
        rtU_tv->TVS_I = can_data.dashboard_tv_parameters.tv_intensity_val;
        rtU_tv->F_raw[12] = can_data.dashboard_tv_parameters.tv_enabled;
    }
}

void tv_IO_initialize(ExtU_tv *rtU_tv)
{
    /* Raw Data */
    rtU_tv->D_raw[0] = 0.0;
    rtU_tv->D_raw[1] = 0.0;
    rtU_tv->D_raw[2] = 310.0;
    rtU_tv->D_raw[3] = 1.0;
    rtU_tv->D_raw[4] = 1.0;
    rtU_tv->D_raw[5] = 0.0;
    rtU_tv->D_raw[6] = 0.0;
    rtU_tv->D_raw[7] = 0.0;
    rtU_tv->D_raw[8] = 0.0;
    rtU_tv->D_raw[9] = 0.0;
    rtU_tv->D_raw[10] = 0.0;
    rtU_tv->D_raw[11] = 0.0;
    rtU_tv->D_raw[12] = 20.0;
    rtU_tv->D_raw[13] = 20.0;
    rtU_tv->D_raw[14] = 20.0;
    rtU_tv->D_raw[15] = 20.0;
    rtU_tv->D_raw[16] = 0.0;
    rtU_tv->D_raw[17] = 0.0;
    rtU_tv->D_raw[18] = 9.81;

    /* Flags */
    rtU_tv->F_raw[0] = 1.0;
    rtU_tv->F_raw[1] = 1.0;
    rtU_tv->F_raw[2] = 1.0;
    rtU_tv->F_raw[3] = 1.0;
    rtU_tv->F_raw[4] = 1.0;
    rtU_tv->F_raw[5] = 1.0;
    rtU_tv->F_raw[6] = 1.0;
    rtU_tv->F_raw[7] = 1.0;
    rtU_tv->F_raw[8] = 1.0;
    rtU_tv->F_raw[9] = 1.0;
    rtU_tv->F_raw[10] = 1.0;
    rtU_tv->F_raw[11] = 1.0;
    rtU_tv->F_raw[12] = 1.0;

    /* Driver Tunable Parameters */
    rtU_tv->dphi = 12.0;
    rtU_tv->TVS_P = 1.0;
    rtU_tv->TVS_I = 1.0;

    /* Sensor Transformation */
    /* Fill Column by Column*/
    rtU_tv->R[0] = 1.0;
    rtU_tv->R[1] = 0.0;
    rtU_tv->R[2] = 0.0;
    rtU_tv->R[3] = 0.0;
    rtU_tv->R[4] = 1.0;
    rtU_tv->R[5] = 0.0;
    rtU_tv->R[6] = 0.0;
    rtU_tv->R[7] = 0.0;
    rtU_tv->R[8] = 1.0;
}