#ifndef _CMD_SPI_DRIVER_H_
#define _CMD_SPI_DRIVER_H_

/*
 *  Load CMD SPI interface master driver
 *  Martin Kopka 2024
 */

#include "common_defs.h"
#include "cmd_spi_registers.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initializes the CMD interface master SPI driver
void cmd_driver_init(void);

// reads data from the specified load register via SPI
uint16_t cmd_read(uint8_t slot_number, uint8_t address, bool *checksum_ok_flag);

// writes data to the specified load register via SPI
void cmd_write(uint8_t slot_number, uint8_t address, uint16_t data);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _CMD_SPI_DRIVER_H_ */
