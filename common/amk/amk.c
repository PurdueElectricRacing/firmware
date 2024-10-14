#include "amk.h"
#include "source/main_module/can/can_parse.h"


/* NOTE:
 * I need change all of this to be a massive state machine in one function
 * like car.c. It will have states such as init which will turn motors on,
 * and a state to turn motors off and a state to actually run stuff.
 * The massive state machine will run periodically (not sure how often yet,
 * has to be < 50ms so that the control word can be send often)
 */

/* NOTE:
 * Step 1 is turning on LV
 * Step 3 happens when HV is turned on and precharging starts
 *  I can check when this is done with precharge complete and then move 
 *  onto other steps
 *  MAYBE we can check when AMK_bSystemReady is on and display a message
 *  on LCD so they know when to turn on the HV to start the precharging.
 *  Not sure if I need that
 */

static amk_motor_t motor = {0};

static AMK_Control_t control = {0};
static AMK_Status_t status = {0};

static void turnMotorsOn();
static void motorsRunning();
static void turnMotorsOff();

/* NOTE: As of now this is just setting everything to 0, but it may make sense
 * to have it in case something changes down the line while I learn more, so
 * this may end up being deleted if everything just inits to 0 */

/* FIXME: Move control word and status words into motor struct */

void motorInit(amk_motor_t* motor)
{
    *motor = (amk_motor_t){
        /* States */
        .states.stage = MOTOR_STAGE_INIT,
        .states.init_state = MOTOR_INIT_POWER_ON,
        /* FIXME: FILL IN ONCE I MAKE ENUMS */
        .states.deinit_state = 0,
        .states.running_state = 0,

        /* Values */
        .values.target_velocity = DEFAULT_TARGET_VELOCITY,
        .values.torque_limit_positive = DEFAULT_POSITIVE_TORQUE_LIMIT,
        .values.torque_limit_negative = DEFAULT_NEGATIVE_TORQUE_LIMIT
    };
}

void motorPeriodic()
{
    switch(motor.states.stage) {
    case MOTOR_STAGE_INIT:
        turnMotorsOn();
        break;
    case MOTOR_STAGE_RUNNING:
        motorsRunning();
        break;
    case MOTOR_STAGE_DEINIT:
        turnMotorsOff();
        break;
    }
}

static void motorsRunning()
{
    /* Set setpoint settings (AMK_TargetVelocity, AMK_TorqueLimitNegativ, AMK_TorqueLimitPositiv) */
    SEND_AMK_SETPOINTS_1(control.bits, 
                         1, 
                         1, 
                         1);
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

    switch (motor->states.init_state) {
        case MOTOR_INIT_POWER_ON:
            /* 1. Turn on 24V DC to inverters */
            /* 1r. Check AMK_bSystemReady = 1*/

            /* if AMK_bSystemReady = 1 */
            motor->states.init_state++;

            break;
        case MOTOR_INIT_PRECHARGE:
            /* 2. Charge DC caps; QUE should be set (is this just DcOn?) */
            /* This step happens when HV turns on. I can check the precharge
             * complete GPIO pin to see when this is finished. When finished
             * I move onto the next state. */

            /* if precharge complete pin is high */
            motor->states.init_state++;

            break;
        case MOTOR_INIT_DC_ON:
            /* 3. Set AMK_bDcOn = 1 */
            control.fields.AMK_bDcOn = true;
            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);
            motor->states.init_state++;
            break;
        case MOTOR_INIT_DC_ON_CHECK:
            /* 3r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
            status.bits = can_data.AMK_Actual_Values_1.AMK_Status;

            /* When will AMK_bQuitDcOn go on? Does it take some time after 
             * DcOn is set?? */
            /* 3r. (QUE & AMK_bDcOn) -> Check AMK_bQuitDcOn = 1 */
                /* Does where do I check QUE??? */

            motor->states.init_state++;
            break;
        case MOTOR_INIT_TORQUE_INIT:
            /* 4. Set AMK_TorqueLimitNegativ = 0 and AMK_TorqueLimitPositiv = 0 */
            motor->values.torque_limit_positive = 0;
            motor->values.torque_limit_negative = 0;

            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);

            motor->states.init_state++;
            break;
        case MOTOR_INIT_ENABLE:
            /* 7. Set AMK_bEnable = 1 */
            control.fields.AMK_bEnable = true;
            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);

            motor->states.init_state++;
            break;
        case MOTOR_INIT_INVERTER_ON:
            /* 8  Set AMK_bInverterOn = 1 */
            control.fields.AMK_bInverterOn = true;
            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);

            motor->states.init_state++;
            break;
        case MOTOR_INIT_INVERTER_ON_CHECK:
            /* 8r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */

            /* Same with AMK_bQuitDcOn, do we need seperate states for these quits?? */
            /* 9. Check AMK_bQuitInverterOn = 1 */

            /* This should be the last init state, so now we move onto the stage for 
             * running the motors */

            motor->states.stage++;
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

    switch(motor->states.deinit_state) {
        case MOTOR_DEINIT_SETPOINTS_DEINIT:
            /* 1. Set setpoint settings to 0 (AMK_TargetVelocity, AMK_TorqueLimitNegativ, AMK_TorqueLimitPositiv) */
            motor->values.torque_limit_positive = 0;
            motor->values.torque_limit_negative = 0;

            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);
            motor->states.deinit_state++;
            break;

        case MOTOR_DEINIT_INVERTER_OFF:
            /* 2. Set AMK_bInverterOn = 0 */
            control.fields.AMK_bInverterOn = true;

            SEND_AMK_SETPOINTS_1(control.bits,
                                 motor->values.target_velocity,
                                 motor->values.torque_limit_positive,
                                 motor->values.torque_limit_negative);

            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_INVERTER_OFF_CHECK:
            /* 2r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_DISABLE:
            /* 3. Set AMK_bEnable = 0 */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_QUIT_INVERTER_CHECK:
            /* 4. Check AMK_bQuitInverterOn = 0 */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_DC_OFF:
            /* 5. Set AMK_bDcOn = 0 */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_DC_OFF_CHECK:
            /* 5r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
            /* 5r. Check AMK_bQuitDcOn = 0 */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_PRECHARGE:
            /* 6. Discharge DC caps; QUE should be reset (is this just DcOn?) */

            /* If discharged, move on */
            motor->states.init_state++;
            break;
        case MOTOR_DEINIT_POWER_OFF:
            /* 7. Turn off 24v DC to inverters */
            motor->states.init_state++;
            break;
    }
}
