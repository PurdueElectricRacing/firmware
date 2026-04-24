/**
 * @file faults.c
 * @brief Faults page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "faults.h"

#include "menu_system.h"

// TODO: implement this page


menu_element_t faults_elements[] = {
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

menu_page_t faults_page = {
    .elements            = faults_elements,
    .num_elements        = sizeof(faults_elements) / sizeof(faults_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

void faults_update() {
    MS_refreshPage(&faults_page);
}

void faults_move_up() {
    MS_moveUp(&faults_page);
}

void faults_move_down() {
    MS_moveDown(&faults_page);
}

void faults_select() {
    MS_select(&faults_page);
}

void faults_telemetry_update() {
    return; // todo
}