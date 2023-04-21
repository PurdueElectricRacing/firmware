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
        switch(msg_header.ExtId)
        {
            case ID_MAIN_HB:
                can_data.main_hb.car_state = msg_data_a->main_hb.car_state;
                can_data.main_hb.precharge_state = msg_data_a->main_hb.precharge_state;
                can_data.main_hb.stale = 0;
                can_data.main_hb.last_rx = sched.os_ticks;
                break;
            case ID_REAR_WHEEL_DATA:
                can_data.rear_wheel_data.left_speed = msg_data_a->rear_wheel_data.left_speed;
                can_data.rear_wheel_data.right_speed = msg_data_a->rear_wheel_data.right_speed;
                can_data.rear_wheel_data.left_normal = msg_data_a->rear_wheel_data.left_normal;
                can_data.rear_wheel_data.right_normal = msg_data_a->rear_wheel_data.right_normal;
                can_data.rear_wheel_data.stale = 0;
                can_data.rear_wheel_data.last_rx = sched.os_ticks;
                break;
            case ID_REAR_MOTOR_CURRENTS_TEMPS:
                can_data.rear_motor_currents_temps.left_current = msg_data_a->rear_motor_currents_temps.left_current;
                can_data.rear_motor_currents_temps.right_current = msg_data_a->rear_motor_currents_temps.right_current;
                can_data.rear_motor_currents_temps.left_temp = msg_data_a->rear_motor_currents_temps.left_temp;
                can_data.rear_motor_currents_temps.right_temp = msg_data_a->rear_motor_currents_temps.right_temp;
                can_data.rear_motor_currents_temps.right_voltage = msg_data_a->rear_motor_currents_temps.right_voltage;
                can_data.rear_motor_currents_temps.stale = 0;
                can_data.rear_motor_currents_temps.last_rx = sched.os_ticks;
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
            case ID_MAX_CELL_TEMP:
                can_data.max_cell_temp.max_temp = msg_data_a->max_cell_temp.max_temp;
                can_data.max_cell_temp.stale = 0;
                can_data.max_cell_temp.last_rx = sched.os_ticks;
                break;
            case ID_REAR_CONTROLLER_TEMPS:
                can_data.rear_controller_temps.left_temp = msg_data_a->rear_controller_temps.left_temp;
                can_data.rear_controller_temps.right_temp = msg_data_a->rear_controller_temps.right_temp;
                can_data.rear_controller_temps.stale = 0;
                can_data.rear_controller_temps.last_rx = sched.os_ticks;
                break;
            case ID_PRECHARGE_HB:
                can_data.precharge_hb.IMD = msg_data_a->precharge_hb.IMD;
                can_data.precharge_hb.BMS = msg_data_a->precharge_hb.BMS;
                can_data.precharge_hb.stale = 0;
                can_data.precharge_hb.last_rx = sched.os_ticks;
                break;
            case ID_TORQUE_REQUEST_MAIN:
                can_data.torque_request_main.front_left = (int16_t) msg_data_a->torque_request_main.front_left;
                can_data.torque_request_main.front_right = (int16_t) msg_data_a->torque_request_main.front_right;
                can_data.torque_request_main.rear_left = (int16_t) msg_data_a->torque_request_main.rear_left;
                can_data.torque_request_main.rear_right = (int16_t) msg_data_a->torque_request_main.rear_right;
                can_data.torque_request_main.stale = 0;
                can_data.torque_request_main.last_rx = sched.os_ticks;
                break;
            case ID_REAR_WHEEL_SPEEDS:
                can_data.rear_wheel_speeds.left_speed_mc = msg_data_a->rear_wheel_speeds.left_speed_mc;
                can_data.rear_wheel_speeds.right_speed_mc = msg_data_a->rear_wheel_speeds.right_speed_mc;
                can_data.rear_wheel_speeds.left_speed_sensor = msg_data_a->rear_wheel_speeds.left_speed_sensor;
                can_data.rear_wheel_speeds.right_speed_sensor = msg_data_a->rear_wheel_speeds.right_speed_sensor;
                can_data.rear_wheel_speeds.stale = 0;
                can_data.rear_wheel_speeds.last_rx = sched.os_ticks;
                break;
            case ID_FLOWRATE_TEMPS:
                can_data.flowrate_temps.battery_in_temp = (int8_t) msg_data_a->flowrate_temps.battery_in_temp;
                can_data.flowrate_temps.battery_out_temp = (int8_t) msg_data_a->flowrate_temps.battery_out_temp;
                can_data.flowrate_temps.drivetrain_in_temp = (int8_t) msg_data_a->flowrate_temps.drivetrain_in_temp;
                can_data.flowrate_temps.drivetrain_out_temp = (int8_t) msg_data_a->flowrate_temps.drivetrain_out_temp;
                can_data.flowrate_temps.battery_flowrate = msg_data_a->flowrate_temps.battery_flowrate;
                can_data.flowrate_temps.drivetrain_flowrate = msg_data_a->flowrate_temps.drivetrain_flowrate;
                can_data.flowrate_temps.battery_fan_speed = msg_data_a->flowrate_temps.battery_fan_speed;
                can_data.flowrate_temps.drivetrain_fan_speed = msg_data_a->flowrate_temps.drivetrain_fan_speed;
                can_data.flowrate_temps.stale = 0;
                can_data.flowrate_temps.last_rx = sched.os_ticks;
                break;
            case ID_COOLANT_OUT:
                can_data.coolant_out.bat_fan = msg_data_a->coolant_out.bat_fan;
                can_data.coolant_out.dt_fan = msg_data_a->coolant_out.dt_fan;
                can_data.coolant_out.bat_pump = msg_data_a->coolant_out.bat_pump;
                can_data.coolant_out.bat_pump_aux = msg_data_a->coolant_out.bat_pump_aux;
                can_data.coolant_out.dt_pump = msg_data_a->coolant_out.dt_pump;
                can_data.coolant_out.stale = 0;
                can_data.coolant_out.last_rx = sched.os_ticks;
                coolant_out_CALLBACK(msg_data_a);
                break;
            case ID_GEARBOX:
                can_data.gearbox.l_temp = (int8_t) msg_data_a->gearbox.l_temp;
                can_data.gearbox.r_temp = (int8_t) msg_data_a->gearbox.r_temp;
                can_data.gearbox.stale = 0;
                can_data.gearbox.last_rx = sched.os_ticks;
                break;
            case ID_DASHBOARD_BL_CMD:
                can_data.dashboard_bl_cmd.cmd = msg_data_a->dashboard_bl_cmd.cmd;
                can_data.dashboard_bl_cmd.data = msg_data_a->dashboard_bl_cmd.data;
                dashboard_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_FAULT_SYNC_MAIN_MODULE:
                can_data.fault_sync_main_module.idx = msg_data_a->fault_sync_main_module.idx;
                can_data.fault_sync_main_module.latched = msg_data_a->fault_sync_main_module.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_DRIVELINE:
                can_data.fault_sync_driveline.idx = msg_data_a->fault_sync_driveline.idx;
                can_data.fault_sync_driveline.latched = msg_data_a->fault_sync_driveline.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_PRECHARGE:
                can_data.fault_sync_precharge.idx = msg_data_a->fault_sync_precharge.idx;
                can_data.fault_sync_precharge.latched = msg_data_a->fault_sync_precharge.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_TORQUE_VECTOR:
                can_data.fault_sync_torque_vector.idx = msg_data_a->fault_sync_torque_vector.idx;
                can_data.fault_sync_torque_vector.latched = msg_data_a->fault_sync_torque_vector.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_TEST_NODE:
                can_data.fault_sync_test_node.idx = msg_data_a->fault_sync_test_node.idx;
                can_data.fault_sync_test_node.latched = msg_data_a->fault_sync_test_node.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_SET_FAULT:
                can_data.set_fault.id = msg_data_a->set_fault.id;
                can_data.set_fault.value = msg_data_a->set_fault.value;
				set_fault_daq(msg_data_a->set_fault.id, msg_data_a->set_fault.value);
                break;
            case ID_RETURN_FAULT_CONTROL:
                can_data.return_fault_control.id = msg_data_a->return_fault_control.id;
				return_fault_control(msg_data_a->return_fault_control.id);
                break;
            case ID_DAQ_COMMAND_DASHBOARD:
                can_data.daq_command_DASHBOARD.daq_command = msg_data_a->daq_command_DASHBOARD.daq_command;
                daq_command_DASHBOARD_CALLBACK(&msg_header);
                break;
            default:
                __asm__("nop");
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.main_hb.stale,
                sched.os_ticks, can_data.main_hb.last_rx,
                UP_MAIN_HB);
    CHECK_STALE(can_data.rear_wheel_data.stale,
                sched.os_ticks, can_data.rear_wheel_data.last_rx,
                UP_REAR_WHEEL_DATA);
    CHECK_STALE(can_data.rear_motor_currents_temps.stale,
                sched.os_ticks, can_data.rear_motor_currents_temps.last_rx,
                UP_REAR_MOTOR_CURRENTS_TEMPS);
    CHECK_STALE(can_data.orion_info.stale,
                sched.os_ticks, can_data.orion_info.last_rx,
                UP_ORION_INFO);
    CHECK_STALE(can_data.orion_currents_volts.stale,
                sched.os_ticks, can_data.orion_currents_volts.last_rx,
                UP_ORION_CURRENTS_VOLTS);
    CHECK_STALE(can_data.orion_errors.stale,
                sched.os_ticks, can_data.orion_errors.last_rx,
                UP_ORION_ERRORS);
    CHECK_STALE(can_data.max_cell_temp.stale,
                sched.os_ticks, can_data.max_cell_temp.last_rx,
                UP_MAX_CELL_TEMP);
    CHECK_STALE(can_data.rear_controller_temps.stale,
                sched.os_ticks, can_data.rear_controller_temps.last_rx,
                UP_REAR_CONTROLLER_TEMPS);
    CHECK_STALE(can_data.precharge_hb.stale,
                sched.os_ticks, can_data.precharge_hb.last_rx,
                UP_PRECHARGE_HB);
    CHECK_STALE(can_data.torque_request_main.stale,
                sched.os_ticks, can_data.torque_request_main.last_rx,
                UP_TORQUE_REQUEST_MAIN);
    CHECK_STALE(can_data.rear_wheel_speeds.stale,
                sched.os_ticks, can_data.rear_wheel_speeds.last_rx,
                UP_REAR_WHEEL_SPEEDS);
    CHECK_STALE(can_data.flowrate_temps.stale,
                sched.os_ticks, can_data.flowrate_temps.last_rx,
                UP_FLOWRATE_TEMPS);
    CHECK_STALE(can_data.coolant_out.stale,
                sched.os_ticks, can_data.coolant_out.last_rx,
                UP_COOLANT_OUT);
    CHECK_STALE(can_data.gearbox.stale,
                sched.os_ticks, can_data.gearbox.last_rx,
                UP_GEARBOX);
    /* END AUTO STALE CHECKS */
}

bool initCANFilter()
{
    CAN1->MCR |= CAN_MCR_INRQ;                // Enter back into INIT state (required for changing scale)
    uint32_t timeout = 0;
    while(!(CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
         ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
         return false;

    CAN1->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN1->FM1R |= 0x07FFFFFF;                 // Set banks 0-27 to id mode
    CAN1->FS1R |= 0x07FFFFFF;                 // Set banks 0-27 to 32-bit scale

    /* BEGIN AUTO FILTER */
    CAN1->FA1R |= (1 << 0);    // configure bank 0
    CAN1->sFilterRegister[0].FR1 = (ID_MAIN_HB << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_REAR_WHEEL_DATA << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_REAR_MOTOR_CURRENTS_TEMPS << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_ORION_INFO << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_ORION_CURRENTS_VOLTS << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_ORION_ERRORS << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_MAX_CELL_TEMP << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_REAR_CONTROLLER_TEMPS << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_PRECHARGE_HB << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_TORQUE_REQUEST_MAIN << 3) | 4;
    CAN1->FA1R |= (1 << 5);    // configure bank 5
    CAN1->sFilterRegister[5].FR1 = (ID_REAR_WHEEL_SPEEDS << 3) | 4;
    CAN1->sFilterRegister[5].FR2 = (ID_FLOWRATE_TEMPS << 3) | 4;
    CAN1->FA1R |= (1 << 6);    // configure bank 6
    CAN1->sFilterRegister[6].FR1 = (ID_COOLANT_OUT << 3) | 4;
    CAN1->sFilterRegister[6].FR2 = (ID_GEARBOX << 3) | 4;
    CAN1->FA1R |= (1 << 7);    // configure bank 7
    CAN1->sFilterRegister[7].FR1 = (ID_DASHBOARD_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[7].FR2 = (ID_FAULT_SYNC_MAIN_MODULE << 3) | 4;
    CAN1->FA1R |= (1 << 8);    // configure bank 8
    CAN1->sFilterRegister[8].FR1 = (ID_FAULT_SYNC_DRIVELINE << 3) | 4;
    CAN1->sFilterRegister[8].FR2 = (ID_FAULT_SYNC_PRECHARGE << 3) | 4;
    CAN1->FA1R |= (1 << 9);    // configure bank 9
    CAN1->sFilterRegister[9].FR1 = (ID_FAULT_SYNC_TORQUE_VECTOR << 3) | 4;
    CAN1->sFilterRegister[9].FR2 = (ID_FAULT_SYNC_TEST_NODE << 3) | 4;
    CAN1->FA1R |= (1 << 10);    // configure bank 10
    CAN1->sFilterRegister[10].FR1 = (ID_SET_FAULT << 3) | 4;
    CAN1->sFilterRegister[10].FR2 = (ID_RETURN_FAULT_CONTROL << 3) | 4;
    CAN1->FA1R |= (1 << 11);    // configure bank 11
    CAN1->sFilterRegister[11].FR1 = (ID_DAQ_COMMAND_DASHBOARD << 3) | 4;
    /* END AUTO FILTER */

    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
         ;

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
