#ifndef _HW_CONFIG_H_
#define _HW_CONFIG_H_

//---- SYSTEM ----------------------------------------------------------------------------------------------------------------------------------------------------

#define F_XOSC_HZ       12000000    // crystal oscillator frequency [Hz]
#define F_CORE_HZ       100000000   // core clock frequency [Hz]

#define DEBUG_UART          UART1
#define DEBUG_UART_BAUD     115200
#define DEBUG_UART_TX_GPIO  4
#define DEBUG_UART_RX_GPIO  5

//---- KEY BACKLIGHT LED -----------------------------------------------------------------------------------------------------------------------------------------

#define KEY_BACKLIGHT_LED_GPIO          17
#define KEY_BACKLIGHT_LED_BRIGHTNESS    4095

//---- PUSH BUTTONS ----------------------------------------------------------------------------------------------------------------------------------------------

#define KEY_ENCODER_GPIO    3
#define KEY_MODE_GPIO       25
#define KEY_SET_GPIO        2
#define KEY_SEQ_GPIO        20
#define KEY_SEQ_EN_GPIO     18
#define KEY_MENU_GPIO       19
#define KEY_EN_GPIO         15
#define KEY_COUNT 7

//---- ROTARY ENCODER --------------------------------------------------------------------------------------------------------------------------------------------

#define ENCODER_A_GPIO 7
#define ENCODER_B_GPIO 6
#define ENCODER_DEBOUNCE_TIME_MS 50

//---- DISPLAY ---------------------------------------------------------------------------------------------------------------------------------------------------

#define DISPLAY_I2C                 I2C0
#define DISPLAY_SDA_GPIO            0
#define DISPLAY_SCL_GPIO            1
#define DISPLAY_I2C_ADDRESS         0x3C
#define DISPLAY_I2C_FREQUENCY_HZ    400000
#define DISPLAY_WIDTH               128
#define DISPLAY_HEIGHT              64

//---- LOAD CMD SPI ----------------------------------------------------------------------------------------------------------------------------------------------

#define LOAD_CMD_SPI                SPI1
#define LOAD_CMD_SPI_CS0_GPIO       13
#define LOAD_CMD_SPI_CS1_GPIO       9
#define LOAD_CMD_SPI_CS2_GPIO       14
#define LOAD_CMD_SPI_CS3_GPIO       8
#define LOAD_CMD_SPI_SCK_GPIO       10
#define LOAD_CMD_SPI_MOSI_GPIO      11
#define LOAD_CMD_SPI_MISO_GPIO      12
#define LOAD_CMD_SPI_FREQUENCY_HZ   128000

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _HW_CONFIG_H_ */