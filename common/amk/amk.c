#include "amk.h"

void turnMotorsOn()
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
    
    /* 1. Turn on 24V DC to inverters */
        /* 1r. Check AMK_bSystemReady = 1 for all inverters */
    /* 2. Charge DC caps; QUE should be set (is this just DcOn?) */
    /* 3. Set AMK_bDcOn = 1 */
        /* 3r. AMK_bDcOn is mirrored in AMK_Status, so should be on there */
        /* 3r. (QUE & AMK_bDcOn) -> Check AMK_bQuitDcOn = 1 */
            /* Does where do I check QUE??? */
    /* 4. Set AMK_TorqueLimitNegativ = 0 and AMK_TorqueLimitPositiv = 0 */
    /* 5. Set X15 hardware signals EF and EF2 = 1 */
    /* 6. Set X140 hardware signal BE1 = 1 */
    /* 7. Set AMK_bEnable = 1 */
    /* 8  Set AMK_bInverterOn = 1 */
        /* 8r. AMK_bInverterOn is mirrored in AMK_Status, so should be on there */
    /* 9. Check AMK_bQuitInverterOn = 1 */
    /* 10. Set X140 hardware signal BE2 = 1 */
    /* 11. Set setpoint settings (AMK_TargetVelocity, AMK_TorqueLimitNegativ, AMK_TorqueLimitPositiv) */
}

void turnMotorsOff()
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
