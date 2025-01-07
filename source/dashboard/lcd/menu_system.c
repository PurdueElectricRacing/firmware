#include "menu_system.h"

#include "nextion.h"
#include <stdint.h>

// Style configurations
// todo macro these eventually
#define MENU_GREY  38066
// #define STYLE_NORMAL_FG    BLACK
//#define STYLE_HOVER_BG     52857
// #define STYLE_HOVER_FG     BLACK
// #define STYLE_SELECTED_BG  64512
// #define STYLE_SELECTED_FG  WHITE

void style_normal(menu_element_t *element) {
    set_background(element->object_name, MENU_GREY);
    set_font_color(element->object_name, BLACK);
}

void style_hover(menu_element_t *element) {
    set_background(element->object_name, WHITE);
    set_font_color(element->object_name, BLACK);
    // todo change to set border to indicate selected
}

void style_selected(menu_element_t *element) {
    set_background(element->object_name, ORANGE);
    set_font_color(element->object_name, WHITE);
}

void apply_element_style(menu_element_t *element, bool is_hover) {
    if (element->type == ELEMENT_LIST && element->current_value == 1) {
        return; // Skip styling for special list case
    }
    
    if (is_hover) {
        style_hover(element);
    } else {
        style_normal(element);
    }
}

void menu_move_up(menu_page_t *page) {
    if (page->is_element_selected) {
        menu_increment_value(&page->elements[page->current_index]);
        return;
    }

    // Clear current element styling
    apply_element_style(&page->elements[page->current_index], false);

    // Move to previous element
    if (page->current_index == 0) {
        page->current_index = page->num_elements - 1;
    } else {
        page->current_index--;
    }

    // Style new element
    apply_element_style(&page->elements[page->current_index], true);
}

void menu_move_down(menu_page_t *page) {
    if (page->is_element_selected) {
        menu_decrement_value(&page->elements[page->current_index]);
        return;
    }

    // Clear current element styling
    apply_element_style(&page->elements[page->current_index], false);

    // Move to next element
    page->current_index = (page->current_index + 1) % page->num_elements;

    // Style new element
    apply_element_style(&page->elements[page->current_index], false);
}

void menu_select(menu_page_t *page) {
    menu_element_t* current = &page->elements[page->current_index];

    if (page->is_element_selected) {
        // Deselect element
        page->is_element_selected = false;
        style_hover(current);
        
        // Call onChange if defined
        if (current->on_change != NULL) {
            current->on_change();
        }
        return;
    }

    // Select element if it's a selectable type
    switch (current->type) {
        case ELEMENT_LIST:
            // Clear other options
            for (uint8_t i = 0; i < page->num_elements; i++) {
                if (page->elements[i].type == ELEMENT_LIST) {
                    page->elements[i].current_value = 0;
                    style_normal(&page->elements[i]);
                }
            }
            current->current_value = 1;
            if (current->on_change != NULL) {
                current->on_change();
            }
            style_selected(current);
            break;
        case ELEMENT_OPTION:
            current->current_value ^= 1;
            set_value(current->object_name, current->current_value);
            if (current->on_change != NULL) {
                current->on_change();
            }
            break;
        case ELEMENT_FLT: // Fall through
        case ELEMENT_NUM:
            page->is_element_selected = true;
            style_selected(current);
            break;
        default:
            break;
    }
}

void menu_increment_value(menu_element_t *element) {
    if (element->current_value + element->increment <= element->max_value) {
        element->current_value += element->increment;
    } else {
        element->current_value = element->min_value;
    }

    switch (element->type) {
        case ELEMENT_FLT: // Fall through
        case ELEMENT_NUM:
            set_value(element->object_name, element->current_value);
            set_textf(element->object_name, "%d", element->current_value);
            break;
        default:
            break;
    }
}

void menu_decrement_value(menu_element_t *element) {
    if (element->current_value >= element->increment + element->min_value) {
        element->current_value -= element->increment;
    } else {
        element->current_value = element->max_value;
    }

    switch (element->type) {
        case ELEMENT_FLT: // Fall through
        case ELEMENT_NUM:
            set_value(element->object_name, element->current_value);
            set_textf(element->object_name, "%d", element->current_value);
            break;
        default:
            break;
    }
}

void menu_refresh_page(menu_page_t *page) {
    page->is_element_selected = false;
    page->current_index = 0;
    int list_index = menu_list_get_selected(page);
    for (uint8_t i = 0; i < page->num_elements; i++) {
        menu_element_t *curr_element = &page->elements[i];
        switch (curr_element->type) {
            case ELEMENT_LIST:
                if (i == list_index) {  // nothing happens if -1
                    style_selected(curr_element);
                }
                break;
            case ELEMENT_NUM:
                set_textf(curr_element->object_name, "%d", curr_element->current_value);
                break;
            case ELEMENT_FLT: // Fall through
            case ELEMENT_OPTION:
                set_value(curr_element->object_name, curr_element->current_value);
                break;
            default:
                break;
        }
    }
}

int menu_list_get_selected(menu_page_t *page) {
    for (uint8_t i = 0; i < page->num_elements; i++) {
        if (page->elements[i].current_value) {
            return i;
        }
    }
    return -1;
}