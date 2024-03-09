#include "common_defs.h"
#include "hal/xosc.h"
#include "hal/pll.h"
#include "hal/clocks.h"
#include "hal/fc0.h"

//---- CONSTANTS -------------------------------------------------------------------------------------------------------------------------------------------------

#define MAX_BYTES_PARSED    64      // maximum parsed bytes per update to avoid missed deadlines of other tasks or infinite loop in case of a fault

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void blink_task(void);
void uart_task(void);

int main() {

    // enable the crystal oscillator, initialize the PLL and set the system clock source to PLL without glitching the system clock
    xosc_enable();
    clocks_set_source(clk_sys, CLOCK_SRC_ROSC_CLKSRC_PH);
    pll_init(PLL_SYS, 100, 6, 2);
    clocks_set_aux_source(clk_sys, CLOCK_AUXSRC_CLKSRC_PLL_SYS);
    clocks_set_source(clk_sys, CLOCK_SRC_CLKSRC_CLK_REF_AUX);

    // set the XOSC as a reference clock and initialize the frequency counter
    clocks_set_source(clk_ref, CLOCK_SRC_XOSC_CLKSRC);
    fc0_init(F_XOSC_HZ);

    gpio_init();
    
    kernel_init(F_CORE_HZ);
    kernel_create_task(uart_task, 1000);
    kernel_create_task(blink_task, 1000);
    kernel_start();

	while (1) {

        NVIC_SystemReset();
    }
}

void blink_task(void) {

    gpio_set_dir(16, GPIO_DIR_OUTPUT);

    while (1) {

        gpio_toggle(16);
        kernel_sleep_ms(500);
    }
}

void uart_task(void) {

    char tx_fifo[512];              // transmit fifo to be provided to the UART driver
    char rx_fifo[128];              // receive fifo to be provided to the UART driver
    char message_buffer[128];       // buffer for parsing an incomming message until a newline is received
    uint32_t buffer_pos = 0;        // current index into the message_buffer

    uart_init(DEBUG_UART, DEBUG_UART_BAUD, DEBUG_UART_TX_GPIO, DEBUG_UART_RX_GPIO, (char*)&tx_fifo, sizeof(tx_fifo), (char*)&rx_fifo, sizeof(rx_fifo));
    uart_puts(DEBUG_UART, "Ready.\n");

    while (1) {

        uint32_t bytes_parsed = 0;          // limit parsed bytes per update to avoid missed deadlines of other tasks or infinite loop in case of a fault

        while (uart_has_data(DEBUG_UART) && bytes_parsed < MAX_BYTES_PARSED) {

            char c = uart_getc(DEBUG_UART);
            bytes_parsed++;

            // if newline reached or the buffer was filled, handle incomming message by sending it to the shell
            if (c == '\n' || buffer_pos == sizeof(message_buffer)) {

                message_buffer[buffer_pos] = '\0';       // append a null-terminator at the end
                buffer_pos = 0;

                uart_puts(DEBUG_UART, "> ");
                uart_puts(DEBUG_UART, message_buffer);
                uart_putc(DEBUG_UART, '\n');

            } else message_buffer[buffer_pos++] = c;     // else continue parsing
        }

        kernel_yield();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
