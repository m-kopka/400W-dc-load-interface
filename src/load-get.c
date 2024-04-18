#include "load.h"

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

extern uint16_t load_register[CMD_REGISTER_COUNT];

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// returns the ENABLE flag in status register
bool load_get_enable(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_ENABLED));}

// returns the FAULT flag in status register
bool load_get_fault(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_FAULT));}

// returns the READY flag in status register
bool load_get_ready(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_READY));}

// returns the NO_REG flag in status register
bool load_get_not_in_reg(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_NO_REG));}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns current load mode (CC, CV, CR or CP)
load_mode_t load_get_mode(void) {return (load_register[CMD_ADDRESS_CONFIG] & 0x3);}

// returns the current VSEN source. 1 -> Remote, 0 -> Internal
bool load_get_vsensrc(void) {return (!!(load_register[CMD_ADDRESS_CONFIG] & LOAD_CONFIG_VSEN_SRC));}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the fault register
load_fault_t load_get_fault_flags(void) {return load_register[CMD_ADDRESS_FAULT];}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the fault mask register
load_fault_t load_get_fault_mask(void) {return load_register[CMD_ADDRESS_FAULT_MASK];}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns load CC level [mA]
uint32_t load_get_cc_level_ma(void) {return load_register[CMD_ADDRESS_CC_LEVEL];}

// returns load CV level [mV]
uint32_t load_get_cv_level_mv(void) {return load_register[CMD_ADDRESS_CV_LEVEL] * 10;}

// returns load CR level [mOhm]
uint32_t load_get_cr_level_mr(void) {return load_register[CMD_ADDRESS_CR_LEVEL] * 10;}

// returns load CP level [mW]
uint32_t load_get_cp_level_mw(void) {return load_register[CMD_ADDRESS_CP_LEVEL] * 100;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns load input voltage [mV]
uint32_t load_get_voltage_mv(void) {return load_register[CMD_ADDRESS_VOLTAGE] * 10;}

// returns load total current [mA]
uint32_t load_get_total_current_ma(void) {return load_register[CMD_ADDRESS_CURRENT];}

// returns the total load power [mW]
uint32_t load_get_power_mw(void) {return load_register[CMD_ADDRESS_POWER] * 100;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns total load enable time in [s]
uint32_t load_get_ena_time(void) {return ((load_register[CMD_ADDRESS_TOTAL_TIME_H] << 16) | load_register[CMD_ADDRESS_TOTAL_TIME_L]);}

// returns total load consumed current [mAh]
uint32_t load_get_total_mah(void) {return ((load_register[CMD_ADDRESS_TOTAL_MAH_H] << 16) | load_register[CMD_ADDRESS_TOTAL_MAH_L]);}

// returns total load consumed energy [mWh]
uint32_t load_get_total_mwh(void) {return ((load_register[CMD_ADDRESS_TOTAL_MWH_H] << 16) | load_register[CMD_ADDRESS_TOTAL_MWH_L]);}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns the load temperature [°C]
uint8_t load_get_temp(void) {
    
    if (load_register[CMD_ADDRESS_TEMP_L] > load_register[CMD_ADDRESS_TEMP_R]) return (load_register[CMD_ADDRESS_TEMP_L]);
    else return load_register[CMD_ADDRESS_TEMP_R];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
