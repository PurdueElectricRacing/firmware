// original author: Matthew Flanagan
// matthewdavidflanagan@outlook.com

// converted by: Luke Oxley 
// lcoxley@purdue.edu

#ifndef __NEXTION_H__
#define __NEXTION_H__

#include "stm32l432xx.h"

  // TODO: get max len from uart
#define NXT_STR_SIZE 30
#define SET_VALUE_EXTRA 	13
#define SET_BCO_EXTRA 		13
#define SET_TEXT_EXTRA 		10
#define SET_PAGE_EXTRA      8
#define ASCII_OFFSET 		48
#define NXT_CMD_TERM "\xFF\xFF\xFF"

#define RED					63488
#define YELLOW				65504
#define GREEN				4065


#define FLAG_ENABLED_PIC 1
#define FLAG_DISABLED_PIC 1

#define NXT_BACKGROUND_COLOR ".bco="
#define NXT_FONT_COLOR ".pco="
#define NXT_VALUE ".val="
#define NXT_TEXT ".txt="
#define NXT_PICTURE ".pic="

typedef struct
{
    char* name;
    // thresholding 
} label_t;

typedef struct 
{
    char* name;
    uint8_t dirs[4]; // up right down left (CW)
    uint8_t norm_id;
    uint8_t high_id;
} button_t;

typedef enum {
    A_LABEL,
    A_VALUE,
    A_FLAG,
} attribute_type_t;


typedef struct
{
    char* name;
    attribute_type_t type;

    // Type specific settings
    union{
        struct { // A_VALUE
            void *val_addr;
            uint8_t val_size;
            uint16_t last_val;
            uint16_t low_thresh; // changes to low color
            uint16_t low_color;  // when <
            uint16_t high_thresh; // changes to high color
            uint16_t high_color;  // when >
        };
    };
} attribute_t;

typedef struct
{
    char* name;
    uint8_t num_attributes;
    attribute_t *attributes;
    uint8_t num_buttons;
    button_t *buttons;
} page_t;

void set_flag(char* obj_name, uint8_t enable);
void set_float(char* obj_name, char* param, float num, uint8_t precision);
void set_value(char* obj_name, char* param, uint16_t val);
void set_text(char* obj_name, char* param, char* text);
void set_page(char* page_name);
void lcd_send(char* msg);

#endif