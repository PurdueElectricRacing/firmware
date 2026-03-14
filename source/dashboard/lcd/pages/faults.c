/**
 * @file faults.c
 * @brief Faults page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "faults.h"

#include "menu_system.h"

// TODO: implement this page


menu_element_t fault_view_elements[] = {
    [0] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT1_BUTTON,
        .on_change   = nullptr
    },
    [1] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT2_BUTTON,
        .on_change   = nullptr
    },
    [2] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT3_BUTTON,
        .on_change   = nullptr
    },
    [3] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT4_BUTTON,
        .on_change   = nullptr
    },
    [4] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT5_BUTTON,
        .on_change   = nullptr
    },
    [5] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT6_BUTTON,
        .on_change   = nullptr
    },
    [6] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT7_BUTTON,
        .on_change   = nullptr
    },
    [7] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT8_BUTTON,
        .on_change   = nullptr
    }
};

menu_page_t fault_view_page = {
    .elements            = fault_view_elements,
    .num_elements        = sizeof(fault_view_elements) / sizeof(fault_view_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

void fault_view_refresh() {
    MS_refreshPage(&fault_view_page);
}

void fault_view_moveUp() {
    MS_moveUp(&fault_view_page);
}

void fault_view_moveDown() {
    MS_moveDown(&fault_view_page);
}

void fault_view_select() {
    MS_select(&fault_view_page);
}

void fault_telemetry_update() {
    return; // todo
}