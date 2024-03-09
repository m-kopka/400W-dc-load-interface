#ifndef _SH1106_H_
#define _SH1106_H_

/*
 *  SH1106 Dot Matrix OLED Controller I2C Driver
 *  Martin Kopka 2024
 */

#include "common_defs.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initialize I2C communication and send the display inizialization sequence
void sh1106_init(void);

// clear display RAM
void sh1106_clear_screen(void);

// send pixel data to display RAM
void sh1106_send_pixel_data(uint8_t *data, uint16_t size, uint8_t page, uint8_t column_adress);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _SH1106_H_ */