#include "em_pp.h"
#include "em.h"
#include "can_parse.h"
#include "common_defs.h"

void em_pp(ExtU_em *rtU_tm, ExtY_tv *rtY_tv)
{
    /* Torque Vectoring Input */
    // rtU_tm->rTVS[0] = rtY_tv->rTVS[0];
    // rtU_tm->rTVS[1] = rtY_tv->rTVS[1];

    // rtU_tm->rEQUAL[0] = rtY_tv->rEQUAL[0];
    // rtU_tm->rEQUAL[1] = rtY_tv->rEQUAL[1];

    // rtU_tm->TVS_STATE = rtY_tv->TVS_STATE;

    // rtU_tm->w[0] = rtY_tv->w[0];
    // rtU_tm->w[1] = rtY_tv->w[1];

    // rtU_tm->V = rtY_tv->V;

    /* Throttle Map Input */
    rtU_tm->rTVS[0] = (can_data.filt_throttle_brake.throttle/4095.0);
    rtU_tm->rTVS[1] = (can_data.filt_throttle_brake.throttle/4095.0);

    rtU_tm->rEQUAL[0] = (can_data.filt_throttle_brake.throttle/4095.0);
    rtU_tm->rEQUAL[1] = (can_data.filt_throttle_brake.throttle/4095.0);

    rtU_tm->TVS_STATE = can_data.dashboard_tv_parameters.tv_enabled;

    rtU_tm->w[0] = (can_data.rear_wheel_speeds.left_speed_sensor*8.75*0.01);
    rtU_tm->w[1] = (can_data.rear_wheel_speeds.right_speed_sensor*8.75*0.01);

    rtU_tm->V = (can_data.orion_currents_volts.pack_current*0.1);
}