
#include "freertos.h"

void rtosWrapper(void *thread)
{
    ThreadWrapper *wrapper = (ThreadWrapper *) thread;

    while(1)
    {
        wrapper->taskFunction();
        osDelay(wrapper->delay);
    }

    osThreadTerminate(NULL);
}
