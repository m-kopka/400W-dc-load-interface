#include "common_defs.h"
#include "hal/xosc.h"
#include "hal/pll.h"
#include "hal/clocks.h"
#include "hal/fc0.h"
#include "hal/watchdog.h"
#include "gui.h"

void blink_task(void);

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

    watchdog_start_tick(F_XOSC_HZ);

    gpio_init();
    
    kernel_init(F_CORE_HZ);
    kernel_create_task(debug_uart_task, 2000);
    kernel_create_task(gui_task, 100);
    kernel_start();

	while (1) {

        NVIC_SystemReset();
    }
}

void blink_task(void) {

    gpio_write(16, HIGH);
    gpio_set_dir(16, GPIO_DIR_OUTPUT);

    while (1) {

        gpio_toggle(16);
        kernel_sleep_ms(500);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
