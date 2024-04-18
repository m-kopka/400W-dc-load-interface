#include "load.h"

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

extern uint16_t load_register[CMD_REGISTER_COUNT];

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

void __reload_watchdog(void);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// clears specified load fault flags
void load_clear_fault(load_fault_t faults) {

    cmd_write(0, CMD_ADDRESS_FAULT, faults);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the fault mask register
void load_set_fault_mask(load_fault_t mask) {

    cmd_write(0, CMD_ADDRESS_FAULT_MASK, mask);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load mode (CC, CV, CR or CP)
void load_set_mode(load_mode_t mode) {

    uint16_t config_reg = load_register[CMD_ADDRESS_CONFIG];        // get current config register value
    config_reg &= ~0x3;                                             // clear mode bits
    config_reg |= (mode & 0x3);                                     // set new mode bits
    
    cmd_write(0, CMD_ADDRESS_CONFIG, config_reg);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// enables or disables the load
void load_set_enable(bool enabled) {

    if (enabled) {
        
        __reload_watchdog();
        load_clear_fault(LOAD_FAULT_COM);
    }
    cmd_write(0, CMD_ADDRESS_ENABLE, (enabled) ? LOAD_ENABLE_KEY : 0x0000);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CC level
void load_set_cc_level(uint32_t current_ma) {

    if (current_ma < LOAD_MIN_CC_LEVEL_MA) return;
    if (current_ma > LOAD_MAX_CC_LEVEL_MA) return;

    cmd_write(0, CMD_ADDRESS_CC_LEVEL, current_ma);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CV level
void load_set_cv_level(uint32_t voltage_mv) {

    if (voltage_mv < LOAD_MIN_CV_LEVEL_MV) return;
    if (voltage_mv > LOAD_MAX_CV_LEVEL_MV) return;

    cmd_write(0, CMD_ADDRESS_CV_LEVEL, voltage_mv / 10);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CR level
void load_set_cr_level(uint32_t resistance_mr) {

    if (resistance_mr < LOAD_MIN_CR_LEVEL_MR) return;
    if (resistance_mr > LOAD_MAX_CR_LEVEL_MR) return;

    cmd_write(0, CMD_ADDRESS_CR_LEVEL, resistance_mr);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CP level
void load_set_cp_level(uint32_t power_mw) {

    if (power_mw < LOAD_MIN_CP_LEVEL_MW) return;
    if (power_mw > LOAD_MAX_CP_LEVEL_MW) return;

    cmd_write(0, CMD_ADDRESS_CP_LEVEL, power_mw / 100);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// enables or disables voltage remote sensing
void load_set_remote_sense(bool enabled) {

    uint16_t config_reg = load_register[CMD_ADDRESS_CONFIG];        // get current config register value
    
    if (enabled) config_reg |= LOAD_CONFIG_VSEN_SRC;
    else config_reg &= ~LOAD_CONFIG_VSEN_SRC;
    
    cmd_write(0, CMD_ADDRESS_CONFIG, config_reg);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// enables or disables automatic voltage sense source switching
void load_set_auto_vsensrc(bool enabled) {

    uint16_t config_reg = load_register[CMD_ADDRESS_CONFIG];        // get current config register value
    
    if (enabled) config_reg |= LOAD_CONFIG_AUTO_VSEN_SRC;
    else config_reg &= ~LOAD_CONFIG_AUTO_VSEN_SRC;
    
    cmd_write(0, CMD_ADDRESS_CONFIG, config_reg);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the auto disable voltage level
void load_set_discharge_voltage(uint32_t voltage_mv) {

    if (voltage_mv < LOAD_MIN_CV_LEVEL_MV) return;
    if (voltage_mv > LOAD_MAX_CV_LEVEL_MV) return;

    cmd_write(0, CMD_ADDRESS_DISCH_LEVEL, voltage_mv / 10);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
