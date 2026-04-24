#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/**
 * @file lcd.h
 * @brief LCD display management
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

typedef enum : uint8_t {
    PAGE_RACE        = 0,
    PAGE_FAULTS      = 1,
    PAGE_CALIBRATION = 2,
    PAGE_AMK         = 3,
    NUM_PAGES, // leave as auto to count number of pages
    PAGE_PREFLIGHT, // not selectable, only shown on power on
} page_t;

typedef struct {
    void (*update)(void);
    void (*move_up)(void);
    void (*move_down)(void);
    void (*select)(void);
    void (*telemetry)(void);
    char *string;
} page_handler_t;

void advancePage(void);
void backPage(void);
void selectItem(void);
void moveUp(void);
void moveDown(void);
void updatePage(void);
void updateTelemetryPages(void);
void flush_LCD(void);

#endif // LCD_H
