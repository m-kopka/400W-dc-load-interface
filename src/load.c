#include "load.h"

//---- ENUMERATIONS ----------------------------------------------------------------------------------------------------------------------------------------------

// communication state machine
typedef enum {

    LOAD_COM_STATE_GETTING_ID,              // waiting for slave to respond with valid ID
    LOAD_COM_STATE_WAITING_FOR_READY,       // waiting for slave to complete its power-up selftest
    LOAD_COM_STATE_READY                    // slave ready, parsing data

} load_communication_state_t;

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

uint8_t module_count = 0;                       // number of detected slave modules
load_communication_state_t com_state;           // current communication state machine state
bool data_capture_started = true;               // after set reun, kernel task starts parsing data from slave. Automatically set false after parsing is done
static bool checksum_fault = false;             // true if the slave keeps responding with invalid checksum or doesn't respond at all
kernel_time_t prev_watchdog_reload_time = 0;    // absolute time of previous load watchdog reload. Used for reloading the watchdog in regular intervals while the load is enabled

uint16_t load_register[CMD_REGISTER_COUNT] = {0};       // stores parsed data
static uint8_t read_queue_pos = 0;                      // current position in the read queue

// list of load registers to be parsed each data capture
int8_t read_queue[] = {

    CMD_ADDRESS_STATUS,
    CMD_ADDRESS_FAULT,
    CMD_ADDRESS_FAULT_MASK,
    CMD_ADDRESS_CONFIG,
    CMD_ADDRESS_CC_LEVEL,
    CMD_ADDRESS_CV_LEVEL,
    CMD_ADDRESS_CR_LEVEL,
    CMD_ADDRESS_CP_LEVEL,
    CMD_ADDRESS_VOLTAGE,
    CMD_ADDRESS_CURRENT,
    CMD_ADDRESS_POWER,
    CMD_ADDRESS_TEMP_L,
    CMD_ADDRESS_TEMP_R,
    CMD_ADDRESS_FAN_RPM1,
    CMD_ADDRESS_FAN_RPM2,
    CMD_ADDRESS_TOTAL_TIME_L,
    CMD_ADDRESS_TOTAL_TIME_H,
    CMD_ADDRESS_TOTAL_MAH_L,
    CMD_ADDRESS_TOTAL_MAH_H,
    CMD_ADDRESS_TOTAL_MWH_L,
    CMD_ADDRESS_TOTAL_MWH_H,
};

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

uint8_t __detect_load_modules(void);
void __update_register(uint8_t reg_address);
void __reload_watchdog(void);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// kernel task for parsing data from slave modules and handling communication faults
void load_task(void) {

    cmd_driver_init();

    // check all SPI slots while no slave modules are detected
    while (module_count == 0) module_count = __detect_load_modules();

    while (1) {

        bool checksum_ok = false;

        switch (com_state) {

            //---- WAIT FOR VALID ID -----------------------------------------------------------------------------------------------------------------------------

            case LOAD_COM_STATE_GETTING_ID:

                // continue if slave responded with valid ID and the checksum is correct
                if (cmd_read(0, CMD_ADDRESS_ID, &checksum_ok) == LOAD_ID_CODE && checksum_ok) com_state = LOAD_COM_STATE_WAITING_FOR_READY;
                break;

            //---- WAIT FOR READY FLAG ---------------------------------------------------------------------------------------------------------------------------

            case LOAD_COM_STATE_WAITING_FOR_READY:

                load_register[CMD_ADDRESS_STATUS] = cmd_read(0, CMD_ADDRESS_STATUS, &checksum_ok);      // read the status register

                if (checksum_ok && load_get_ready()) {
                    
                    load_set_fault_mask(LOAD_FAULT_ALL & ~(LOAD_FAULT_REG | LOAD_FAULT_EXTERNAL));      // set fault mask
                    
                    com_state = LOAD_COM_STATE_READY;                  // continue if the READY bit in the status register is set and checksum is correct
                }

                break;

            //---- PARSE DATA ------------------------------------------------------------------------------------------------------------------------------------

            case LOAD_COM_STATE_READY:

                // reload the watchdog in regular intervals while the load is enabled
                if (load_get_enable() && kernel_get_time_since(prev_watchdog_reload_time) >= LOAD_WD_TIMEOUT_MS / 2) __reload_watchdog();

                // parse next register in the queue
                if (data_capture_started) {

                    // read the register and save the data if checksum is correct
                    __update_register(read_queue[read_queue_pos++]);

                    // end of read queue, capture is done
                    if (read_queue_pos == sizeof(read_queue)) {
                        
                        read_queue_pos = 0;
                        data_capture_started = false;
                    }
                }

                break;

            //----------------------------------------------------------------------------------------------------------------------------------------------------
        }

        kernel_yield();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns number of populated SPI slots (only devices which responded with valid ID count)
uint8_t load_get_module_count(void) {return module_count;}

// starts parsing data from the slave module
void load_start_data_capture(void) {data_capture_started = true;}

// returns true if the data parsing is done
bool load_get_data_capture_done(void) {return !data_capture_started;}

// retuns true if the slave keeps responding with invalid checksum or doesn't respond at all
bool load_get_checksum_fault(void) {return checksum_fault;}

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// checks all SPI slots and returns number of populated slots which respond with valid ID (slots must be populated in order from slot 0)
uint8_t __detect_load_modules(void) {

    uint8_t module_count = 0;

    for (int module = 0; module < LOAD_CMD_SPI_SLOT_COUNT; module++) {

        // attempt connection 4 times for each SPI slot
        for (int attempt = 0; attempt < 4; attempt++) {

            bool checksum_ok = false;

            // slave returned ID and correct checksum. Load module found
            if (cmd_read(module, CMD_ADDRESS_ID, &checksum_ok) == LOAD_ID_CODE && checksum_ok) {

                module_count++;
                break;
            }

            // slots must be populated in order. If non-populated slot was found, skip the rest.
            if (attempt == 3) return module_count;

            kernel_yield();
        }
    }

    return module_count;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// reloads the load watchdog
void __reload_watchdog(void) {

    prev_watchdog_reload_time = kernel_get_time_ms();
    cmd_write(0, CMD_ADDRESS_WD_RELOAD, LOAD_WD_RELOAD_KEY);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
