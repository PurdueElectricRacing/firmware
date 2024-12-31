#include "menu_system.h"
#include "nextion.h"

// Style configurations
// todo macro these eventually
#define MENU_GREY   38066
// #define STYLE_NORMAL_FG    BLACK
//#define STYLE_HOVER_BG     52857
// #define STYLE_HOVER_FG     BLACK
// #define STYLE_SELECTED_BG  64512
// #define STYLE_SELECTED_FG  WHITE

void style_normal(menu_element_t* element) {
    set_background(element->object_name, MENU_GREY);
    set_font_color(element->object_name, BLACK);
}

void style_hover(menu_element_t* element) {
    set_background(element->object_name, WHITE);
    set_font_color(element->object_name, BLACK);
    // todo change to set border to indicate selected
}

void style_selected(menu_element_t* element) {
    switch (element->type) {
        case ELEMENT_NUM:
            set_background(element->object_name, ORANGE);
            set_font_color(element->object_name, WHITE);
            break;
        case ELEMENT_BAR: // todo
            set_background(element->object_name, WHITE); 
            break;
        default:
            break;
    }
}

void menu_move_up(menu_page_t* page) {
    if (page->is_element_selected) {
        menu_increment_value(&page->elements[page->current_index]);
        return;
    }

    // Clear current element styling
    style_normal(&page->elements[page->current_index]);

    // Move to previous element
    if (page->current_index == 0) {
        page->current_index = page->num_elements - 1;
    } else {
        page->current_index--;
    }

    // Style new element
    style_hover(&page->elements[page->current_index]);
}

void menu_move_down(menu_page_t* page) {
    if (page->is_element_selected) {
        menu_decrement_value(&page->elements[page->current_index]);
        return;
    }

    // Clear current element styling
    style_normal(&page->elements[page->current_index]);

    // Move to next element
    page->current_index = (page->current_index + 1) % page->num_elements;

    // Style new element
    style_hover(&page->elements[page->current_index]);
}

void menu_select(menu_page_t* page) {
    menu_element_t* current = &page->elements[page->current_index];

    if (page->is_element_selected) {
        // Deselect element
        page->is_element_selected = false;
        style_hover(current);
        
        // Call onChange if defined
        if (current->on_change != NULL) {
            current->on_change();
        }
    } else {
        // Select element if it's a selectable type
        switch (current->type) {
            case ELEMENT_OPTION:
                current->is_enabled = !current->is_enabled;
                set_value(current->object_name, current->is_enabled);
                if (current->on_change != NULL) {
                    current->on_change();
                }
                break;
            case ELEMENT_NUM:
            case ELEMENT_BAR:
                page->is_element_selected = true;
                style_selected(current);
                break;
            case ELEMENT_LIST:
                // Toggle option state
                current->is_enabled = !current->is_enabled;
                set_value(current->object_name, current->is_enabled);
                if (current->on_change != NULL) {
                    current->on_change();
                }
                break;
            default:
                break;
        }
    }
}

void menu_increment_value(menu_element_t* element) {
    if (element->current_value + element->increment <= element->max_value) {
        element->current_value += element->increment;
    } else {
        element->current_value = element->min_value;
    }

    switch (element->type) {
        case ELEMENT_NUM:
            set_value(element->object_name, element->current_value);
            set_textf(element->object_name, "%d", element->current_value);
            break;
        case ELEMENT_BAR:
            set_value(element->object_name, element->current_value);
            break;
        default:
            break;
    }
}

void menu_decrement_value(menu_element_t* element) {
    if (element->current_value >= element->increment + element->min_value) {
        element->current_value -= element->increment;
    } else {
        element->current_value = element->max_value;
    }

    switch (element->type) {
        case ELEMENT_NUM:
            set_value(element->object_name, element->current_value);
            set_textf(element->object_name, "%d", element->current_value);
            break;
        case ELEMENT_BAR:
            set_value(element->object_name, element->current_value);
            break;
        default:
            break;
    }
}
