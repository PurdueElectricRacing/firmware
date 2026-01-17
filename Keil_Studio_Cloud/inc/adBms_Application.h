/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adBms_Application.h
* @brief:   Bms application header file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! @addtogroup APPLICATION
*  @{
*
*/

/*! @addtogroup APPLICATION
*  @{
*
*/

#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <stdint.h>
#include "adbms_main.h"

void app_main(void);
void run_command(int cmd);
void adBms6830_init_config(uint8_t tIC, cell_asic *ic);
void adBms6830_write_read_config(uint8_t tIC, cell_asic *ic);
void adBms6830_write_config(uint8_t tIC, cell_asic *ic);
void adBms6830_read_config(uint8_t tIC, cell_asic *ic);
void adBms6830_start_adc_cell_voltage_measurment(uint8_t tIC);
void adBms6830_read_cell_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_start_adc_s_voltage_measurment(uint8_t tIC);
void adBms6830_read_s_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_start_avgcell_voltage_measurment(uint8_t tIC);
void adBms6830_read_avgcell_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_start_fcell_voltage_measurment(uint8_t tIC);
void adBms6830_read_fcell_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_start_aux_voltage_measurment(uint8_t tIC, cell_asic *ic);
void adBms6830_read_aux_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_start_raux_voltage_measurment(uint8_t tIC, cell_asic *ic);
void adBms6830_read_raux_voltages(uint8_t tIC, cell_asic *ic);
void adBms6830_read_status_registers(uint8_t tIC, cell_asic *ic);
void measurement_loop(void);
void adBms6830_read_device_sid(uint8_t tIC, cell_asic *ic);
void adBms6830_set_reset_gpio_pins(uint8_t tIC, cell_asic *ic);
void adBms6830_enable_mute(uint8_t tIC, cell_asic *ic);
void adBms6830_disable_mute(uint8_t tIC, cell_asic *ic);
void adBms6830_soft_reset(uint8_t tIC);
void adBms6830_reset_cmd_count(uint8_t tIC);
void adBms6830_reset_pec_error_flag(uint8_t tIC, cell_asic *ic);
void adBms6830_snap(uint8_t tIC);
void adBms6830_unsnap(uint8_t tIC);
void adBms6830_clear_cell_measurement(uint8_t tIC);
void adBms6830_clear_aux_measurement(uint8_t tIC);
void adBms6830_clear_spin_measurement(uint8_t tIC);
void adBms6830_clear_fcell_measurement(uint8_t tIC);
void adBms6830_clear_ovuv_measurement(uint8_t tIC);
void adBms6830_clear_all_flags(uint8_t tIC, cell_asic *ic);
void adBms6830_set_dcc_discharge(uint8_t tIC, cell_asic *ic);
void adBms6830_clear_dcc_discharge(uint8_t tIC, cell_asic *ic);
void adBms6830_write_read_pwm_duty_cycle(uint8_t tIC, cell_asic *ic);
void adBms6830_gpio_spi_communication(uint8_t tIC, cell_asic *ic);
void adBms6830_gpio_i2c_write_to_slave(uint8_t tIC, cell_asic *ic);
void adBms6830_gpio_i2c_read_from_slave(uint8_t tIC, cell_asic *ic);
void adBms6830_set_dtrng_dcto_value(uint8_t tIC, cell_asic *ic);
void adBms6830_run_osc_mismatch_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_thermal_shutdown_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_supply_error_detection_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_thermal_shutdown_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_fuse_ed_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_fuse_med_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_run_tmodchk_self_test(uint8_t tIC, cell_asic *ic);
void adBms6830_check_latent_fault_csflt_status_bits(uint8_t tIC, cell_asic *ic);
void adBms6830_check_rdstatc_err_bit_functionality(uint8_t tIC, cell_asic *ic);
void adBms6830_cell_openwire_test(uint8_t tIC, cell_asic *ic);
void adBms6830_redundant_cell_openwire_test(uint8_t tIC, cell_asic *ic);
void adBms6830_cell_ow_volatge_collect(uint8_t tIC, cell_asic *ic, TYPE type, OW_C_S ow_c_s);
void adBms6830_aux_openwire_test(uint8_t tIC, cell_asic *ic);
void adBms6830_gpio_pup_up_down_volatge_collect(uint8_t tIC, cell_asic *ic, PUP pup);
void adBms6830_open_wire_detection_condtion_check(uint8_t tIC, cell_asic *ic, TYPE type);
void adBms6830_read_rdcvall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdacall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdsall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdfcall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdcsall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdacsall_voltage(uint8_t tIC, cell_asic *ic);
void adBms6830_read_rdasall_voltage(uint8_t tIC, cell_asic *ic);

#endif
/** @}*/
/** @}*/