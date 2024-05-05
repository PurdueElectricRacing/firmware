#include "tv_pp.h"
#include "tv.h"
#include "can_parse.h"
#include "common_defs.h"

void tv_pp(ExtU_tv *rtU_tv, GPS_Handle_t *GPS)
{
    /* Flags */
    rtU_tv->F_raw[0] = (can_data.main_hb.stale == 0);
    rtU_tv->F_raw[1] = (can_data.filt_throttle_brake.stale == 0);
    rtU_tv->F_raw[2] = (can_data.LWS_Standard.stale == 0);
    rtU_tv->F_raw[3] = (can_data.rear_wheel_speeds.stale == 0);
    rtU_tv->F_raw[4] = (can_data.orion_currents_volts.stale == 0);
    rtU_tv->F_raw[5] = (can_data.max_cell_temp.stale == 0);
    rtU_tv->F_raw[6] = (can_data.LWS_Standard.Ok == 1);
    rtU_tv->F_raw[7] = (GPS->gyro_OK == 1);
    rtU_tv->F_raw[8] = (GPS->fix_type == 3);
    rtU_tv->F_raw[9] = true;
    rtU_tv->F_raw[10] = true;
    rtU_tv->F_raw[11] = true;

    /* Raw Data */
    rtU_tv->D_raw[0] = (can_data.filt_throttle_brake.throttle/4095.0); /* Incoming is a scalar in the range [0 4095] */
    rtU_tv->D_raw[1] = (can_data.LWS_Standard.LWS_ANGLE*0.1); /* Incoming is 10*degree of CCSA  */
    rtU_tv->D_raw[2] = (can_data.orion_currents_volts.pack_voltage*0.1); /* Incoming is 10*V of terminal*/
    rtU_tv->D_raw[3] = (can_data.rear_wheel_speeds.left_speed_sensor*0.01*8.75); /* Incoming is 100*rad/s of tire */
    rtU_tv->D_raw[4] = (can_data.rear_wheel_speeds.right_speed_sensor*0.01*8.75); /* Incoming is 100*rad/s of tire */
    rtU_tv->D_raw[5] = (GPS->g_speed*0.001); /* Incoming data is 1000*m/s */
    rtU_tv->D_raw[6] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    rtU_tv->D_raw[7] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    rtU_tv->D_raw[8] = (GPS->gyroscope.z); /* Incoming data is rad/s */
    rtU_tv->D_raw[9] = (can_data.orion_currents_volts.pack_current*0.1); /* Incoming is 10*A out of battery */
    rtU_tv->D_raw[10] = 40.0; /* Incoming is degree C */
    rtU_tv->D_raw[11] = 40.0; /* Incoming is degree C */
    // rtU_tv->D_raw[10] = MAX((can_data.rear_motor_temps.left_ctrl_temp),(can_data.rear_motor_temps.right_ctrl_temp)); /* Incoming is degree C */
    // rtU_tv->D_raw[11] = MAX((can_data.rear_motor_temps.left_mot_temp), (can_data.rear_motor_temps.right_mot_temp)); /* Incoming is degree C */
    rtU_tv->D_raw[12] = (can_data.max_cell_temp.max_temp*0.1); /* Incoming is deg C */
    rtU_tv->D_raw[13] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    rtU_tv->D_raw[14] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    rtU_tv->D_raw[15] = (GPS->acceleration.z); /* Incoming data is m/s^2 */

    /* Driver Tunable Parameters */
    /* If Can is up to date, update driver parameters */
    if (!can_data.dashboard_tv_parameters.stale) {
        rtU_tv->dphi = can_data.dashboard_tv_parameters.tv_deadband_val;
        rtU_tv->TVS_P = can_data.dashboard_tv_parameters.tv_p_val;
        rtU_tv->TVS_I = can_data.dashboard_tv_parameters.tv_intensity_val;
        rtU_tv->F_raw[12] = (can_data.dashboard_tv_parameters.tv_enabled == 1);
    }

    // /* Flags */
    // rtU_tv->F_raw[0] = true;
    // rtU_tv->F_raw[1] = true;
    // rtU_tv->F_raw[2] = true;
    // rtU_tv->F_raw[3] = true;
    // rtU_tv->F_raw[4] = true;
    // rtU_tv->F_raw[5] = true;
    // rtU_tv->F_raw[6] = true;
    // rtU_tv->F_raw[7] = true;
    // rtU_tv->F_raw[8] = true;
    // rtU_tv->F_raw[9] = true;
    // rtU_tv->F_raw[10] = true;
    // rtU_tv->F_raw[11] = true;

    // /* Raw Data */
    // rtU_tv->D_raw[0] = 500.0/4095.0; /* Incoming is a scalar in the range [0 4095] */
    // rtU_tv->D_raw[1] = 50; /* Incoming is degree of CCSA  */
    // rtU_tv->D_raw[2] = 300; /* Incoming is V of terminal*/
    // rtU_tv->D_raw[3] = 437.5; /* Incoming is rad/s of tire */
    // rtU_tv->D_raw[4] = 437.5; /* Incoming is rad/s of tire */
    // rtU_tv->D_raw[5] = 10.0; /* Incoming data is mm/s */
    // rtU_tv->D_raw[6] = 0.0; /* Incoming data is mm/s */
    // rtU_tv->D_raw[7] = 0.0; /* Incoming data is mm/s */
    // rtU_tv->D_raw[8] = (GPS->gyroscope.x);  /* Incoming data is rad/s */
    // rtU_tv->D_raw[9] = (GPS->gyroscope.y);  /* Incoming data is rad/s */
    // rtU_tv->D_raw[10] = (GPS->gyroscope.z); /* Incoming data is rad/s */
    // rtU_tv->D_raw[11] = 50; /* Incoming is 10*A out of battery */
    // rtU_tv->D_raw[12] = 40; /* Incoming is degree C */
    // rtU_tv->D_raw[13] = 40; /* Incoming is degree C */
    // rtU_tv->D_raw[14] = 40; /* Incoming is degree C */
    // rtU_tv->D_raw[15] = 40; /* Incoming is degree C */
    // rtU_tv->D_raw[16] = (GPS->acceleration.x); /* Incoming data is m/s^2 */
    // rtU_tv->D_raw[17] = (GPS->acceleration.y); /* Incoming data is m/s^2 */
    // rtU_tv->D_raw[18] = (GPS->acceleration.z); /* Incoming data is m/s^2 */

    // /* Driver Tunable Parameters */
    // /* If Can is up to date, update driver parameters */
    // if (true) {
    //     rtU_tv->dphi = 12;
    //     rtU_tv->TVS_P = 0.5;
    //     rtU_tv->TVS_I = 1.0;
    //     rtU_tv->F_raw[12] = true;
    // }
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
    rtU_tv->D_raw[10] = 20.0;
    rtU_tv->D_raw[11] = 20.0;
    rtU_tv->D_raw[12] = 20.0;
    rtU_tv->D_raw[13] = 0.0;
    rtU_tv->D_raw[14] = 0.0;
    rtU_tv->D_raw[15] = 9.81;

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