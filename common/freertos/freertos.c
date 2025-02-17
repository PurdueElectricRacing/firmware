
#include "freertos.h"

void rtosWrapper(void *thread)
{
    ThreadWrapper *wrapper = (ThreadWrapper *) thread;

    while(1)
    {
        wrapper->taskFunction();
        osDelay(wrapper->period);
    }

    osThreadTerminate(NULL);
}
