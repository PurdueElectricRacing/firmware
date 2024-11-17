#include "amk.h"
#include "source/main_module/can/can_parse.h"

/* NOTE:
 * Step 1 is turning on LV
 * Step 3 happens when HV is turned on and precharging starts (I think this is actually step 2)
 *  I can check when this is done with precharge complete and then move 
 *  onto other steps
 *  MAYBE we can check when AMK_bSystemReady is on and display a message
 *  on LCD so they know when to turn on the HV to start the precharging.
 *  Not sure if I need that
 */

amk_motor_t right = {0};

static void turnMotorOn(amk_motor_t* motor);
static void motorRunning(amk_motor_t* motor);
static void turnMotorOff(amk_motor_t* motor);
static void motorGetData(amk_motor_t* motor);
static void motorReset(amk_motor_t* motor);

/* NOTE: As of now this is just setting everything to 0, but it may make sense
 * to have it in case something changes down the line while I learn more, so
 * this may end up being deleted if everything just inits to 0 */
void motorInit(amk_motor_t* motor, bool* pchg_complete)
{
    *motor = (amk_motor_t){
        /* States */
        .states.stage = MOTOR_STAGE_INIT,
        .states.init_stage = MOTOR_INIT_POWER_ON,
        .states.deinit_stage = MOTOR_DEINIT_SETPOINTS_DEINIT,
        .states.running_stage = MOTOR_RUNNING_GOOD,
        .states.reset_state = MOTOR_RESET_INVERTER_OFF,

        /* Values */
        .torque_setpoint = DEFAULT_TORQUE_SETPOINT,
        .torque_limit_positive = DEFAULT_POSITIVE_TORQUE_LIMIT,
        .torque_limit_negative = DEFAULT_NEGATIVE_TORQUE_LIMIT,

        .pchg_complete = pchg_complete
    };
}

void amkCanTesting(amk_motor_t* motor)
{
    SEND_AMK_TESTING(motor->states.init_stage, motor->control.bits, motor->status.bits, *motor->pchg_complete);
}

/* TODO: Determine period of this. Should be pretty often of course. The control
 * word needs to be sent every 50ms or the motors will shut off. Plettenberg did
 * every 15ms so maybe we will just do that?
 */
void motorPeriodic(amk_motor_t* motor)
{
    motorGetData(motor);

    amkCanTesting(motor);

    switch(motor->states.stage) {
    case MOTOR_STAGE_INIT:
        turnMotorOn(motor);
        break;
    case MOTOR_STAGE_RUNNING:
        motorRunning(motor);
        break;
    case MOTOR_STAGE_DEINIT:
        turnMotorOff(motor);
        break;
    }
}

/* Sets the torque setpoint from -1000% to 1000% of nominal torque.
 * But this function just takes in -100% to 100% to stay consistent */
void motorSetTorque(amk_motor_t* motor, int16_t torque_setpoint)
{
    if (torque_setpoint > MAX_POSITIVE_TORQUE_SETPOINT 
        || torque_setpoint < MAX_NEGATIVE_TORQUE_SETPOINT)
        return;

    /* Scale up since unit is 0.1% of nominal torque */
    torque_setpoint *= 10;

    motor->torque_setpoint = torque_setpoint;

    if (torque_setpoint < 0) {
        motor->torque_limit_negative = -100;
        motor->torque_limit_positive = 0;
    } else {
        /* FIXME: Was told 9.8Nm is nominal, and to set to limit to 17Nm for now,
         * but in the mean time I am going to go less than 100% as I am unsure */
        motor->torque_limit_positive = 500;

        /* FIXME: Should this actually be negative? */
        /* NOTE: For some reason it cannot be 0, so do -0.1% (according to UIUC's team) */
        motor->torque_limit_negative = -1;
    }

}

static void motorGetData(amk_motor_t* motor)
{
    if (!can_data.AMK_Actual_Values_1.stale) {
        motor->status.bits = can_data.AMK_Actual_Values_1.AMK_Status;
        motor->actual_torque = can_data.AMK_Actual_Values_1.AMK_ActualTorque;
        motor->serial_num = can_data.AMK_Actual_Values_1.AMK_MotorSerialNumber;
    }

    if (!can_data.AMK_Actual_Values_2.stale) {
        motor->actual_speed = can_data.AMK_Actual_Values_2.AMK_ActualSpeed;
        motor->dc_bus_voltage = can_data.AMK_Actual_Values_2.AMK_DCBusVoltage;
        motor->system_reset = can_data.AMK_Actual_Values_2.AMK_SystemReset;
        /* FIXME: Will this grab diagnostic number in time? It should? */
        motor->diagnostic_num = can_data.AMK_Actual_Values_2.AMK_DiagnosticNumber;
    }

    if (!can_data.AMK_Temperatures_1.stale) {
        motor->motor_temp = can_data.AMK_Temperatures_1.AMK_MotorTemp;
        motor->inverter_temp = can_data.AMK_Temperatures_1.AMK_InverterTemp;
        motor->igbt_temp = can_data.AMK_Temperatures_1.AMK_IGBTTemp;
    }
}

static void motorReset(amk_motor_t* motor)
{
    /* State machine here based on 8.2.6 */
    switch (motor->states.reset_state) {
    case MOTOR_RESET_INVERTER_OFF:
        /* 1. Set AMK_bInverterOn = 0 */
        motor->control.fields.AMK_bInverterOn = false;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.reset_state = MOTOR_RESET_ERROR_RESET_ON;

        break;
    case MOTOR_RESET_ERROR_RESET_ON:
        motor->control.fields.AMK_bErrorReset = true;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             0,
                             0,
                             0);

        motor->states.reset_state = MOTOR_RESET_ERROR_RESET_OFF;

        break;
    case MOTOR_RESET_ERROR_RESET_OFF:
        motor->control.fields.AMK_bErrorReset = false;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             0,
                             0,
                             0);

        motor->states.reset_state = MOTOR_RESET_CHECK_SYSTEM_READY;

        break;
    case MOTOR_RESET_CHECK_SYSTEM_READY:
        if (motor->status.fields.AMK_bSystemReady) {
            motor->states.reset_state = MOTOR_RESET_INVERTER_OFF;
            motor->states.running_stage = MOTOR_RUNNING_GOOD;
        }

        break;
    }
}

/* TODO: Not really sure what needs to be done here. We just need to push
 * these values that are determined in car.c state machine (READY2DRIVE state)
 */
static void motorRunning(amk_motor_t* motor)
{
    switch (motor->states.running_stage) {
    case MOTOR_RUNNING_GOOD:
        /* Set setpoints as needed */
        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        /* Check for errors */
        if (motor->status.fields.AMK_bError)
            motor->states.running_stage = MOTOR_RUNNING_ERROR;

        break;
    case MOTOR_RUNNING_ERROR:
        /* FIXME: We should also send a message to dash, so we can see
         * when this happens, or do we just latch an error in the fault
         * library? */
        motorReset(motor);

        break;
    }
}

static void turnMotorOn(amk_motor_t* motor)
{
    /*
     * Motor Datasheet:
     * https://www.amk-motion.com/amk-dokucd/dokucd/en/content/resources/pdf-dateien/pdk_205481_kw26-s5-fse-4q_en_.pdf
     *
     * Section 9.4 goes over turning the motors on and off 
     *
     * Steps with the "r" suffix are requirement steps, the requirement needs to
     * be met before moving onto the next step.
     */

    switch (motor->states.init_stage) {
    case MOTOR_INIT_POWER_ON:
        /* 1. Turn on 24V DC to inverters */
        /* 1r. Check AMK_bSystemReady = 1*/
        if (motor->status.fields.AMK_bSystemReady)
            motor->states.init_stage = MOTOR_INIT_PRECHARGE;

        break;
    case MOTOR_INIT_PRECHARGE:
        /* 2. Charge DC caps; QUE should be set (is this just DcOn?) */
        /* This step happens when HV turns on. I can check the precharge
         * complete GPIO pin to see when this is finished. When finished
         * I move onto the next state. */

        /* if precharge complete pin is high */
        /* NOTE: This is found for us in car.c. Can check the pin ourselves
         * if we should not be touching this struct outside of car.c */
        if (*motor->pchg_complete)
            motor->states.init_stage = MOTOR_INIT_DC_ON;

        break;
    case MOTOR_INIT_DC_ON:
        /* 3. Set AMK_bDcOn = 1 */
        motor->control.fields.AMK_bDcOn = true;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.init_stage = MOTOR_INIT_DC_ON_CHECK;

        break;
    case MOTOR_INIT_DC_ON_CHECK:
        /* 3r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
        /* When will AMK_bQuitDcOn go on? Does it take some time after 
         * DcOn is set?? */
        /* 3r. (QUE & AMK_bDcOn) -> Check AMK_bQuitDcOn = 1 */
            /* Does where do I check QUE??? */

        if (motor->status.fields.AMK_bQuitDcOn)
            motor->states.init_stage = MOTOR_INIT_TORQUE_INIT;

        break;
    case MOTOR_INIT_TORQUE_INIT:
        /* 4. Set AMK_TorqueLimitNegativ = 0 and AMK_TorqueLimitPositiv = 0 */
        /* Should already be done, just doing again to confirm */
        motor->torque_limit_positive = 0;
        motor->torque_limit_negative = 0;

        /* Says I also need to set act speed to 0? */
        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.init_stage = MOTOR_INIT_ENABLE;

        break;
    case MOTOR_INIT_ENABLE:
        /* 7. Set AMK_bEnable = 1 */
        motor->control.fields.AMK_bEnable = true;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.init_stage = MOTOR_INIT_INVERTER_ON;

        break;
    case MOTOR_INIT_INVERTER_ON:
        /* 8  Set AMK_bInverterOn = 1 */
        motor->control.fields.AMK_bInverterOn = true;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.init_stage = MOTOR_INIT_INVERTER_ON_CHECK;

        break;
    case MOTOR_INIT_INVERTER_ON_CHECK:
        /* 8r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */
        /* Same with AMK_bQuitDcOn, do we need seperate states for these quits?? */
        /* 9. Check AMK_bQuitInverterOn = 1 */

        /* This should be the last init state, so now we move onto the stage for 
         * running the motors */
        if (motor->status.fields.AMK_bQuitInverterOn) {
            motor->states.init_stage = MOTOR_INIT_DONE;
            motor->states.stage = MOTOR_STAGE_RUNNING;
        }

        break;
    }
}

static void turnMotorOff(amk_motor_t* motor)
{
    /*
     * Motor Datasheet:
     * https://www.amk-motion.com/amk-dokucd/dokucd/en/content/resources/pdf-dateien/pdk_205481_kw26-s5-fse-4q_en_.pdf
     *
     * Section 9.4 goes over turning the motors on and off 
     *
     * Steps with the "r" suffix are requirement steps, the requirement needs to
     * be met before moving onto the next step.
     */

    /* NOTE: For some reason in the deinit state machine, we have to turn the
     * inverter off, then disable, and then check for the inverter being off.
     * This is different to how it is done in the init state machine but it
     * is correct based on the datasheet.
     */

    switch(motor->states.deinit_stage) {
    case MOTOR_DEINIT_SETPOINTS_DEINIT:
        /* 1. Set setpoint settings to 0 (AMK_TargetVelocity, AMK_TorqueLimitNegativ, AMK_TorqueLimitPositiv) */
        motor->torque_setpoint = 0;
        motor->torque_limit_positive = 0;
        motor->torque_limit_negative = 0;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);
        
        motor->states.deinit_stage = MOTOR_DEINIT_INVERTER_OFF;

        break;

    case MOTOR_DEINIT_INVERTER_OFF:
        /* 2. Set AMK_bInverterOn = 0 */
        motor->control.fields.AMK_bInverterOn = false;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.deinit_stage = MOTOR_DEINIT_DISABLE;

        break;
    case MOTOR_DEINIT_DISABLE:
        /* 3. Set AMK_bEnable = 0 */

        motor->control.fields.AMK_bEnable = false;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.deinit_stage = MOTOR_DEINIT_QUIT_INVERTER_CHECK;

        break;
    case MOTOR_DEINIT_QUIT_INVERTER_CHECK:
        /* 4. Check AMK_bQuitInverterOn = 0 */
        if (!(motor->status.fields.AMK_bQuitInverterOn)) {
            motor->states.deinit_stage = MOTOR_DEINIT_DISABLE;
        }

        motor->states.deinit_stage = MOTOR_DEINIT_DC_OFF;

        break;
    case MOTOR_DEINIT_DC_OFF:
        /* 5. Set AMK_bDcOn = 0 */

        motor->control.fields.AMK_bDcOn = false;

        SEND_AMK_SETPOINTS(motor->control.bits,
                             motor->torque_setpoint,
                             motor->torque_limit_positive,
                             motor->torque_limit_negative);

        motor->states.deinit_stage = MOTOR_DEINIT_DC_OFF_CHECK;

        break;
    case MOTOR_DEINIT_DC_OFF_CHECK:
        /* 6r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
        /* 6r. Check AMK_bQuitDcOn = 0 */
        /* When will AMK_bQuitDcOn go on? Does it take some time after 
         * DcOn is set?? */
        /* 3r. (QUE & AMK_bDcOn) -> Check AMK_bQuitDcOn = 1 */
            /* Does where do I check QUE??? */

        if (!(motor->status.fields.AMK_bQuitDcOn))
            motor->states.init_stage = MOTOR_INIT_TORQUE_INIT;

        motor->states.deinit_stage = MOTOR_DEINIT_PRECHARGE;

        break;
    case MOTOR_DEINIT_PRECHARGE:
        /* 7. Discharge DC caps; QUE should be reset (is this just DcOn?) */
        
        /* FIXME: Will this work? Not sure if this goes low when discharged */
        if (!(*motor->pchg_complete))
            motor->states.init_stage = MOTOR_DEINIT_POWER_OFF;

        /* If discharged, move on */
        motor->states.deinit_stage = MOTOR_DEINIT_POWER_OFF;

        break;
    case MOTOR_DEINIT_POWER_OFF:
        /* 8. Turn off 24v DC to inverters */
        /* We don't do much here, just have to turn off the car I guess */
        motor->states.deinit_stage = MOTOR_DEINIT_DONE;
        motor->states.stage = MOTOR_STAGE_OFF;

        break;
    }
}
