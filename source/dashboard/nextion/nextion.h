#ifndef __NEXTION_H__
#define __NEXTION_H__

#include <string.h>
#include <stdio.h>
#include "common/queue/queue.h"

#define NXT_STR_SIZE        100 // Important: Used for usart queue size, issues arise if less than len of longest fault msg
#define SET_VALUE_EXTRA 	13
#define SET_BCO_EXTRA 		13
#define SET_TEXT_EXTRA 		10
#define SET_PAGE_EXTRA      8
#define ASCII_OFFSET 		48
#define NXT_CMD_TERM "\xFF\xFF\xFF" // Serial insturcitons must be terminated with 3 bytes 0xFF

#define RED    63488
#define YELLOW 65504
#define GREEN  4065
#define RACE_GREEN 1376
#define WHITE 65535
#define BLACK 0
#define GOLD 50273
#define GREY 57051
#define ORANGE 64512

#define FLAG_ENABLED_PIC  1
#define FLAG_DISABLED_PIC 1

#define NXT_BACKGROUND_COLOR ".bco="
#define NXT_FONT_COLOR       ".pco="
#define NXT_VALUE            ".val="
#define NXT_TEXT             ".txt="
#define NXT_BORDERW          ".borderw=" // Cannot modify the border width of the TEXT object 
#define NXT_PAGE             "page "
#define NXT_BRIGHTNESS       "dims="
#define NXT_BAUD             "bauds="

void set_flag(char* obj_name, uint8_t enable);
void set_float(char* obj_name, char* param, float num, uint8_t precision);
void set_value(char* obj_name, uint16_t val);
void set_background(char* obj_name, uint16_t val);
void set_font_color(char* obj_name, uint16_t val);
void set_border_width(char* obj_name, uint16_t val);
void set_text(char* obj_name, char* text);
void set_textf(char* obj_name, const char* format, ...);
void set_page(char* page_name);
void set_brightness(uint8_t brightness);
void set_baud(uint32_t baud);

#endif