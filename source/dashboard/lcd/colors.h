#ifndef COLORS_H
#define COLORS_H

/**
 * @file colors.c
 * @brief Color definitions for LCD display
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

// PER26 Color Pallete in 565 format
static constexpr uint16_t WHITE        = 65535;
static constexpr uint16_t DARK_GRAY    = 33808;
static constexpr uint16_t LIGHT_GRAY   = 55261;
static constexpr uint16_t BLUE         = 1055;
static constexpr uint16_t MUTED_BLUE   = 2278;
static constexpr uint16_t GREEN        = 1632;
static constexpr uint16_t MUTED_GREEN  = 320;
static constexpr uint16_t BLACK        = 0;
static constexpr uint16_t RED          = 63910;
static constexpr uint16_t MUTED_RED    = 63488;
static constexpr uint16_t YELLOW       = 65156;
static constexpr uint16_t MUTED_YELLOW = 12610;

#endif // COLORS_H