#include "gui.h"

#include "display_driver/display.h"
#include "keypad.h"
#include "load.h"
#include "gui-bitmaps.h"

void gui_print_decimal(int value, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void gui_task(void) {

    uint32_t keypad_buttons_stack[32];
    kernel_create_task(keypad_buttons_task, keypad_buttons_stack, sizeof(keypad_buttons_stack), 10);

    uint8_t current_screen = 0;

    bool simulated_fault_triggered = false;

    bool bug_enabled = false;
    int bug_pos_x = 0;
    int bug_pos_y = 0;
    bool bug_dir_x = 1;
    bool bug_dir_y = 1;

    kernel_time_t load_toggled_since = 0;

    display_init();

    kernel_sleep_ms(500);

    while (!load_get_ready()) {

        display_draw_bitmap(bitmap_mk_logo_32x24, 8, 8);
        display_draw_string("400W DC Load", font_6x8, 48, 8, 0);
        display_draw_string("Martin Kopka 2024", font_6x8, 13, 32, 0);
        display_draw_bitmap(bitmap_progress_bar_32x8[(kernel_get_time_ms() >> 8) & 0x7], 48, 48);

        if (kernel_get_time_ms() > 5000) {

            display_flush_frame_buffer();
            display_draw_bitmap(bitmap_sad_emoji_32x32, 8, 8);
            display_draw_string("ERROR", font_6x8, 48, 8, 0);
            display_draw_string("Module not", font_6x8, 48, 24, 0);
            display_draw_string("responding", font_6x8, 48, 32, 0);

            uint8_t cursor = 28;
            display_draw_string("retrying ", font_6x8, cursor, 56, &cursor);

            kernel_time_t time = kernel_get_time_ms();
            uint8_t state = (time & (0x3 << 8)) >> 8;
            display_draw_string(".", font_6x8, cursor + (3 * state), 56, 0); 
        }

        display_render_frame();
        kernel_sleep_ms(17);
    }

    while (1) {

        while (!load_get_data_capture_done()) kernel_yield();

        bool load_enabled = load_get_enable();
        load_mode_t load_mode = load_get_mode();
        load_fault_t fault_register = load_get_faults();
        uint32_t load_voltage_mv = load_get_voltage_mv();
        uint32_t load_current_ma = load_get_total_current_ma();
        uint32_t load_power_mw = load_voltage_mv * load_current_ma / 1000;
        uint8_t load_temp = load_get_temp();
        uint16_t iset_ma = load_get_cc_level_ma();
        uint16_t total_mah = load_get_total_mah();
        uint16_t total_mwh = load_get_total_mwh();
        uint32_t seconds = load_get_ena_time();
        uint32_t minutes = 0;
        uint32_t hours = 0;

        while (seconds >= 60) {

            seconds -= 60;
            minutes++;
        }

        while (minutes >= 60) {

            minutes -= 60;
            hours++;
        }

        if (simulated_fault_triggered) {
            fault_register |= LOAD_FAULT_OTP;
            current_screen = 1;
        } else current_screen = 0;

        display_flush_frame_buffer();

        keypad_set_led(load_enabled);

        switch (current_screen) {

            case 0: {

                // load enable button
                if (keypad_is_pressed(KEY_EN, true)) load_set_enable(!load_enabled);
                if (keypad_is_pressed_for_ms(KEY_MENU, 1000, true)) bug_enabled = !bug_enabled;
                if (keypad_is_pressed(KEY_SEQ, true)) simulated_fault_triggered = true;

                // rotary encoder
                int16_t encoder_delta = keypad_get_encoder_pos();

                if (encoder_delta != 0) {

                    int16_t new_current = iset_ma + 100 * encoder_delta;
                    if (new_current < 000) iset_ma = 0;
                    if (new_current > 50000) iset_ma = 50000;

                    load_set_cc_level(new_current);
                }

                uint8_t cursor_pos = 0;

                if (hours > 0) {

                    display_draw_int(hours, font_6x8, cursor_pos, 0, &cursor_pos);
                    display_draw_char(':', font_6x8, cursor_pos, 0, &cursor_pos);
                }

                display_draw_int(minutes, font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_char(':', font_6x8, cursor_pos, 0, &cursor_pos);

                if (seconds < 10) display_draw_char('0', font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_int(seconds, font_6x8, cursor_pos, 0, &cursor_pos);

                cursor_pos = 0;
                display_draw_int(total_mah, font_6x8, cursor_pos, 8, &cursor_pos);
                display_draw_string("mAh", font_6x8, cursor_pos, 8, &cursor_pos);

                cursor_pos = 0;
                if (total_mwh < 1000) {

                    display_draw_int(total_mwh, font_6x8, cursor_pos, 16, &cursor_pos);
                    display_draw_string("mWh", font_6x8, cursor_pos, 16, &cursor_pos);
        
                } else if (total_mwh < 10000) {

                    gui_print_decimal(total_mwh, font_6x8, cursor_pos, 16, 2, &cursor_pos);
                    display_draw_string("Wh", font_6x8, cursor_pos, 16, &cursor_pos);

                } else if (total_mwh < 10000) {

                    gui_print_decimal(total_mwh, font_6x8, cursor_pos, 16, 1, &cursor_pos);
                    display_draw_string("Wh", font_6x8, cursor_pos, 16, &cursor_pos);
                } else {

                    display_draw_int(total_mwh / 1000, font_6x8, cursor_pos, 16, &cursor_pos);
                    display_draw_string("Wh", font_6x8, cursor_pos, 16, &cursor_pos);
                }

                if (load_get_not_in_reg()) display_draw_string("REG!", font_6x8, 48, 0, 0);

                cursor_pos = 104;
                if (load_temp < 10) display_draw_char(' ', font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_int(load_temp, font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_string("\'C", font_6x8, cursor_pos, 0, &cursor_pos);

                display_draw_bitmap(bitmap_temp_icon0_9x8, 92, 0);

                     if (load_mode == LOAD_MODE_CC) display_draw_string("CC", font_6x8, 0, 56, 0);
                else if (load_mode == LOAD_MODE_CV) display_draw_string("CV", font_6x8, 0, 56, 0);
                else if (load_mode == LOAD_MODE_CR) display_draw_string("CR", font_6x8, 0, 56, 0);
                else if (load_mode == LOAD_MODE_CP) display_draw_string("CP", font_6x8, 0, 56, 0);

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

            } break;

            case 1: {

                if (keypad_is_pressed_for_ms(KEY_SET, 500, true)) simulated_fault_triggered = false;

                //display_draw_bitmap(bitmap_sad_emoji_32x32, 8, 8);
                display_draw_bitmap(bitmap_fault_otp_32x48, 8, 8);
                display_draw_string("ERROR", font_6x8, 48, 8, 0);
                display_draw_string("OTP Fault", font_6x8, 48, 24, 0);
                display_draw_string("triggered", font_6x8, 48, 32, 0);

                uint8_t cursor_pos = 16;
                //if (fault_register & LOAD_FAULT_COM) display_draw_string("COM ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_REG) display_draw_string("REG ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_OTP) display_draw_string("OTP ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_TEMP_L) display_draw_string("TMPL ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_TEMP_R) display_draw_string("TMPR ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FAN1) display_draw_string("FAN1 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FAN2) display_draw_string("FAN2 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_OCP) display_draw_string("OCP ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_OPP) display_draw_string("OPP ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FUSE_L1) display_draw_string("FL1 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FUSE_L2) display_draw_string("FL2 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FUSE_R1) display_draw_string("FR1 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_FUSE_R2) display_draw_string("FR2 ", font_6x8, cursor_pos, 32, &cursor_pos);
                //if (fault_register & LOAD_FAULT_EXTERNAL) display_draw_string("EXT ", font_6x8, cursor_pos, 32, &cursor_pos);

                display_draw_string("clear ", font_6x8, 48, 56, 0);

                if (keypad_is_pressed(KEY_SET, false)) {

                    display_draw_bitmap(bitmap_progress_bar_32x8[(keypad_get_hold_time(KEY_SET) >> 6) & 0x7], 48, 56);
                }

            } break;

            default:
                break;
        }

        if (bug_enabled) {

            display_draw_bitmap_not_aligned((kernel_get_time_ms() & (1 << 8)) ? bitmap_bug_16x18 : bitmap_bug1_16x18, bug_pos_x, bug_pos_y, true);

            bug_pos_x += (bug_dir_x) ? 1 : -1;
            bug_pos_y += (bug_dir_y) ? 1 : -1;

            if (bug_pos_x == 0) bug_dir_x = 1;
            if (bug_pos_x >= DISPLAY_WIDTH - 16) bug_dir_x = 0;
            if (bug_pos_y == 0) bug_dir_y = 1;
            if (bug_pos_y >= DISPLAY_HEIGHT - 18) bug_dir_y = 0;
        }

        display_render_frame();

        load_start_data_capture();

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
