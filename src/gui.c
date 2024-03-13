#include "gui.h"

#include "display_driver/display.h"
#include "keypad.h"
#include "load.h"
#include "gui-bitmaps.h"

void gui_print_decimal(int value, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void gui_task(void) {

    kernel_create_task(keypad_buttons_task, 10);

    int bug_pos_x = 0;
    int bug_pos_y = 0;
    bool bug_dir_x = 1;
    bool bug_dir_y = 1;

    kernel_time_t load_toggled_since = 0;

    display_init();

    if (!load_get_ready()) {

        display_draw_bitmap(bitmap_onsemi_logo_92x16, 18, 16);
        display_draw_string("DC Electronic Load", font_6x8, 10, 40, 0);
        display_draw_string("Martin Kopka 2024", font_6x8, 10, 48, 0);
        display_render_frame();
    }

    while (!load_get_ready()) kernel_yield();

    while (1) {

        bool load_enabled = load_get_enable();
        uint32_t load_voltage_mv = load_get_voltage_mv();
        uint32_t load_current_ma = load_get_total_current_ma();
        uint32_t load_power_mw = load_voltage_mv * load_current_ma / 1000;
        uint8_t load_temp = load_get_temp();

        uint16_t iset_ma = load_get_cc_level_ma();

        keypad_set_led(load_enabled);
        
        // load enable button
        if (keypad_is_pressed(6, true)) {

            load_set_enable(!load_enabled);
        }

        // rotary encoder
        int16_t encoder_delta = keypad_get_encoder_pos();

        if (encoder_delta != 0) {

            iset_ma += 100 * encoder_delta;
            if (iset_ma < 100) iset_ma = 100;
            if (iset_ma > 40000) iset_ma = 40000;

            load_set_cc_level(iset_ma);
        }

        display_flush_frame_buffer();

        display_draw_string("00:00:00\n", font_6x8, 0, 0, 0);
        display_draw_string("0000mAh\n", font_6x8, 0, 8, 0);

        uint8_t cursor_pos = 104;
        if (load_temp < 10) display_draw_char(' ', font_6x8, cursor_pos, 0, &cursor_pos);
        display_draw_int(load_temp, font_6x8, cursor_pos, 0, &cursor_pos);
        display_draw_string("\'C", font_6x8, cursor_pos, 0, &cursor_pos);

        display_draw_bitmap(bitmap_temp_icon0_9x8, 92, 0);
        display_draw_string("CC", font_6x8, 0, 56, 0);

        cursor_pos = 0;
        gui_print_decimal(iset_ma, font_6x8, cursor_pos, 48, 2, &cursor_pos);
        display_draw_char('A', font_6x8, cursor_pos, 48, &cursor_pos);

        cursor_pos = 32;
        if (load_current_ma < 10000) cursor_pos += 16;
        gui_print_decimal(load_current_ma, font_16x24, cursor_pos, 16, 2, &cursor_pos);
        display_draw_string(" A", font_6x8, cursor_pos, 32, &cursor_pos);

        cursor_pos = 48;
        gui_print_decimal(load_voltage_mv, font_6x8, cursor_pos, 48, 2, &cursor_pos);
        display_draw_char('V', font_6x8, cursor_pos, 48, &cursor_pos);;

        cursor_pos = 48;
        gui_print_decimal(load_power_mw, font_6x8, cursor_pos, 56, 1, &cursor_pos);
        display_draw_char('W', font_6x8, cursor_pos, 56, &cursor_pos);


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

void gui_print_decimal(int value, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor) {

    char string[16];        // stores the string generated from the input integer
    int buff_index = 0;     // write head to the string array
    int remainder = value;
    int i = 0;

    while (remainder > 0 || i < 3) {    // iterate while there are digits left or the decimal part isn't complete

        int digit = remainder % 10;     // get last digit
        remainder /= 10;

        // convert the current digit to a character and append it to the buffer; skip if the digit is ignored (dec_places setting)
        if (++i > 3 - dec_places) string[buff_index++] = digit + '0';

        // append a decimal point after the decimal part is finished
        if (i == 3) string[buff_index++] = '.';
    }        

    if (value < 1000) string[buff_index++] = '0';        // if the value doesn't have an integer part, append a zero

    // print the buffer on screen (the characters are in reverse order)
    while (buff_index > 0) {

        display_draw_char(string[--buff_index], font, *cursor, y_pos, cursor);

        if (string[buff_index] == '.') {
            
            // the dot is too narrow and we don't support non-monospace font rendering; this artificially decreases the space after a dot is printed
            if (font == font_6x8) *cursor -= 2;
            else if (font == font_16x24) *cursor -= 8;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
