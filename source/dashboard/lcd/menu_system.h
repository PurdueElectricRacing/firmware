#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

// Element types
typedef enum {
    ELEMENT_NUM,    // Numeric value and float types
    ELEMENT_BAR,    // Shared with numeric value
    ELEMENT_TEXT,   // Text type
    ELEMENT_OPTION, // On/off toggle
    ELEMENT_LIST    // Item in a list
} element_type_t;

// Element states
typedef enum {
    STATE_NORMAL,
    STATE_HOVER,
    STATE_SELECTED
} element_state_t;

// Element structure
typedef struct {
    element_type_t type;
    element_state_t state;
    char* object_name;          // Nextion object name
    uint8_t current_value;      // Current value for numeric types
    uint8_t min_value;          // Minimum value for numeric types
    uint8_t max_value;          // Maximum value for numeric types
    uint8_t increment;          // Increment for numeric types
    bool is_enabled;            // For toggles and options
    void (*on_change)(void);    // Callback when value changes
} menu_element_t; // todo eventually make this const (to store in flash)

// Page structure
typedef struct {
    menu_element_t* elements;   // Array of elements
    uint8_t num_elements;       // Number of elements in array
    uint8_t current_index;      // Currently selected element index
    bool is_element_selected;   // Is an element currently selected?
    bool saved;                 // Generic saved state flag
    void* page_data;            // Pointer to page-specific data
} menu_page_t;

// Style functions
void style_normal(menu_element_t* element);
void style_hover(menu_element_t* element);
void style_selected(menu_element_t* element);

// Navigation functions
void menu_move_up(menu_page_t* page);
void menu_move_down(menu_page_t* page);
void menu_select(menu_page_t* page);

// Value modification functions
void menu_increment_value(menu_element_t* element);
void menu_decrement_value(menu_element_t* element);

#endif // MENU_SYSTEM_H
