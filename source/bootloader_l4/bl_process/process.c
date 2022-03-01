/**
 * @file process.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "process.h"
#include "common/phal_L4/flash/flash.h"

uint32_t app_flash_start_address;   /* Store the start address of the Application, never changes */
uint32_t app_flash_current_address; /* Current address we are writing to */
uint32_t app_flash_end_address;     /* Expected end address to stop writing */

void initBLProcess(uint32_t app_flash_start)
{
    app_flash_start_address     = app_flash_start;
    app_flash_end_address       = app_flash_start;
    app_flash_current_address   = app_flash_start;
}

void process_bl_cmd(BLCmd_t cmd, uint64_t data)
{
    switch (cmd)
    {
        case BLCMD_INFO:
        {
            app_flash_current_address = app_flash_start_address;
            app_flash_end_address =+ data;
        }
        case BLCMD_ADD_ADDRESS:
        {
            app_flash_current_address += data;
            break;
        }
        case BLCMD_FW_DATA:
        {
            if(app_flash_current_address >= app_flash_start_address)
            {
                PHAL_flashWriteU32(app_flash_current_address, data);
                app_flash_current_address += 0x4;
            }
            break;
        }
        default:
        {
            // Error, invalid message
        }
    }
}