#include "gui.h"

#include "display_driver/display.h"
//#include "keypad.h"
#include "gui-bitmaps.h"


//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void gui_task(void) {

    int bug_pos_x = 0;
    int bug_pos_y = 0;
    bool bug_dir_x = 1;
    bool bug_dir_y = 1;

    display_init();

    display_draw_bitmap(bitmap_onsemi_logo_92x16, 18, 16);
    display_draw_string("DC Electronic Load", font_6x8, 10, 40, 0);
    display_draw_string("Martin Kopka 2024", font_6x8, 10, 48, 0);
    display_render_frame();
    kernel_sleep_ms(2000);

    while (1) {

        display_flush_frame_buffer();

        display_draw_bitmap(bitmap_temp_icon0_9x8, 92, 0);
        display_draw_string("CC", font_6x8, 0, 56, 0);
        display_draw_bitmap(bitmap_off_button_32x16, 96, 48);
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
