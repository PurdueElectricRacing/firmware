#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <stdint.h>

typedef enum {
    UPDATE_PAGE,
    MENU_UP,
    MENU_DOWN,
    BACK_PAGE,
    FORWARD_PAGE,
    SELECT_BUTTON,
    START_BUTTON,
    MARK_DATA,
    TOGGLE_REGEN,
    EBB_MINUS,
    EBB_PLUS,
    TV1_MINUS,
    TV1_PLUS
} interface_action_t;

static constexpr uint32_t DRIVER_INTERFACE_PERIOD_MS = 50;
void driver_interface_periodic(void);

void driver_interface_init(void);

// Button EXTI handlers
void EXTI9_5_IRQHandler();
void EXTI15_10_IRQHandler();

#endif // DRIVER_INTERFACE_H