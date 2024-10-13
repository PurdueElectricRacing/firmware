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

/* THIS NEEDS TO BE SENT EVERY 50ms */
static AMK_Control_t control = {0};

static AMK_Status_t status = {0};

static void turnMotorsOn();
static void motorsRunning();
static void turnMotorsOff();

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

static void turnMotorsOn()
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

    /*
     * Steps 3 and 4, both are in same CAN message, how do I do this? Can I
     * just send 0 during step 3?
     */
    switch (motor.states.init_state) {
        case MOTOR_INIT_POWER_ON:
            /* 1. Turn on 24V DC to inverters */
            /* 1r. Check AMK_bSystemReady = 1*/

            /* if AMK_bSystemReady = 1 */
            motor.states.init_state++;

            break;
        case MOTOR_INIT_PRECHARGE:
            /* 2. Charge DC caps; QUE should be set (is this just DcOn?) */
            /* This step happens when HV turns on. I can check the precharge
             * complete GPIO pin to see when this is finished. When finished
             * I move onto the next state. */

            /* if precharge complete pin is high */
            motor.states.init_state++;

            break;
        case MOTOR_INIT_DC_ON:
            /* 3. Set AMK_bDcOn = 1 */
            control.fields.AMK_bDcOn = true;
            SEND_AMK_SETPOINTS_1(control.bits, 
                                 DEFAULT_TARGET_VELOCITY, 
                                 DEFAULT_POSITIVE_TORQUE_LIMIT, 
                                 DEFAULT_NEGATIVE_TORQUE_LIMIT);
            motor.states.init_state++;
            break;
        case MOTOR_INIT_DC_ON_CHECK:
            /* 3r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
            status.bits = can_data.AMK_Actual_Values_1.AMK_Status;

            /* When will AMK_bQuitDcOn go on? Does it take some time after 
             * DcOn is set?? */
            /* 3r. (QUE & AMK_bDcOn) -> Check AMK_bQuitDcOn = 1 */
                /* Does where do I check QUE??? */

            motor.states.init_state++;
            break;
        case MOTOR_INIT_TORQUE_INIT:
            /* 4. Set AMK_TorqueLimitNegativ = 0 and AMK_TorqueLimitPositiv = 0 */
            SEND_AMK_SETPOINTS_1(control.bits, 
                                 DEFAULT_TARGET_VELOCITY, 
                                 DEFAULT_POSITIVE_TORQUE_LIMIT, 
                                 DEFAULT_NEGATIVE_TORQUE_LIMIT);

            motor.states.init_state++;
            break;
        case MOTOR_INIT_ENABLE:
            /* 7. Set AMK_bEnable = 1 */
            control.fields.AMK_bEnable = true;
            SEND_AMK_SETPOINTS_1(control.bits, 
                                 DEFAULT_TARGET_VELOCITY, 
                                 DEFAULT_POSITIVE_TORQUE_LIMIT, 
                                 DEFAULT_NEGATIVE_TORQUE_LIMIT);

            motor.states.init_state++;
            break;
        case MOTOR_INIT_INVERTER_ON:
            /* 8  Set AMK_bInverterOn = 1 */
            control.fields.AMK_bInverterOn = true;
            SEND_AMK_SETPOINTS_1(control.bits, 
                                 DEFAULT_TARGET_VELOCITY, 
                                 DEFAULT_POSITIVE_TORQUE_LIMIT, 
                                 DEFAULT_NEGATIVE_TORQUE_LIMIT);

            motor.states.init_state++;
            break;
        case MOTOR_INIT_INVERTER_ON_CHECK:
            /* 8r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */

            /* Same with AMK_bQuitDcOn, do we need seperate states for these quits?? */
            /* 9. Check AMK_bQuitInverterOn = 1 */

            /* This should be the last init state, so now we move onto the stage for 
             * running the motors */

            motor.states.stage++;
            break;
    }
    

}

static void turnMotorsOff()
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

    /* 1. Set setpoint settings to 0 (AMK_TargetVelocity, AMK_TorqueLimitNegativ, AMK_TorqueLimitPositiv) */
    /* 2. Set X140 hardware signal BE2 = 0 */
    /* 3  Set AMK_bInverterOn = 1 */
    /* 3r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */
    /* 4. Set AMK_bEnable = 0 */
    /* 5. Check AMK_bQuitInverterOn = 0 */
    /* 6. Set X140 hardware signal BE1 = 0 */
    /* 7. Set X15 hardware signals EF and EF2 = 0 */
    /* 8. Set AMK_bDcOn = 1 */
    /* 8r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
    /* 8r. Check AMK_bQuitDcOn = 0 */
    /* Does where do I check QUE??? */
    /* 9. Charge DC caps; QUE should be set (is this just DcOn?) */
    /* 10. Turn off 24v DC to inverters */

}
