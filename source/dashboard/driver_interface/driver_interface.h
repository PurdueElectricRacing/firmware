#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <stdint.h>

static constexpr uint32_t DRIVER_INTERFACE_PERIOD_MS = 100;
void driver_interface_periodic(void);

// Button EXTI handlers
void EXTI9_5_IRQHandler();
void EXTI15_10_IRQHandler();

#endif // DRIVER_INTERFACE_H