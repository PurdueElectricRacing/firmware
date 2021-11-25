#include "apps.h"

/// Local Defines
#define APPS_BRAKE_THRESHOLD                (0.3f)  // EV.5.7.1
#define APPS_THROTTLE_FAULT_THRESHOLD       (0.25f) // EV.5.7.1
#define APPS_THROTTLE_CLEARFAULT_THRESHOLD  (0.05f) // EV.5.7.2

/// Private Typedefs
typedef struct
{
    bool enabled;       // Enable for this module

    bool apps_faulted;  // Holds state of current apps fault
} apps_state_internal_S;

/// Local Variables
static apps_state_internal_S apps_state = {
    .enabled = true,        // Enable module
    .apps_faulted = true,   // Start off faulted, this will clear if pedals are okay
};

/// Function definitions

// Initilize this module
void apps_Init(void)
{

}

// Called at motor torque send rate.
// Tick APPS state machine
void apps_Tick(float throttle_pos, float brake_pos)
{
    if (apps_state.apps_faulted == false)
    {
        // APPS faults when we are breaking and trying to accelerate
        if (APPS_BRAKE_THRESHOLD >= brake_pos)
        {
            // Fault when we are trying to accelerate beyond the threshold
            apps_state.apps_faulted = APPS_THROTTLE_FAULT_THRESHOLD <= throttle_pos;
        }
    }
    else
    {
        // Clear fault when we are under the clear threshold
        apps_state.apps_faulted = APPS_THROTTLE_CLEARFAULT_THRESHOLD >= throttle_pos;
    }
}

// Current state of apps fault
bool apps_IsAPPSFaulted()
{
    return (apps_state.enabled) && (apps_state.apps_faulted);
}

// Enable the faulted output of this module
void apps_SetEnabled(bool enabled)
{
    apps_state.enabled = enabled;
}