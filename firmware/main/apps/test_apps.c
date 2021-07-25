#include "apps.h"
#include "assert.h"

void test_apps()
{
    apps_Init();

    apps_Tick(0.0, 0.5);
    assert(apps_IsAPPSFaulted() == false);

    apps_Tick(0.3, 0.5);
    assert(apps_IsAPPSFaulted() == true);

    apps_Tick(0.3, 0.0);
    assert(apps_IsAPPSFaulted() == true);

    apps_Tick(0.04, 0.0);
    assert(apps_IsAPPSFaulted() == false);
}