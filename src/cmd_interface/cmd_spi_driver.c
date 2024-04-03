#include "cmd_interface/cmd_spi_driver.h"
#include "hal/spi.h"

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

// calculates the checksum of a data frame
static inline uint8_t __calculate_checksum(uint8_t address, uint16_t data) {

    return (~((uint8_t)(address) ^ (uint8_t)(data & 0xff) ^ (uint8_t)((data >> 8) & 0xff)));
}

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initializes the CMD interface master SPI driver
void cmd_driver_init(void) {

    gpio_set_function(LOAD_CMD_SPI_CS_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LOAD_CMD_SPI_SCK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LOAD_CMD_SPI_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LOAD_CMD_SPI_MISO_GPIO, GPIO_FUNC_SPI);
    
    spi_init(LOAD_CMD_SPI, LOAD_CMD_SPI_FREQUENCY_HZ, 8);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

//dodelat
// reads one message from the write command buffer; callee should read the cmd_has_data() flag first
uint16_t cmd_read(uint8_t address, bool *checksum_ok_flag) {

    if (!cmd_address_valid(address)) return 0;

    while (spi_rx_not_empty(LOAD_CMD_SPI)) spi_read(LOAD_CMD_SPI);

    spi_write(LOAD_CMD_SPI, CMD_FRAME_SYNC_BYTE);               // send frame sync
    spi_write(LOAD_CMD_SPI, address | (CMD_READ_BIT >> 24));    // send address and read bit
    spi_write(LOAD_CMD_SPI, 0x00);                              // generate 8 SCK pulses to read in data high byte
    spi_write(LOAD_CMD_SPI, 0x00);                              // generate 8 SCK pulses to read in data low byte
    spi_write(LOAD_CMD_SPI, 0x00);                              // generate 8 SCK pulses to read in the checksum

    while (spi_busy(LOAD_CMD_SPI));

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

// writes data to the specified register; if the interface receives a read command on this address, this value will be transmitted to the master
void cmd_write(uint8_t address, uint16_t data) {

    if (!cmd_address_valid(address)) return;

    uint8_t checksum = __calculate_checksum(address, data);		// compute checksum

    spi_write(LOAD_CMD_SPI, CMD_FRAME_SYNC_BYTE);       // send frame sync
    spi_write(LOAD_CMD_SPI, address);                   // send address
    spi_write(LOAD_CMD_SPI, data >> 8);                 // send data high byte
    spi_write(LOAD_CMD_SPI, data & 0xff);               // send data low byte
    spi_write(LOAD_CMD_SPI, checksum);                  // send checksum

    while (spi_busy(LOAD_CMD_SPI));     // wait while sending; NOT NEEDED?
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
