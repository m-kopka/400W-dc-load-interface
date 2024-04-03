#ifndef _CMD_SPI_DRIVER_H_
#define _CMD_SPI_DRIVER_H_

/*
 *  Load CMD SPI interface master driver
 *  Martin Kopka 2024
 */

#include "common_defs.h"
#include "cmd_interface/cmd_spi_registers.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initializes the CMD interface master SPI driver
void cmd_driver_init(void);

// reads one message from the write command buffer; callee should read the cmd_has_data() flag first
uint16_t cmd_read(uint8_t address, bool *checksum_ok_flag);

// writes data to the specified register; if the interface receives a read command on this address, this value will be transmitted to the master
void cmd_write(uint8_t address, uint16_t data);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _CMD_SPI_DRIVER_H_ */
