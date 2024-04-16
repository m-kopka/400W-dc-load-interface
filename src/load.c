#include "load.h"

typedef enum {

    LOAD_COM_STATE_GETTING_ID,
    LOAD_COM_STATE_WAITING_FOR_READY,
    LOAD_COM_STATE_READY

} load_communication_state_t;

load_communication_state_t com_state;
bool data_capture_started = true;
kernel_time_t prev_watchdog_reload_time = 0;
uint16_t load_register[CMD_REGISTER_COUNT] = {0};
static uint8_t queue_pos = 0;
static bool checksum_fault = false;


int8_t read_queue[] = {

    CMD_ADDRESS_STATUS,
    CMD_ADDRESS_FAULT,
    CMD_ADDRESS_CONFIG,
    CMD_ADDRESS_CC_LEVEL,
    CMD_ADDRESS_VOLTAGE,
    CMD_ADDRESS_CURRENT,
    CMD_ADDRESS_TEMP_L,
    CMD_ADDRESS_TEMP_R,
    CMD_ADDRESS_TOTAL_TIME_L,
    CMD_ADDRESS_TOTAL_MAH_L,
    CMD_ADDRESS_TOTAL_MWH_L
};

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

void __update_register(uint8_t reg_address);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void load_task(void) {

    cmd_driver_init();

    while (1) {

        bool checksum_ok = false;

        switch (com_state) {

            case LOAD_COM_STATE_GETTING_ID:

                if (cmd_read(0, CMD_ADDRESS_ID, &checksum_ok) == LOAD_ID_CODE && checksum_ok) com_state = LOAD_COM_STATE_WAITING_FOR_READY;
                break;

            case LOAD_COM_STATE_WAITING_FOR_READY:

                load_register[CMD_ADDRESS_STATUS] = cmd_read(0, CMD_ADDRESS_STATUS, &checksum_ok);

                if (checksum_ok && load_get_ready()) com_state = LOAD_COM_STATE_READY;
                break;

            case LOAD_COM_STATE_READY:

                if (load_get_enable() && kernel_get_time_since(prev_watchdog_reload_time) >= LOAD_WD_TIMEOUT_MS / 2) {

                    prev_watchdog_reload_time = kernel_get_time_ms();
                    cmd_write(0, CMD_ADDRESS_WD_RELOAD, LOAD_WD_RELOAD_KEY);
                }

                if (data_capture_started) {

                    __update_register(read_queue[queue_pos++]);

                    if (queue_pos == sizeof(read_queue)) {
                        
                        queue_pos = 0;
                        data_capture_started = false;
                    }
                }

                break;
        }

        kernel_yield();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void load_start_data_capture(void) {

    data_capture_started = true;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool load_get_data_capture_done(void) {

    return !data_capture_started;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// enables or disables the load
void load_set_enable(bool enabled) {

    if (enabled) {
        
        prev_watchdog_reload_time = kernel_get_time_ms();
        cmd_write(0, CMD_ADDRESS_WD_RELOAD, LOAD_WD_RELOAD_KEY);
        cmd_write(0, CMD_ADDRESS_ENABLE, LOAD_ENABLE_KEY);

    } else cmd_write(0, CMD_ADDRESS_ENABLE, 0x0000);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets the load CC level
void load_set_cc_level(uint16_t current_ma) {

    cmd_write(0, CMD_ADDRESS_CC_LEVEL, current_ma);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns true if the load is ready
bool load_get_ready(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_READY));}

// returns true if the load is enabled
bool load_get_enable(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_ENABLED));}

load_mode_t load_get_mode(void) {return (load_register[CMD_ADDRESS_CONFIG] & 0x3);}

// returns current load voltage sense source
bool load_get_vsensrc(void) {return (!!(load_register[CMD_ADDRESS_CONFIG] & LOAD_CONFIG_VSEN_SRC));}

// returns true if the load is not in regulation (measured current doesn't match CC level)
bool load_get_not_in_reg(void) {return (!!(load_register[CMD_ADDRESS_STATUS] & LOAD_STATUS_NO_REG));}

// returns load fault flags
load_fault_t load_get_faults(void) {return load_register[CMD_ADDRESS_FAULT];}

// returns load CC level [mA]
uint32_t load_get_cc_level_ma(void) {return load_register[CMD_ADDRESS_CC_LEVEL];}

// returns load input voltage [mV]
uint32_t load_get_voltage_mv(void) {return load_register[CMD_ADDRESS_VOLTAGE] * 10;}

// returns load total current [mA]
uint32_t load_get_total_current_ma(void) {return load_register[CMD_ADDRESS_CURRENT];}

// returns the load temperature
uint8_t load_get_temp(void) {
    
    if (load_register[CMD_ADDRESS_TEMP_L] > load_register[CMD_ADDRESS_TEMP_R]) return (load_register[CMD_ADDRESS_TEMP_L]);
    else return load_register[CMD_ADDRESS_TEMP_R];
}

// returns total load enable time in [s]
uint32_t load_get_ena_time(void) {return ((load_register[CMD_ADDRESS_TOTAL_TIME_H] << 16) | load_register[CMD_ADDRESS_TOTAL_TIME_L]);}

// returns total load consumed current [mAh]
uint32_t load_get_total_mah(void) {return ((load_register[CMD_ADDRESS_TOTAL_MAH_H] << 16) | load_register[CMD_ADDRESS_TOTAL_MAH_L]);}

// returns total load consumed energy [mWh]
uint32_t load_get_total_mwh(void) {return ((load_register[CMD_ADDRESS_TOTAL_MWH_H] << 16) | load_register[CMD_ADDRESS_TOTAL_MWH_L]);}

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// reads data from the load slave module and updates the register if checksum is ok
// triggers a checksum fault after 16 consecutive checksum fails
void __update_register(uint8_t reg_address) {

    static uint32_t checksum_fail_cumulative_counter = 0;
    bool checksum_ok = false; 
    uint16_t value = cmd_read(0, reg_address, &checksum_ok);   // read from load module

    // update the register if checksum ok
    if (checksum_ok) {
        
        load_register[reg_address] = value;

        if (checksum_fail_cumulative_counter > 0) {

            checksum_fail_cumulative_counter--;
            checksum_fault = false;
        }

    } else if (!checksum_fault) {   // increment cumulative counter if checksum check failed

        if (++checksum_fail_cumulative_counter == 16) {

            checksum_fault = true;
            com_state = LOAD_COM_STATE_GETTING_ID;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
