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

    if (enabled) __reload_watchdog();
    cmd_write(0, CMD_ADDRESS_ENABLE, (enabled) ? LOAD_ENABLE_KEY : 0x0000);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CC level
void load_set_cc_level(uint16_t current_ma) {

    cmd_write(0, CMD_ADDRESS_CC_LEVEL, current_ma);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
