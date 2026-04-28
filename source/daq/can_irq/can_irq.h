

#ifndef CAN_IRQ_H
#define CAN_IRQ_H

#include <stdint.h>

extern volatile uint32_t last_can_rx_time_ms;
void CAN1_RX0_IRQHandler();
void CAN2_RX0_IRQHandler();

#endif // CAN_IRQ_H