// original author: Matthew Flanagan
// matthewdavidflanagan@outlook.com

// converted by: Luke Oxley
// lcoxley@purdue.edu

#ifndef __NEXTION_H__
#define __NEXTION_H__

#include "stm32l496xx.h"
#include <string.h>
#include <stdio.h>
#include "common/queue/queue.h"

#define NXT_STR_SIZE 85
#define SET_VALUE_EXTRA 	13
#define SET_BCO_EXTRA 		13
#define SET_TEXT_EXTRA 		10
#define SET_PAGE_EXTRA      8
#define ASCII_OFFSET 		48
#define NXT_CMD_TERM "\xFF\xFF\xFF"

#define RED    63488
#define YELLOW 65504
#define GREEN  4065
#define RACE_GREEN 1376
#define WHITE 65535
#define BLACK 0



#define FLAG_ENABLED_PIC  1
#define FLAG_DISABLED_PIC 1

#define NXT_BACKGROUND_COLOR ".bco="
#define NXT_FONT_COLOR       ".pco="
#define NXT_VALUE            ".val="
#define NXT_TEXT             ".txt="
#define NXT_PICTURE          ".pic="

typedef struct
{
    char* name;
    uint8_t dirs[4]; // up right down left (CW)
    uint8_t norm_id;
    uint8_t high_id;
} button_t;

void set_flag(char* obj_name, uint8_t enable);
void set_float(char* obj_name, char* param, float num, uint8_t precision);
void set_value(char* obj_name, char* param, uint16_t val);
void set_text(char* obj_name, char* param, char* text);
void set_page(char* page_name);

#endif