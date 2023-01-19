/**
 * @file can_parse.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief Parsing of CAN messages using auto-generated structures with bit-fields
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "can_parse.h"

// prototypes
bool initCANFilter();

can_data_t can_data;
q_handle_t* q_rx_can_a;
volatile uint32_t last_can_rx_time_ms = 0;

void initCANParse(q_handle_t* rx_a)
{
    q_rx_can_a = rx_a;
    initCANFilter();
}

void canRxUpdate()
{
    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(q_rx_can_a, &msg_header) == SUCCESS_G)
    {
        last_can_rx_time_ms = sched.os_ticks;
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        /* BEGIN AUTO CASES */
        if (msg_header.Bus == CAN2)
        {
            switch(msg_header.ExtId)
            {
                case ID_MODULE_TEMP_0:
                    can_data.module_temp_0.mod_temp_0 = msg_data_a->module_temp_0.mod_temp_0;
                    can_data.module_temp_0.mod_temp_1 = msg_data_a->module_temp_0.mod_temp_1;
                    can_data.module_temp_0.mod_temp_2 = msg_data_a->module_temp_0.mod_temp_2;
                    can_data.module_temp_0.mod_temp_3 = msg_data_a->module_temp_0.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_1:
                    can_data.module_temp_1.mod_temp_0 = msg_data_a->module_temp_1.mod_temp_0;
                    can_data.module_temp_1.mod_temp_1 = msg_data_a->module_temp_1.mod_temp_1;
                    can_data.module_temp_1.mod_temp_2 = msg_data_a->module_temp_1.mod_temp_2;
                    can_data.module_temp_1.mod_temp_3 = msg_data_a->module_temp_1.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_2:
                    can_data.module_temp_2.mod_temp_0 = msg_data_a->module_temp_2.mod_temp_0;
                    can_data.module_temp_2.mod_temp_1 = msg_data_a->module_temp_2.mod_temp_1;
                    can_data.module_temp_2.mod_temp_2 = msg_data_a->module_temp_2.mod_temp_2;
                    can_data.module_temp_2.mod_temp_3 = msg_data_a->module_temp_2.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_3:
                    can_data.module_temp_3.mod_temp_0 = msg_data_a->module_temp_3.mod_temp_0;
                    can_data.module_temp_3.mod_temp_1 = msg_data_a->module_temp_3.mod_temp_1;
                    can_data.module_temp_3.mod_temp_2 = msg_data_a->module_temp_3.mod_temp_2;
                    can_data.module_temp_3.mod_temp_3 = msg_data_a->module_temp_3.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_4:
                    can_data.module_temp_4.mod_temp_0 = msg_data_a->module_temp_4.mod_temp_0;
                    can_data.module_temp_4.mod_temp_1 = msg_data_a->module_temp_4.mod_temp_1;
                    can_data.module_temp_4.mod_temp_2 = msg_data_a->module_temp_4.mod_temp_2;
                    can_data.module_temp_4.mod_temp_3 = msg_data_a->module_temp_4.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_5:
                    can_data.module_temp_5.mod_temp_0 = msg_data_a->module_temp_5.mod_temp_0;
                    can_data.module_temp_5.mod_temp_1 = msg_data_a->module_temp_5.mod_temp_1;
                    can_data.module_temp_5.mod_temp_2 = msg_data_a->module_temp_5.mod_temp_2;
                    can_data.module_temp_5.mod_temp_3 = msg_data_a->module_temp_5.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_6:
                    can_data.module_temp_6.mod_temp_0 = msg_data_a->module_temp_6.mod_temp_0;
                    can_data.module_temp_6.mod_temp_1 = msg_data_a->module_temp_6.mod_temp_1;
                    can_data.module_temp_6.mod_temp_2 = msg_data_a->module_temp_6.mod_temp_2;
                    can_data.module_temp_6.mod_temp_3 = msg_data_a->module_temp_6.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_7:
                    can_data.module_temp_7.mod_temp_0 = msg_data_a->module_temp_7.mod_temp_0;
                    can_data.module_temp_7.mod_temp_1 = msg_data_a->module_temp_7.mod_temp_1;
                    can_data.module_temp_7.mod_temp_2 = msg_data_a->module_temp_7.mod_temp_2;
                    can_data.module_temp_7.mod_temp_3 = msg_data_a->module_temp_7.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_8:
                    can_data.module_temp_8.mod_temp_0 = msg_data_a->module_temp_8.mod_temp_0;
                    can_data.module_temp_8.mod_temp_1 = msg_data_a->module_temp_8.mod_temp_1;
                    can_data.module_temp_8.mod_temp_2 = msg_data_a->module_temp_8.mod_temp_2;
                    can_data.module_temp_8.mod_temp_3 = msg_data_a->module_temp_8.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_9:
                    can_data.module_temp_9.mod_temp_0 = msg_data_a->module_temp_9.mod_temp_0;
                    can_data.module_temp_9.mod_temp_1 = msg_data_a->module_temp_9.mod_temp_1;
                    can_data.module_temp_9.mod_temp_2 = msg_data_a->module_temp_9.mod_temp_2;
                    can_data.module_temp_9.mod_temp_3 = msg_data_a->module_temp_9.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_10:
                    can_data.module_temp_10.mod_temp_0 = msg_data_a->module_temp_10.mod_temp_0;
                    can_data.module_temp_10.mod_temp_1 = msg_data_a->module_temp_10.mod_temp_1;
                    can_data.module_temp_10.mod_temp_2 = msg_data_a->module_temp_10.mod_temp_2;
                    can_data.module_temp_10.mod_temp_3 = msg_data_a->module_temp_10.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_11:
                    can_data.module_temp_11.mod_temp_0 = msg_data_a->module_temp_11.mod_temp_0;
                    can_data.module_temp_11.mod_temp_1 = msg_data_a->module_temp_11.mod_temp_1;
                    can_data.module_temp_11.mod_temp_2 = msg_data_a->module_temp_11.mod_temp_2;
                    can_data.module_temp_11.mod_temp_3 = msg_data_a->module_temp_11.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_12:
                    can_data.module_temp_12.mod_temp_0 = msg_data_a->module_temp_12.mod_temp_0;
                    can_data.module_temp_12.mod_temp_1 = msg_data_a->module_temp_12.mod_temp_1;
                    can_data.module_temp_12.mod_temp_2 = msg_data_a->module_temp_12.mod_temp_2;
                    can_data.module_temp_12.mod_temp_3 = msg_data_a->module_temp_12.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_13:
                    can_data.module_temp_13.mod_temp_0 = msg_data_a->module_temp_13.mod_temp_0;
                    can_data.module_temp_13.mod_temp_1 = msg_data_a->module_temp_13.mod_temp_1;
                    can_data.module_temp_13.mod_temp_2 = msg_data_a->module_temp_13.mod_temp_2;
                    can_data.module_temp_13.mod_temp_3 = msg_data_a->module_temp_13.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_14:
                    can_data.module_temp_14.mod_temp_0 = msg_data_a->module_temp_14.mod_temp_0;
                    can_data.module_temp_14.mod_temp_1 = msg_data_a->module_temp_14.mod_temp_1;
                    can_data.module_temp_14.mod_temp_2 = msg_data_a->module_temp_14.mod_temp_2;
                    can_data.module_temp_14.mod_temp_3 = msg_data_a->module_temp_14.mod_temp_3;
                    break;
                case ID_MODULE_TEMP_15:
                    can_data.module_temp_15.mod_temp_0 = msg_data_a->module_temp_15.mod_temp_0;
                    can_data.module_temp_15.mod_temp_1 = msg_data_a->module_temp_15.mod_temp_1;
                    can_data.module_temp_15.mod_temp_2 = msg_data_a->module_temp_15.mod_temp_2;
                    can_data.module_temp_15.mod_temp_3 = msg_data_a->module_temp_15.mod_temp_3;
                    break;
                default:
                    __asm__("nop");
            }
        }
        else if (msg_header.Bus == CAN1)
        {
            switch(msg_header.ExtId)
            {
                case ID_ELCON_CHARGER_STATUS:
                    can_data.elcon_charger_status.charge_voltage = msg_data_a->elcon_charger_status.charge_voltage;
                    can_data.elcon_charger_status.charge_current = msg_data_a->elcon_charger_status.charge_current;
                    can_data.elcon_charger_status.hw_fail = msg_data_a->elcon_charger_status.hw_fail;
                    can_data.elcon_charger_status.temp_fail = msg_data_a->elcon_charger_status.temp_fail;
                    can_data.elcon_charger_status.input_v_fail = msg_data_a->elcon_charger_status.input_v_fail;
                    can_data.elcon_charger_status.startup_fail = msg_data_a->elcon_charger_status.startup_fail;
                    can_data.elcon_charger_status.communication_fail = msg_data_a->elcon_charger_status.communication_fail;
                    can_data.elcon_charger_status.stale = 0;
                    can_data.elcon_charger_status.last_rx = sched.os_ticks;
                    break;
                case ID_ORION_INFO:
                    can_data.orion_info.discharge_enable = msg_data_a->orion_info.discharge_enable;
                    can_data.orion_info.charge_enable = msg_data_a->orion_info.charge_enable;
                    can_data.orion_info.charger_safety = msg_data_a->orion_info.charger_safety;
                    can_data.orion_info.dtc_status = msg_data_a->orion_info.dtc_status;
                    can_data.orion_info.multi_input = msg_data_a->orion_info.multi_input;
                    can_data.orion_info.always_on = msg_data_a->orion_info.always_on;
                    can_data.orion_info.is_ready = msg_data_a->orion_info.is_ready;
                    can_data.orion_info.is_charging = msg_data_a->orion_info.is_charging;
                    can_data.orion_info.multi_input_2 = msg_data_a->orion_info.multi_input_2;
                    can_data.orion_info.multi_input_3 = msg_data_a->orion_info.multi_input_3;
                    can_data.orion_info.reserved = msg_data_a->orion_info.reserved;
                    can_data.orion_info.multi_output_2 = msg_data_a->orion_info.multi_output_2;
                    can_data.orion_info.multi_output_3 = msg_data_a->orion_info.multi_output_3;
                    can_data.orion_info.multi_output_4 = msg_data_a->orion_info.multi_output_4;
                    can_data.orion_info.multi_enable = msg_data_a->orion_info.multi_enable;
                    can_data.orion_info.multi_output_1 = msg_data_a->orion_info.multi_output_1;
                    can_data.orion_info.pack_dcl = msg_data_a->orion_info.pack_dcl;
                    can_data.orion_info.pack_ccl = msg_data_a->orion_info.pack_ccl;
                    can_data.orion_info.pack_soc = msg_data_a->orion_info.pack_soc;
                    can_data.orion_info.stale = 0;
                    can_data.orion_info.last_rx = sched.os_ticks;
                    break;
                case ID_ORION_CURRENTS_VOLTS:
                    can_data.orion_currents_volts.pack_current = (int16_t) msg_data_a->orion_currents_volts.pack_current;
                    can_data.orion_currents_volts.pack_voltage = msg_data_a->orion_currents_volts.pack_voltage;
                    can_data.orion_currents_volts.stale = 0;
                    can_data.orion_currents_volts.last_rx = sched.os_ticks;
                    break;
                case ID_ORION_ERRORS:
                    can_data.orion_errors.discharge_limit_enforce = msg_data_a->orion_errors.discharge_limit_enforce;
                    can_data.orion_errors.charger_safety_relay = msg_data_a->orion_errors.charger_safety_relay;
                    can_data.orion_errors.internal_hardware = msg_data_a->orion_errors.internal_hardware;
                    can_data.orion_errors.heatsink_thermistor = msg_data_a->orion_errors.heatsink_thermistor;
                    can_data.orion_errors.software = msg_data_a->orion_errors.software;
                    can_data.orion_errors.max_cellv_high = msg_data_a->orion_errors.max_cellv_high;
                    can_data.orion_errors.min_cellv_low = msg_data_a->orion_errors.min_cellv_low;
                    can_data.orion_errors.pack_overheat = msg_data_a->orion_errors.pack_overheat;
                    can_data.orion_errors.reserved0 = msg_data_a->orion_errors.reserved0;
                    can_data.orion_errors.reserved1 = msg_data_a->orion_errors.reserved1;
                    can_data.orion_errors.reserved2 = msg_data_a->orion_errors.reserved2;
                    can_data.orion_errors.reserved3 = msg_data_a->orion_errors.reserved3;
                    can_data.orion_errors.reserved4 = msg_data_a->orion_errors.reserved4;
                    can_data.orion_errors.reserved5 = msg_data_a->orion_errors.reserved5;
                    can_data.orion_errors.reserved6 = msg_data_a->orion_errors.reserved6;
                    can_data.orion_errors.reserved7 = msg_data_a->orion_errors.reserved7;
                    can_data.orion_errors.internal_comms = msg_data_a->orion_errors.internal_comms;
                    can_data.orion_errors.cell_balancing_foff = msg_data_a->orion_errors.cell_balancing_foff;
                    can_data.orion_errors.weak_cell = msg_data_a->orion_errors.weak_cell;
                    can_data.orion_errors.low_cellv = msg_data_a->orion_errors.low_cellv;
                    can_data.orion_errors.open_wire = msg_data_a->orion_errors.open_wire;
                    can_data.orion_errors.current_sensor = msg_data_a->orion_errors.current_sensor;
                    can_data.orion_errors.max_cellv_o5v = msg_data_a->orion_errors.max_cellv_o5v;
                    can_data.orion_errors.cell_asic = msg_data_a->orion_errors.cell_asic;
                    can_data.orion_errors.weak_pack = msg_data_a->orion_errors.weak_pack;
                    can_data.orion_errors.fan_monitor = msg_data_a->orion_errors.fan_monitor;
                    can_data.orion_errors.thermistor = msg_data_a->orion_errors.thermistor;
                    can_data.orion_errors.external_comms = msg_data_a->orion_errors.external_comms;
                    can_data.orion_errors.redundant_psu = msg_data_a->orion_errors.redundant_psu;
                    can_data.orion_errors.hv_isolation = msg_data_a->orion_errors.hv_isolation;
                    can_data.orion_errors.input_psu = msg_data_a->orion_errors.input_psu;
                    can_data.orion_errors.charge_limit_enforce = msg_data_a->orion_errors.charge_limit_enforce;
                    can_data.orion_errors.stale = 0;
                    can_data.orion_errors.last_rx = sched.os_ticks;
                    break;
                case ID_PRECHARGE_BL_CMD:
                    can_data.precharge_bl_cmd.cmd = msg_data_a->precharge_bl_cmd.cmd;
                    can_data.precharge_bl_cmd.data = msg_data_a->precharge_bl_cmd.data;
                    precharge_bl_cmd_CALLBACK(msg_data_a);
                    break;
                case ID_FAULT_SYNC_MAIN_MODULE:
                    can_data.fault_sync_main_module.idx = msg_data_a->fault_sync_main_module.idx;
                    can_data.fault_sync_main_module.latched = msg_data_a->fault_sync_main_module.latched;
                    fault_sync_main_module_CALLBACK(msg_data_a);
                    break;
                case ID_FAULT_SYNC_DRIVELINE:
                    can_data.fault_sync_driveline.idx = msg_data_a->fault_sync_driveline.idx;
                    can_data.fault_sync_driveline.latched = msg_data_a->fault_sync_driveline.latched;
                    fault_sync_driveline_CALLBACK(msg_data_a);
                    break;
                case ID_FAULT_SYNC_DASHBOARD:
                    can_data.fault_sync_dashboard.idx = msg_data_a->fault_sync_dashboard.idx;
                    can_data.fault_sync_dashboard.latched = msg_data_a->fault_sync_dashboard.latched;
                    fault_sync_dashboard_CALLBACK(msg_data_a);
                    break;
                case ID_FAULT_SYNC_TORQUE_VECTOR:
                    can_data.fault_sync_torque_vector.idx = msg_data_a->fault_sync_torque_vector.idx;
                    can_data.fault_sync_torque_vector.latched = msg_data_a->fault_sync_torque_vector.latched;
                    fault_sync_torque_vector_CALLBACK(msg_data_a);
                    break;
                case ID_FAULT_SYNC_TEST_NODE:
                    can_data.fault_sync_test_node.idx = msg_data_a->fault_sync_test_node.idx;
                    can_data.fault_sync_test_node.latched = msg_data_a->fault_sync_test_node.latched;
                    fault_sync_test_node_CALLBACK(msg_data_a);
                    break;
                case ID_SET_FAULT:
                    can_data.set_fault.id = msg_data_a->set_fault.id;
                    can_data.set_fault.value = msg_data_a->set_fault.value;
                    set_fault_CALLBACK(msg_data_a);
                    break;
                case ID_RETURN_FAULT_CONTROL:
                    can_data.return_fault_control.id = msg_data_a->return_fault_control.id;
                    return_fault_control_CALLBACK(msg_data_a);
                    break;
                case ID_DAQ_COMMAND_PRECHARGE:
                    can_data.daq_command_PRECHARGE.daq_command = msg_data_a->daq_command_PRECHARGE.daq_command;
                    daq_command_PRECHARGE_CALLBACK(&msg_header);
                    break;
                default:
                    __asm__("nop");
            }
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.elcon_charger_status.stale,
                sched.os_ticks, can_data.elcon_charger_status.last_rx,
                UP_ELCON_CHARGER_STATUS);
    CHECK_STALE(can_data.orion_info.stale,
                sched.os_ticks, can_data.orion_info.last_rx,
                UP_ORION_INFO);
    CHECK_STALE(can_data.orion_currents_volts.stale,
                sched.os_ticks, can_data.orion_currents_volts.last_rx,
                UP_ORION_CURRENTS_VOLTS);
    CHECK_STALE(can_data.orion_errors.stale,
                sched.os_ticks, can_data.orion_errors.last_rx,
                UP_ORION_ERRORS);
    /* END AUTO STALE CHECKS */
}

bool initCANFilter()
{
    CAN1->MCR |= CAN_MCR_INRQ;                // Enter back into INIT state (required for changing scale)             
    uint32_t timeout = 0;
    while( !(CAN1->MSR & CAN_MSR_INAK)
           && ++timeout < PHAL_CAN_INIT_TIMEOUT);
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
         return false;


    CAN1->FMR |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    /** 
     * Configure the CAN2 start bank.
     *  There are 28 total filter banks that are shared between CAN1 and CAN2.
     *  The CAN2SB field indicates where the split between CAN1 and 2 is.
     *  A value of 14 means that 0...13 are for CAN1 and 14...27 are for CAN2.
     * 
     * Make sure that all CAN filter configuration is done with the CAN1 peripheral.
     * CAN2 does not have access to modify/view the filters. 
     */ 
    CAN1->FMR &= ~CAN_FMR_CAN2SB;
    CAN1->FMR |= (14 << CAN_FMR_CAN2SB_Pos); // Set 0..13 for CAN1 and 14..27 for CAN2
    CAN1->FM1R |= 0x07FFFFFF;                 // Set banks 0-27 to id mode
    CAN1->FS1R |= 0x07FFFFFF;                 // Set banks 0-27 to 32-bit scale

    /* BEGIN AUTO FILTER */
    CAN1->FA1R |= (1 << 14);    // configure bank 14
    CAN1->sFilterRegister[14].FR1 = (ID_MODULE_TEMP_0 << 3) | 4;
    CAN1->sFilterRegister[14].FR2 = (ID_MODULE_TEMP_1 << 3) | 4;
    CAN1->FA1R |= (1 << 15);    // configure bank 15
    CAN1->sFilterRegister[15].FR1 = (ID_MODULE_TEMP_2 << 3) | 4;
    CAN1->sFilterRegister[15].FR2 = (ID_MODULE_TEMP_3 << 3) | 4;
    CAN1->FA1R |= (1 << 16);    // configure bank 16
    CAN1->sFilterRegister[16].FR1 = (ID_MODULE_TEMP_4 << 3) | 4;
    CAN1->sFilterRegister[16].FR2 = (ID_MODULE_TEMP_5 << 3) | 4;
    CAN1->FA1R |= (1 << 17);    // configure bank 17
    CAN1->sFilterRegister[17].FR1 = (ID_MODULE_TEMP_6 << 3) | 4;
    CAN1->sFilterRegister[17].FR2 = (ID_MODULE_TEMP_7 << 3) | 4;
    CAN1->FA1R |= (1 << 18);    // configure bank 18
    CAN1->sFilterRegister[18].FR1 = (ID_MODULE_TEMP_8 << 3) | 4;
    CAN1->sFilterRegister[18].FR2 = (ID_MODULE_TEMP_9 << 3) | 4;
    CAN1->FA1R |= (1 << 19);    // configure bank 19
    CAN1->sFilterRegister[19].FR1 = (ID_MODULE_TEMP_10 << 3) | 4;
    CAN1->sFilterRegister[19].FR2 = (ID_MODULE_TEMP_11 << 3) | 4;
    CAN1->FA1R |= (1 << 20);    // configure bank 20
    CAN1->sFilterRegister[20].FR1 = (ID_MODULE_TEMP_12 << 3) | 4;
    CAN1->sFilterRegister[20].FR2 = (ID_MODULE_TEMP_13 << 3) | 4;
    CAN1->FA1R |= (1 << 21);    // configure bank 21
    CAN1->sFilterRegister[21].FR1 = (ID_MODULE_TEMP_14 << 3) | 4;
    CAN1->sFilterRegister[21].FR2 = (ID_MODULE_TEMP_15 << 3) | 4;
    CAN1->FA1R |= (1 << 0);    // configure bank 0
    CAN1->sFilterRegister[0].FR1 = (ID_ELCON_CHARGER_STATUS << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_ORION_INFO << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_ORION_CURRENTS_VOLTS << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_ORION_ERRORS << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_PRECHARGE_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_FAULT_SYNC_MAIN_MODULE << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_FAULT_SYNC_DRIVELINE << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_FAULT_SYNC_DASHBOARD << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_FAULT_SYNC_TORQUE_VECTOR << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_FAULT_SYNC_TEST_NODE << 3) | 4;
    CAN1->FA1R |= (1 << 5);    // configure bank 5
    CAN1->sFilterRegister[5].FR1 = (ID_SET_FAULT << 3) | 4;
    CAN1->sFilterRegister[5].FR2 = (ID_RETURN_FAULT_CONTROL << 3) | 4;
    CAN1->FA1R |= (1 << 6);    // configure bank 6
    CAN1->sFilterRegister[6].FR1 = (ID_DAQ_COMMAND_PRECHARGE << 3) | 4;
    /* END AUTO FILTER */
    
    CAN1->FMR &= ~CAN_FMR_FINIT;       // Enable Filters (exit filter init mode)
    CAN1->MCR &= ~CAN_MCR_INRQ;        // Enter back into NORMAL mode

    while((CAN1->MSR & CAN_MSR_INAK)
            && ++timeout < PHAL_CAN_INIT_TIMEOUT);
    return timeout != PHAL_CAN_INIT_TIMEOUT;
}


void canProcessRxIRQs(CanMsgTypeDef_t* rx)
{
    CanParsedData_t* msg_data_a;

    msg_data_a = (CanParsedData_t *) rx->Data;
    switch(rx->ExtId)
    {
        /* BEGIN AUTO RX IRQ */
        /* END AUTO RX IRQ */
        default:
            __asm__("nop");
    }
}
