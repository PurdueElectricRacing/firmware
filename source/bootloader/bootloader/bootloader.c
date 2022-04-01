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

static bool bl_unlock = false;
static uint32_t num_msg = 0;
void BL_processCommand(BLCmd_t cmd, uint32_t data)
{
    static uint64_t data_buffer;
    bool program_buffer = false;
    switch (cmd)
    {
        case BLCMD_INFO:
        {
            app_flash_current_address = app_flash_start_address;
            app_flash_end_address += data; // Number of words
            *timeout_ticks = 0;
            uint8_t total_pages = (data / 0x2000);
            if (total_pages < 3) total_pages = 3;
            
            for(int i = 0; i < total_pages; i++)
                PHAL_flashErasePage(4+i);

            BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t) app_flash_current_address);
            num_msg = 0;
            program_buffer = false;
            bl_unlock = true;
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
            if(app_flash_current_address >= app_flash_start_address && bl_unlock)
            {
                if (((uint32_t) app_flash_current_address & 0b111) != 0)
                {
                    // Address is 2-word aligned, do the actual programming now
                    PHAL_flashWriteU64((uint32_t) app_flash_current_address & (~0b111), (((uint64_t) data) << 32) | data_buffer);
                    data_buffer = 0;
                }
                else 
                {
                    data_buffer = data;
                }
                *timeout_ticks = 0;
                num_msg++;
                BL_sendStatusMessage(BLSTAT_PROGRESS, (uint32_t)  app_flash_current_address);
                app_flash_current_address ++;
            }
            else
            {
                asm("bkpt");
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
    {
        if (data_buffer != 0 )
        {
            // We did not land on an 8-byte aligned address, program the last piece of data
            PHAL_flashWriteU64((uint32_t) app_flash_current_address & (~0b100), data_buffer);
        }
        BL_sendStatusMessage(BLSTAT_DONE, 0);
    
    }
}

bool BL_flashStarted(void)
{
    return (app_flash_end_address != app_flash_start_address);
}

bool BL_flashComplete(void)
{
    return (app_flash_end_address != app_flash_start_address) && (app_flash_current_address >= app_flash_end_address);
}

/*
    Component specific callbacks 
*/

// Quickly setup case statments for send function based on Node ID
#define NODE_CASE_BL_RESPONSE(app_id, resp_func) \
case app_id:\
            resp_func(q_tx_can, cmd, data);\
            break;\

void BL_sendStatusMessage(uint8_t cmd, uint32_t data)
{
    switch(APP_ID)
    {
        NODE_CASE_BL_RESPONSE(APP_MAINMODULE,       SEND_MAINMODULE_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_DASHBOARD,        SEND_DASHBOARD_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_TORQUEVECTOR,     SEND_TORQUEVECTOR_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_DRIVELINE_F,      SEND_DRIVELINE_F_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_DRIVELINE_R,      SEND_DRIVELINE_R_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_PRECHARGE,        SEND_PRECHARGE_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_A,            SEND_BMS_A_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_B,            SEND_BMS_B_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_C,            SEND_BMS_C_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_D,            SEND_BMS_D_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_E,            SEND_BMS_E_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_F,            SEND_BMS_F_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_G,            SEND_BMS_G_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_H,            SEND_BMS_H_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_I,            SEND_BMS_I_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_BMS_J,            SEND_BMS_J_BL_RESP)
        default:
            asm("bkpt");
    }
}

// Quickly setup the CAN callbacks based on Node ID
#define NODE_BL_CMD_CALLBACK(callback_name, can_msg_name, node_id) \
void callback_name(CanParsedData_t* msg_data_a) {\
    if(APP_ID != node_id) return; \
    BL_processCommand((BLCmd_t) msg_data_a->can_msg_name.cmd, msg_data_a->can_msg_name.data); \
} \

NODE_BL_CMD_CALLBACK(mainmodule_bl_cmd_CALLBACK,    mainmodule_bl_cmd,      APP_MAINMODULE)
NODE_BL_CMD_CALLBACK(dashboard_bl_cmd_CALLBACK,     dashboard_bl_cmd,       APP_DASHBOARD)
NODE_BL_CMD_CALLBACK(torquevector_bl_cmd_CALLBACK,  torquevector_bl_cmd,    APP_TORQUEVECTOR)
NODE_BL_CMD_CALLBACK(driveline_f_bl_cmd_CALLBACK,   driveline_f_bl_cmd,     APP_DRIVELINE_F)
NODE_BL_CMD_CALLBACK(driveline_r_bl_cmd_CALLBACK,   driveline_r_bl_cmd,     APP_DRIVELINE_R)
NODE_BL_CMD_CALLBACK(precharge_bl_cmd_CALLBACK,     precharge_bl_cmd,       APP_PRECHARGE)
NODE_BL_CMD_CALLBACK(bms_a_bl_cmd_CALLBACK,         bms_a_bl_cmd,           APP_BMS_A)
NODE_BL_CMD_CALLBACK(bms_b_bl_cmd_CALLBACK,         bms_b_bl_cmd,           APP_BMS_B)
NODE_BL_CMD_CALLBACK(bms_c_bl_cmd_CALLBACK,         bms_c_bl_cmd,           APP_BMS_C)
NODE_BL_CMD_CALLBACK(bms_d_bl_cmd_CALLBACK,         bms_d_bl_cmd,           APP_BMS_D)
NODE_BL_CMD_CALLBACK(bms_e_bl_cmd_CALLBACK,         bms_e_bl_cmd,           APP_BMS_E)
NODE_BL_CMD_CALLBACK(bms_f_bl_cmd_CALLBACK,         bms_f_bl_cmd,           APP_BMS_F)
NODE_BL_CMD_CALLBACK(bms_g_bl_cmd_CALLBACK,         bms_g_bl_cmd,           APP_BMS_G)
NODE_BL_CMD_CALLBACK(bms_h_bl_cmd_CALLBACK,         bms_h_bl_cmd,           APP_BMS_H)
NODE_BL_CMD_CALLBACK(bms_i_bl_cmd_CALLBACK,         bms_i_bl_cmd,           APP_BMS_I)
NODE_BL_CMD_CALLBACK(bms_j_bl_cmd_CALLBACK,         bms_j_bl_cmd,           APP_BMS_J)
