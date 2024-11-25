/**
 * @file crc.h
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief Hardware CRC32 w/ software fallback
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef _PHAL_CRC_H
#define _PHAL_CRC_H

#if defined(STM32F407xx)
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#include "system_stm32f7xx.h"
#else
#error "Please define a MCU arch"
#endif

void PHAL_CRC32_Reset(void);

uint32_t PHAL_CRC32_Calculate(uint32_t *data, uint32_t count);

uint32_t PHAL_CRC32_CalculateSW(uint32_t *data, uint32_t count);

#endif // _PHAL_CRC_H
