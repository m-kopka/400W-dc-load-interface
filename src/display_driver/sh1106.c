#include "display_driver/sh1106.h"
#include "hal/i2c.h"

//---- COMMAND TABLE ---------------------------------------------------------------------------------------------------------------------------------------------

#define SH1106_CMD_SET_COLUMN_ADDRESS_LOW           0x00    // Sets 4 lower bits of column address of display RAM in register.
#define SH1106_CMD_SET_COLUMN_ADDRESS_HIGH          0x10    // Sets 4 higher bits of column address of display RAM in register.
#define SH1106_CMD_SET_PUMP_VOLTAGE_VALUE           0x30    // This command is to control the DC-DC voltage output value.
#define SH1106_CMD_SET_DISPLAY_START_LINE           0x40    // Specifies RAM display line for COMO.
#define SH1106_CMD_CONTRAST_CONTROL_MODE_SET        0x81    // This command is to set Contrast Setting of the display. The chip has 256 contrast steps from 00 to FF.
#define SH1106_CMD_SET_SEGMENT_REMAP                0xA0    // The right (0) or left (1) rotation.
#define SH1106_CMD_SET_ENTIRE_DISPLAY_OFF           0xA4    // Selects normal display
#define SH1106_CMD_SET_ENTIRE_DISPLAY_ON            0xA5    // Selects entire display ON
#define SH1106_CMD_SET_NORMAL_DISPLAY               0xA6    // Normal indication
#define SH1106_CMD_SET_REVERSE_DISPLAY              0xA7    // Reverse indication
#define SH1106_CMD_MULTIPLEX_RATION_MODE_SET        0xA8    // This command switches default 63 multiplex mode to any multiplex ration from 1 to 64.
#define SH1106_CMD_DC_DC_CONTROL_MODE_SET           0xAD    // This command is to control the DC-DC voltage DC-DC will be turned on when display on converter (1) or DC-DC off (0).
#define SH1106_CMD_DISPLAY_OFF                      0xAE    // Turns OFF the OLED panel.
#define SH1106_CMD_DISPLAY_ON                       0xAF    // Turns ON the OLED panel.
#define SH1106_CMD_SET_PAGE_ADDRESS                 0xB0    // Specifies page address to load display RAM data to page address register.
#define SH1106_CMD_SET_COMMON_OUTPUT_SCAN_DIR       0xC0    // Scan from COMO to COM[N - 1] (0) or Scan from COM[N - 1] to COMO (1).
#define SH1106_CMD_DISPLAY_OFFSET_MODE_SET          0xD3    // This is a double byte command which specifies the mapping of display start line to one of COMO-63.
#define SH1106_CMD_SET_DISPLAY_DIVIDE_RATIO         0xD5    // This command is used to set the frequency of the internal display clocks.
#define SH1106_CMD_DISCHARGE_PERIOD_MODE_SET        0xD9    // This command is used to set the duration of the dis-charge and pre-charge period.
#define SH1106_CMD_COMMON_PADS_HW_CONFIG_MODE_SET   0xDA    // This command is to set the common signals pad configuration.
#define SH1106_CMD_VCOM_DESELECT_LEVEL_MODE_SET     0xDB    // This command is to set the common pad output voltage level at deselect stage.

//---- INTERNAL FUNCTION PROTOTYPES ------------------------------------------------------------------------------------------------------------------------------

void __sh1106_command_8(uint8_t command);
void __sh1106_command_16(uint8_t command, uint8_t setting);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initialize I2C communication and send the display inizialization sequence
void sh1106_init(void) {

    i2c_init(DISPLAY_I2C, DISPLAY_I2C_FREQUENCY_KHZ, DISPLAY_SDA_GPIO, DISPLAY_SCL_GPIO);

    __sh1106_command_8(SH1106_CMD_DISPLAY_OFF);
    __sh1106_command_16(SH1106_CMD_SET_DISPLAY_DIVIDE_RATIO, 0x80);
    __sh1106_command_16(SH1106_CMD_MULTIPLEX_RATION_MODE_SET, 0x3F);
    __sh1106_command_16(SH1106_CMD_DISPLAY_OFFSET_MODE_SET, 0x00);
    __sh1106_command_8(SH1106_CMD_SET_SEGMENT_REMAP | 1);
    __sh1106_command_8(SH1106_CMD_SET_PUMP_VOLTAGE_VALUE | 8);
    __sh1106_command_8(SH1106_CMD_SET_COMMON_OUTPUT_SCAN_DIR | (1 << 3));
    __sh1106_command_16(SH1106_CMD_COMMON_PADS_HW_CONFIG_MODE_SET, 0x12);
    __sh1106_command_16(SH1106_CMD_CONTRAST_CONTROL_MODE_SET, 0xFF);
    __sh1106_command_16(SH1106_CMD_DISCHARGE_PERIOD_MODE_SET, 0xF1);
    __sh1106_command_16(SH1106_CMD_VCOM_DESELECT_LEVEL_MODE_SET, 0x40);
    __sh1106_command_8(SH1106_CMD_SET_ENTIRE_DISPLAY_OFF);
    __sh1106_command_8(SH1106_CMD_SET_NORMAL_DISPLAY);

    sh1106_clear_screen();
    __sh1106_command_8(SH1106_CMD_DISPLAY_ON);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// clear display RAM
void sh1106_clear_screen(void) {

    uint8_t temp_buff[DISPLAY_WIDTH] = {0};
    for (int page = 0; page < 8; page ++) sh1106_send_pixel_data(temp_buff, DISPLAY_WIDTH, page, 0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// send pixel data to display RAM
void sh1106_send_pixel_data(uint8_t *data, uint16_t size, uint8_t page, uint8_t column_adress) {

    __sh1106_command_8(SH1106_CMD_SET_PAGE_ADDRESS | page);                                   // set page address
    __sh1106_command_8(SH1106_CMD_SET_COLUMN_ADDRESS_LOW  | ((2 + column_adress) & 0xf));     // set column address low 4 bits
    __sh1106_command_8(SH1106_CMD_SET_COLUMN_ADDRESS_HIGH | ((2 + column_adress) >> 4));      // set column address high 4 bits
    
    i2c_start_transmission(DISPLAY_I2C, DISPLAY_I2C_ADDRESS);
    i2c_write(DISPLAY_I2C, SH1106_CMD_SET_DISPLAY_START_LINE | 0x0, false);

    for (uint16_t i = 0; i < size; i++) i2c_write(DISPLAY_I2C, data[i], i == (size - 1));
}

//---- PRIVATE FUNCTIONS -----------------------------------------------------------------------------------------------------------------------------------------

// send a single byte display command via I2C
void __sh1106_command_8(uint8_t command) {

    i2c_start_transmission(DISPLAY_I2C, DISPLAY_I2C_ADDRESS);
    i2c_write(DISPLAY_I2C, 0x00, false);
    i2c_write(DISPLAY_I2C, command, true);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// send a double byte display command via I2C
void __sh1106_command_16(uint8_t command, uint8_t setting) {

    __sh1106_command_8(command);
    __sh1106_command_8(setting);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
