#include "gui.h"

#include "display_driver/display.h"
#include "keypad.h"
#include "gui-bitmaps.h"

#include "cmd_interface/cmd_spi_driver.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void gui_task(void) {

    kernel_create_task(keypad_buttons_task, 100);

    int bug_pos_x = 0;
    int bug_pos_y = 0;
    bool bug_dir_x = 1;
    bool bug_dir_y = 1;

    kernel_time_t load_toggled_since = 0;

    cmd_driver_init();

    display_init();

    display_draw_bitmap(bitmap_onsemi_logo_92x16, 18, 16);
    display_draw_string("DC Electronic Load", font_6x8, 10, 40, 0);
    display_draw_string("Martin Kopka 2024", font_6x8, 10, 48, 0);
    display_render_frame();
    kernel_sleep_ms(2000);

    while (1) {

        cmd_read(CMD_ADDRESS_ID);

        uint16_t status_register = cmd_read(CMD_ADDRESS_STATUS);
        bool load_enabled = status_register & 0x0001;

        keypad_set_led(load_enabled);
        
        // load enable button
        if (keypad_is_pressed(6, true)) {
            
            if (!load_enabled) cmd_write(CMD_ADDRESS_ENABLE, 0xABCD);
            else cmd_write(CMD_ADDRESS_ENABLE, 0x0000);
        }

        display_flush_frame_buffer();

        display_draw_bitmap(bitmap_temp_icon0_9x8, 92, 0);
        display_draw_string("CC", font_6x8, 0, 56, 0);


        if (kernel_get_time_since(load_toggled_since) < 50) display_draw_bitmap(bitmap_off_button_anim_32x16, 96, 48);
        else display_draw_bitmap(load_enabled ? bitmap_on_button_32x16 : bitmap_off_button_32x16, 96, 48);



        display_draw_bitmap_not_aligned(bitmap_bug_16x18, bug_pos_x, bug_pos_y, true);

        if (bug_dir_x) bug_pos_x += 1;
        else bug_pos_x -= 1;
        if (bug_dir_y) bug_pos_y += 1;
        else bug_pos_y -= 1;
        if (bug_pos_x == 0) bug_dir_x = 1;
        if (bug_pos_x >= 112) bug_dir_x = 0;
        if (bug_pos_y == 0) bug_dir_y = 1;
        if (bug_pos_y >= 46) bug_dir_y = 0;
        

        display_render_frame();

        //------------------------------------------------------------------------------------------------------------------------------------------------------------

        kernel_sleep_ms(17);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
