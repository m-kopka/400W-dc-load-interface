#include "cmd_spi_driver.h"
#include "hal/spi.h"

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

static const uint8_t cmd_cs_gpio[] = {LOAD_CMD_SPI_CS0_GPIO, LOAD_CMD_SPI_CS1_GPIO, LOAD_CMD_SPI_CS2_GPIO, LOAD_CMD_SPI_CS3_GPIO};

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// calculates the data frame checksum
static inline uint8_t __calculate_checksum(uint8_t address, uint16_t data) {

    return (~((uint8_t)(address) ^ (uint8_t)(data & 0xff) ^ (uint8_t)((data >> 8) & 0xff)));
}

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initializes the CMD interface master SPI driver
void cmd_driver_init(void) {

    for (int i = 0; i < 4; i++) {

        gpio_write(cmd_cs_gpio[i], HIGH);
        gpio_set_dir(cmd_cs_gpio[i], GPIO_DIR_OUTPUT);
    }
    
    gpio_set_function(LOAD_CMD_SPI_SCK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LOAD_CMD_SPI_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LOAD_CMD_SPI_MISO_GPIO, GPIO_FUNC_SPI);
    
    spi_init(LOAD_CMD_SPI, LOAD_CMD_SPI_FREQUENCY_HZ, 8);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// reads data from the specified load register via SPI
uint16_t cmd_read(uint8_t slot_number, uint8_t address, bool *checksum_ok_flag) {

    if (slot_number >= LOAD_CMD_SPI_SLOT_COUNT) return 0;
    if (!cmd_address_valid(address)) return 0;

    while (spi_rx_not_empty(LOAD_CMD_SPI)) spi_read(LOAD_CMD_SPI);

    spi_write_blocking_software_ss(LOAD_CMD_SPI, CMD_FRAME_SYNC_BYTE, cmd_cs_gpio[slot_number]);               // send frame sync
    spi_write_blocking_software_ss(LOAD_CMD_SPI, address | (CMD_READ_BIT >> 24), cmd_cs_gpio[slot_number]);    // send address and read bit
    spi_write_blocking_software_ss(LOAD_CMD_SPI, 0x00, cmd_cs_gpio[slot_number]);                              // generate 8 SCK pulses to read in data high byte
    spi_write_blocking_software_ss(LOAD_CMD_SPI, 0x00, cmd_cs_gpio[slot_number]);                              // generate 8 SCK pulses to read in data low byte
    spi_write_blocking_software_ss(LOAD_CMD_SPI, 0x00, cmd_cs_gpio[slot_number]);                              // generate 8 SCK pulses to read in the checksum

    // discard the received bytes from the frame sync and address transmission
    spi_read(LOAD_CMD_SPI);
    spi_read(LOAD_CMD_SPI);
    
    uint8_t data_high = spi_read(LOAD_CMD_SPI);
    uint8_t data_low = spi_read(LOAD_CMD_SPI);
    uint8_t checksum = spi_read(LOAD_CMD_SPI);

    uint8_t correct_checksum = __calculate_checksum(address | (CMD_READ_BIT >> 24), (data_high << 8) | data_low);
    *checksum_ok_flag = (checksum == correct_checksum);

    return ((data_high << 8) | data_low);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// writes data to the specified load register via SPI
void cmd_write(uint8_t slot_number, uint8_t address, uint16_t data) {

    if (slot_number >= LOAD_CMD_SPI_SLOT_COUNT) return;
    if (!cmd_address_valid(address)) return;

    uint8_t checksum = __calculate_checksum(address, data);		// compute checksum

    spi_write_blocking_software_ss(LOAD_CMD_SPI, CMD_FRAME_SYNC_BYTE, cmd_cs_gpio[slot_number]);  // send frame sync
    spi_write_blocking_software_ss(LOAD_CMD_SPI, address, cmd_cs_gpio[slot_number]);              // send address
    spi_write_blocking_software_ss(LOAD_CMD_SPI, data >> 8, cmd_cs_gpio[slot_number]);            // send data high byte
    spi_write_blocking_software_ss(LOAD_CMD_SPI, data & 0xff, cmd_cs_gpio[slot_number]);          // send data low byte
    spi_write_blocking_software_ss(LOAD_CMD_SPI, checksum, cmd_cs_gpio[slot_number]);             // send checksum
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
