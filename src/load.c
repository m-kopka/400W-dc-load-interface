#include "load.h"
#include "cmd_interface/cmd_spi_driver.h"

typedef enum {

    LOAD_COM_STATE_GETTING_ID,
    LOAD_COM_STATE_WAITING_FOR_READY,
    LOAD_COM_STATE_READY

} load_communication_state_t;

load_communication_state_t com_state;

uint16_t status_register = 0;
uint16_t fault_register = 0;
uint16_t config_register = 0;
uint16_t cc_level = 0;
uint16_t input_voltage = 0;
uint16_t load_current = 0;
uint8_t load_temp[2] = {0};

uint16_t ena_time = 0;
uint16_t total_mah = 0;
uint16_t total_mwh = 0;

void load_task(void) {

    cmd_driver_init();

    while (1) {

        bool checksum_ok = false;

        switch (com_state) {

            case LOAD_COM_STATE_GETTING_ID:

                if (cmd_read(CMD_ADDRESS_ID, &checksum_ok) == LOAD_ID_CODE) com_state = LOAD_COM_STATE_WAITING_FOR_READY;
                break;

            case LOAD_COM_STATE_WAITING_FOR_READY:

                if (cmd_read(CMD_ADDRESS_ID, &checksum_ok) != LOAD_ID_CODE) com_state = LOAD_COM_STATE_GETTING_ID;
                
                //if (cmd_read(CMD_ADDRESS_STATUS))
                com_state = LOAD_COM_STATE_READY;
                break;

            case LOAD_COM_STATE_READY:

                if (cmd_read(CMD_ADDRESS_ID, &checksum_ok) != LOAD_ID_CODE) com_state = LOAD_COM_STATE_GETTING_ID;

                status_register = cmd_read(CMD_ADDRESS_STATUS, &checksum_ok);
                fault_register = cmd_read(CMD_ADDRESS_FAULT, &checksum_ok);
                config_register = cmd_read(CMD_ADDRESS_CONFIG, &checksum_ok);
                cmd_write(CMD_ADDRESS_WD_RELOAD, 0xBABA);
                cc_level = cmd_read(CMD_ADDRESS_CC_LEVEL, &checksum_ok);
                input_voltage = cmd_read(CMD_ADDRESS_VOLTAGE, &checksum_ok) * 10;
                load_current = cmd_read(CMD_ADDRESS_CURRENT, &checksum_ok);
                load_temp[0] = cmd_read(CMD_ADDRESS_TEMP_L, &checksum_ok);
                load_temp[1] = cmd_read(CMD_ADDRESS_TEMP_R, &checksum_ok);
                ena_time = cmd_read(CMD_ADDRESS_TOTAL_TIME_L, &checksum_ok);
                total_mah = cmd_read(CMD_ADDRESS_TOTAL_MAH_L, &checksum_ok);
                total_mwh = cmd_read(CMD_ADDRESS_TOTAL_MWH_L, &checksum_ok);
                break;
        }

        kernel_yield();
    }
}

bool load_get_ready(void) {

    return (!!(status_register & LOAD_STATUS_READY));
}

void load_set_enable(bool enabled) {

    if (enabled) cmd_write(CMD_ADDRESS_ENABLE, LOAD_ENABLE_KEY);
    else cmd_write(CMD_ADDRESS_ENABLE, 0x0000);
}

bool load_get_enable(void) {

    return (!!(status_register & 0x0001));
}

bool load_get_vsensrc(void) {

    return (!!(config_register & LOAD_CONFIG_VSEN_SRC));
}

bool load_get_not_in_reg(void) {

    return (!!(status_register & LOAD_STATUS_NO_REG));
}

uint16_t load_get_ena_time(void) {

    return ena_time;
}

uint16_t load_get_total_mah(void) {

    return total_mah;
}

uint16_t load_get_total_mwh(void) {

    return total_mwh;
}

load_fault_t load_get_faults(void) {

    return fault_register;
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