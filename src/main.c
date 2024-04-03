#include "common_defs.h"
#include "hal/xosc.h"
#include "hal/pll.h"
#include "hal/clocks.h"
#include "hal/fc0.h"
#include "hal/watchdog.h"
#include "load.h"
#include "gui.h"

int main() {

    // enable the crystal oscillator, initialize the PLL and set the system clock source to PLL without glitching the system clock
    xosc_enable();
    clocks_set_source(clk_sys, CLOCK_SRC_SYS_CLK_REF);
    pll_init(PLL_SYS, 100, 6, 2);
    clocks_set_aux_source(clk_sys, CLOCK_AUXSRC_SYS_CLKSRC_PLL_SYS);
    clocks_set_source(clk_sys, CLOCK_SRC_SYS_CLKSRC_CLK_SYS_AUX);

    // set the XOSC as a reference clock and initialize the frequency counter
    clocks_set_source(clk_ref, CLOCK_SRC_REF_XOSC_CLKSRC);
    fc0_init(F_XOSC_HZ);

    // initialize the watchdog timer to generate a 1us tick for the timer
    watchdog_start_tick(F_XOSC_HZ);

    // configure the peripheral clock
    clocks_set_aux_source(clk_peri, CLOCK_AUXSRC_PERI_CLK_SYS);
    clocks_set_enable(clk_peri, true);

    gpio_init();

    uint32_t debug_uart_stack[512];
    uint32_t load_stack[256];
    uint32_t gui_stack[256];
    
    kernel_init(F_CORE_HZ);
    kernel_create_task(debug_uart_task, debug_uart_stack, sizeof(debug_uart_stack), 2000);
    kernel_create_task(load_task, load_stack, sizeof(load_stack), 100);
    kernel_create_task(gui_task, gui_stack, sizeof(gui_stack), 100);
    kernel_start();

	while (1) {

        NVIC_SystemReset();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
