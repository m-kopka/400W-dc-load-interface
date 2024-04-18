#ifndef _LOAD_H_
#define _LOAD_H_

/*
 *  Load CMD SPI Interface Abstraction Layer
 *  Martin Kopka 2024
*/

#include "common_defs.h"
#include "cmd_spi_driver.h"

//---- INTERFACE CONTROL FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------

// kernel task for parsing data from slave modules and handling communication faults
void load_task(void);

// returns number of populated SPI slots (only devices which responded with valid ID count)
uint8_t load_get_module_count(void);

// starts parsing data from the slave module
void load_start_data_capture(void);

// returns true if the data parsing is done
bool load_get_data_capture_done(void);

// retuns true if the slave keeps responding with invalid checksum or doesn't respond at all
bool load_get_checksum_fault(void);

//---- LOAD SET FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// clears specified load fault flags
void load_clear_fault(load_fault_t faults);

// sets the fault mask register
void load_set_fault_mask(load_fault_t mask);

// sets the load mode (CC, CV, CR or CP)
void load_set_mode(load_mode_t mode);

// enables or disables the load
void load_set_enable(bool enabled);

// sets the load CC level
void load_set_cc_level(uint16_t current_ma);

//---- LOAD GET FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// returns the ENABLE flag in status register
bool load_get_enable(void);

// returns the FAULT flag in status register
bool load_get_fault(void);

// returns the READY flag in status register
bool load_get_ready(void);

// returns the NO_REG flag in status register
bool load_get_not_in_reg(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns current load mode (CC, CV, CR or CP)
load_mode_t load_get_mode(void);

// returns the current VSEN source. 1 -> Remote, 0 -> Internal
bool load_get_vsensrc(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the fault register
load_fault_t load_get_fault_flags(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the fault mask register
load_fault_t load_get_fault_mask(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns load CC level [mA]
uint32_t load_get_cc_level_ma(void);

// returns load CV level [mV]
uint32_t load_get_cv_level_mv(void);

// returns load CR level [mOhm]
uint32_t load_get_cr_level_mr(void);

// returns load CP level [mW]
uint32_t load_get_cp_level_mw(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the load input voltage [mV]
uint32_t load_get_voltage_mv(void);

// returns the total load current [mA]
uint32_t load_get_total_current_ma(void);

// returns the total load power [mW]
uint32_t load_get_power_mw(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns time since load enable [s]
uint32_t load_get_ena_time(void);

// returns total charge consumed since enable [mAh]
uint32_t load_get_total_mah(void);

// returns total energy consumed since enable [mWh]
uint32_t load_get_total_mwh(void);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the load temperature [°C]
uint8_t load_get_temp(void);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _LOAD_H_ */