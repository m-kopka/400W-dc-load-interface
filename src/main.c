#include "common_defs.h"
#include "hal/xosc.h"
#include "hal/pll.h"
#include "hal/clocks.h"
#include "hal/fc0.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

volatile uint32_t time = 0;

int main() {

    uint32_t prev_blink = 0;

    // enable the crystal oscillator, initialize the PLL and set the system clock source to PLL without glitching the system clock
    xosc_enable();
    clocks_set_source(clk_sys, CLOCK_SRC_ROSC_CLKSRC_PH);
    pll_init(PLL_SYS, 100, 6, 2);
    clocks_set_aux_source(clk_sys, CLOCK_AUXSRC_CLKSRC_PLL_SYS);
    clocks_set_source(clk_sys, CLOCK_SRC_CLKSRC_CLK_REF_AUX);

    // set the XOSC as a reference clock and initialize the frequency counter
    clocks_set_source(clk_ref, CLOCK_SRC_XOSC_CLKSRC);
    fc0_init(F_XOSC_HZ);

    SysTick_Config(F_CORE_HZ / 1000);

    gpio_init();
    gpio_set_dir(16, GPIO_DIR_OUTPUT);

	while (1) {

        if (time - prev_blink >= 500) {

            gpio_toggle(16);
            prev_blink = time;
        }
	}
}

//---- IRQ HANDLERS ----------------------------------------------------------------------------------------------------------------------------------------------

void SysTick_Handler() {

    time++;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
