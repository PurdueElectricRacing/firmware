/**
 * @file nextion.h
 * @brief Nextion display driver interface
 *
 * Interface for controlling Nextion display modules through serial communication.
 *
 * @author Matthew Flanagan (matthewdavidflanagan@outlook.com)
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Irving Wang (wang5952@purdue.edu)
 *
 * Original implementation by Matthew Flanagan
 * Converted for current use by Luke Oxley
 * Expanded by Irving Wang
 */

#ifndef __NEXTION_H__
#define __NEXTION_H__

#include <stdint.h>
#include <strbuf.h>

// Serial insturcitons must be terminated with 3 bytes 0xFF
#define NXT_CMD_TERM "\xFF\xFF\xFF"
#define ASCII_OFFSET 48

// Color Definitions in 565 format
#define WHITE        65535
#define DARK_GRAY    33808
#define LIGHT_GRAY   55261
#define BLUE         1055
#define MUTED_BLUE   2278
#define GREEN        1632
#define MUTED_GREEN  320
#define BLACK        0
#define RED          63910
#define MUTED_RED    63488
#define YELLOW       65156
#define MUTED_YELLOW 12610

// Nextion Command Strings
#define NXT_BACKGROUND_COLOR ".bco="
#define NXT_BORDER_COLOR     ".bdrco="
#define NXT_FONT_COLOR       ".pco="
#define NXT_VALUE            ".val="
#define NXT_TEXT             ".txt="
#define NXT_BORDERW          ".borderw=" // Cannot modify the borderw of the TEXT object without intelligent series
#define NXT_PAGE             "page "
#define NXT_BRIGHTNESS       "dims="
#define NXT_BAUD             "bauds="

// Function Prototypes
void NXT_setFloat(char* obj_name, char* param, float num, uint8_t precision);
void NXT_setValue(char* obj_name, uint16_t val);
void NXT_setBackground(char* obj_name, uint16_t val);
void NXT_setFontColor(char* obj_name, uint16_t val);
void NXT_setBorderWidth(char* obj_name, uint16_t val);
void NXT_setBorderColor(char* obj_name, uint16_t val);
void NXT_setText(char* obj_name, char* text);
void NXT_setTextFormatted(char* obj_name, const char* format, ...);
void NXT_setPage(char* page_name);

// Configuration Functions
void NXT_setBrightness(uint8_t brightness);
void NXT_setBaud(uint32_t baud);

#endif
