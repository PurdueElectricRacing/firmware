/**
 * @file node_Defs.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Definitions for per-BMS components
 * @version 0.1
 * @date 2022-02-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _NODE_DEFS_H
#define _NODE_DEFS_H

#include "can_parse.h"
/**
 * Each registered bootloader application ID lives here.
 * DO NOT change any existing IDs
 */

#ifndef BMS_NODE_NAME
    #warning "No BMS node name set, this binary will probably not work inside of the battery!"
    #define BMS_NODE_NAME

    #define SEND_SOC_CELLS      SEND_SOC_CELLS_1
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_1
    #define SEND_PACK_INFO      SEND_PACK_INFO_1
    #define SEND_CELL_INFO      SEND_CELL_INFO_1
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_1
    #define SEND_POWER_LIM      SEND_POWER_LIM_1

#elif BMS_NODE_NAME == BMS_A
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_1
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_1
    #define SEND_PACK_INFO      SEND_PACK_INFO_1
    #define SEND_CELL_INFO      SEND_CELL_INFO_1
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_1
    #define SEND_POWER_LIM      SEND_POWER_LIM_1

#elif BMS_NODE_NAME == BMS_B
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_2
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_2
    #define SEND_PACK_INFO      SEND_PACK_INFO_2
    #define SEND_CELL_INFO      SEND_CELL_INFO_2
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_2
    #define SEND_POWER_LIM      SEND_POWER_LIM_2

#elif BMS_NODE_NAME == BMS_C
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_3
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_3
    #define SEND_PACK_INFO      SEND_PACK_INFO_3
    #define SEND_CELL_INFO      SEND_CELL_INFO_3
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_3
    #define SEND_POWER_LIM      SEND_POWER_LIM_3

#elif BMS_NODE_NAME == BMS_D
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_4
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_4
    #define SEND_PACK_INFO      SEND_PACK_INFO_4
    #define SEND_CELL_INFO      SEND_CELL_INFO_4
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_4
    #define SEND_POWER_LIM      SEND_POWER_LIM_4

#elif BMS_NODE_NAME == BMS_E
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_5
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_5
    #define SEND_PACK_INFO      SEND_PACK_INFO_5
    #define SEND_CELL_INFO      SEND_CELL_INFO_5
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_5
    #define SEND_POWER_LIM      SEND_POWER_LIM_5

#elif BMS_NODE_NAME == BMS_F
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_6
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_6
    #define SEND_PACK_INFO      SEND_PACK_INFO_6
    #define SEND_CELL_INFO      SEND_CELL_INFO_6
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_6
    #define SEND_POWER_LIM      SEND_POWER_LIM_6

#elif BMS_NODE_NAME == BMS_G
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_7
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_7
    #define SEND_PACK_INFO      SEND_PACK_INFO_7
    #define SEND_CELL_INFO      SEND_CELL_INFO_7
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_7
    #define SEND_POWER_LIM      SEND_POWER_LIM_7

#elif BMS_NODE_NAME == BMS_H
    #define SEND_SOC_CELLS      SEND_SOC_CELLS_8
    #define SEND_VOLTS_CELLS    SEND_VOLTS_CELLS_8
    #define SEND_PACK_INFO      SEND_PACK_INFO_8
    #define SEND_CELL_INFO      SEND_CELL_INFO_8
    #define SEND_TEMPS_CELLS    SEND_TEMPS_CELLS_8
    #define SEND_POWER_LIM      SEND_POWER_LIM_8

#else
    #error "Warning! Unknown BMS_NODE_NAME defined."
#endif

#endif /* _NODE_DEFS_H */