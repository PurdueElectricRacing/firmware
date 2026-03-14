/**
 * @file lcd.c
 * @brief LCD display management
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "lcd.h"

#include <stddef.h>
#include <stdint.h>

#include "nextion.h"

// pages
#include "pages/race.h"
#include "pages/calibration.h"
#include "pages/faults.h"

volatile page_t curr_page = PAGE_PREFLIGHT; // Current page displayed on the LCD
volatile page_t prev_page = PAGE_PREFLIGHT; // Previous page displayed on the LCD

// Page handlers array stored in flash
const page_handler_t page_handlers[NUM_PAGES] = { // Order must match page_t enum
    [PAGE_RACE] = {
        .update    = nullptr,
        .move_up   = nullptr,
        .move_down = nullptr,
        .select    = nullptr,
        .telemetry = race_telemetry_update,
        .string    = RACE_STRING
    },
    [PAGE_FAULTS] = {
        .update    = nullptr,
        .move_up   = nullptr,
        .move_down = nullptr,
        .select    = nullptr,
        .telemetry = fault_telemetry_update,
        .string    = FAULT_STRING
    },
    [PAGE_CALIBRATION] = {
        .update    = nullptr,
        .move_up   = nullptr,
        .move_down = nullptr,
        .select    = nullptr,
        .telemetry = calibration_telemetry_update,
        .string    = CALIBRATION_STRING
    }
};

void updatePage();

// Initialize the LCD screen
// Preflight will be shown on power on, then reset to RACE
void LCD_init(uint32_t baud_rate) {
    NXT_setBaud(baud_rate);
    NXT_setBrightness(100);

    // Set page (leave preflight)
    curr_page = PAGE_RACE;
    updatePage();
}

/**
 * @brief Advances to the next selectable page
 */
void advancePage() {
    curr_page = curr_page + 1;
    if (curr_page >= NUM_PAGES) {
        curr_page = 0;
    }

    updatePage();
}

/**
 * @brief Moves to the previous selectable page
 */
void backPage() {
    if (curr_page == 0) {
        curr_page = NUM_PAGES - 1;
    } else {
        curr_page = curr_page - 1;
    }

    updatePage();
}

/**
 * @brief Updates LCD display page
 *
 * Key behaviors:
 * - Maintains display of error pages when active
 */
void updatePage() {
    // If we do not detect a page update, do nothing
    if (curr_page == prev_page) {
        return;
    }

    // Set the page on display
    NXT_setPage(page_handlers[curr_page].string);
    prev_page = curr_page;

    // Call update handler if available
    if (page_handlers[curr_page].update != nullptr) {
        page_handlers[curr_page].update();
    }
}

void moveUp() {
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].move_up != nullptr) {
        page_handlers[curr_page].move_up();
    }
}

void moveDown() {
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].move_down != nullptr) {
        page_handlers[curr_page].move_down();
    }
}

void selectItem() {
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].select != nullptr) {
        page_handlers[curr_page].select();
    }
}

/**
 * @brief Updates current telemetry page by calling its handler if available
 */
void updateTelemetryPages() {
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].telemetry != nullptr) {
        page_handlers[curr_page].telemetry();
    }
}
