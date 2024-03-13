#include "load.h"
#include "cmd_interface/cmd_spi_driver.h"

typedef enum {

    LOAD_COM_STATE_GETTING_ID,
    LOAD_COM_STATE_WAITING_FOR_READY,
    LOAD_COM_STATE_READY

} load_communication_state_t;

load_communication_state_t com_state;

bool load_ready = false;
uint16_t status_register = 0;
uint16_t cc_level = 0;
uint16_t input_voltage = 0;
uint16_t load_current = 0;
uint8_t load_temp[2] = {0};

void load_task(void) {

    cmd_driver_init();

    while (1) {

        switch (com_state) {

            case LOAD_COM_STATE_GETTING_ID:

                if (cmd_read(CMD_ADDRESS_ID) == LOAD_ID_CODE) com_state = LOAD_COM_STATE_WAITING_FOR_READY;
                break;

            case LOAD_COM_STATE_WAITING_FOR_READY:

                if (cmd_read(CMD_ADDRESS_ID) != LOAD_ID_CODE) com_state = LOAD_COM_STATE_GETTING_ID;

                load_ready = true;
                
                //if (cmd_read(CMD_ADDRESS_STATUS))
                com_state = LOAD_COM_STATE_READY;
                break;

            case LOAD_COM_STATE_READY:

                if (cmd_read(CMD_ADDRESS_ID) != LOAD_ID_CODE) com_state = LOAD_COM_STATE_GETTING_ID;

                status_register = cmd_read(CMD_ADDRESS_STATUS);
                cc_level = cmd_read(CMD_ADDRESS_CC_LEVEL);
                input_voltage = cmd_read(CMD_ADDRESS_VIN);
                load_current = cmd_read(CMD_ADDRESS_ITOT);
                load_temp[0] = cmd_read(CMD_ADDRESS_TEMP_L);
                load_temp[1] = cmd_read(CMD_ADDRESS_TEMP_R);
                break;
        }


        kernel_sleep_ms(10);
        //kernel_yield();
    }
}

bool load_get_ready(void) {

    return load_ready;
}

void load_set_enable(bool enabled) {

    if (enabled) cmd_write(CMD_ADDRESS_ENABLE, LOAD_ENABLE_KEY);
    else cmd_write(CMD_ADDRESS_ENABLE, 0x0000);
}

bool load_get_enable(void) {

    return (!!(status_register & 0x0001));
}

uint32_t load_get_cc_level_ma(void) {

    return cc_level;
}

uint32_t load_get_voltage_mv(void) {

    return input_voltage;
}

uint32_t load_get_total_current_ma(void) {

    return load_current;
}

void load_set_cc_level(uint16_t current_ma) {

    cmd_write(CMD_ADDRESS_CC_LEVEL, current_ma);
}

uint8_t load_get_temp(void) {

    return (load_temp[0]);
}