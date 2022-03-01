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

#include "bootloader.h"
#include "common/phal_L4/flash/flash.h"

static uint32_t* app_flash_start_address;   /* Store the start address of the Application, never changes */
static uint32_t* app_flash_current_address; /* Current address we are writing to */
static uint32_t* app_flash_end_address;     /* Expected end address to stop writing */
static volatile uint32_t* timeout_ticks;

extern q_handle_t q_tx_can;

void BL_init(uint32_t* app_flash_start, volatile uint32_t* timeout_ticks_ptr)
{
    app_flash_start_address     = app_flash_start;
    app_flash_end_address       = app_flash_start;
    app_flash_current_address   = app_flash_start;
    timeout_ticks = timeout_ticks_ptr;
}

void BL_processCommand(BLCmd_t cmd, uint32_t data)
{
    switch (cmd)
    {
        case BLCMD_INFO:
        {
            app_flash_current_address = app_flash_start_address;
            app_flash_end_address += data; // Number of words
            *timeout_ticks = 0;
            BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t) app_flash_current_address);
            break;
        }
        case BLCMD_ADD_ADDRESS:
        {
            app_flash_current_address +=  data; // Number of words
            *timeout_ticks = 0;
            BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t) app_flash_current_address);
            break;
        }
        case BLCMD_FW_DATA:
        {
            if(app_flash_current_address >= app_flash_start_address)
            {
                PHAL_flashWriteU32(app_flash_current_address, data);
                app_flash_current_address ++;
                *timeout_ticks = 0;
                BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t) app_flash_current_address);
            }
            break;
        }
        default:
        {
            BL_sendStatusMessage(BLSTAT_UNKOWN_CMD, 0);
            break;
        }
    }

    if (BL_flashComplete())
        BL_sendStatusMessage(BLSTAT_DONE, 0);
}

bool BL_flashComplete(void)
{
    return (app_flash_end_address != app_flash_start_address) && (app_flash_current_address >= app_flash_end_address);
}

/*
    Component specific callbacks 
*/

void BL_sendStatusMessage(uint8_t cmd, uint32_t data)
{
    switch(APP_ID)
    {
        case APP_MAINMODULE: 
            SEND_MAINMODULE_BL_RESP(q_tx_can, cmd, data);
            break;

        case APP_DASHBOARD:
            SEND_DASHBOARD_BL_RESP(q_tx_can, cmd, data);
            break;
        default:
            break;
    }
}

void mainmodule_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a)
{
    if(APP_ID != APP_MAINMODULE) return;
    BL_processCommand((BLCmd_t) msg_data_a->mainmodule_bl_cmd.cmd, msg_data_a->mainmodule_bl_cmd.data);
}

void dashboard_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a)
{
    if(APP_ID != APP_DASHBOARD) return;
    BL_processCommand((BLCmd_t) msg_data_a->dashboard_bl_cmd.cmd, msg_data_a->dashboard_bl_cmd.data);
}