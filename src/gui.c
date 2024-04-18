#include "gui.h"
#include "display_driver/display.h"
#include "keypad.h"
#include "load.h"
#include "gui-bitmaps.h"

//---- ENUMERATIONS ----------------------------------------------------------------------------------------------------------------------------------------------

// gui screens
typedef enum {

    SCREEN_MAIN,
    SCREEN_COM_FAULT,
    SCREEN_FAULT,

} screen_t;

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

void __print_error_window(const uint8_t *icon, char *header, char *line1, char *line2, char *line3);
void __print_decimal_with_unit(int value, char *unit, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor);
void __print_decimal(int value, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

void gui_task(void) {

    uint32_t keypad_buttons_stack[32];
    kernel_create_task(keypad_buttons_task, keypad_buttons_stack, sizeof(keypad_buttons_stack), 10);

    screen_t current_screen = SCREEN_MAIN;

    bool bug_enabled = false;
    int bug_pos_x = 0;
    int bug_pos_y = 0;
    bool bug_dir_x = 1;
    bool bug_dir_y = 1;

    display_init();

    kernel_sleep_ms(100);

    //---- BOOT SCREEN AND COM FAULT MESSAGE ---------------------------------------------------------------------------------------------------------------------

    while (!load_get_ready()) {

        display_flush_frame_buffer();

        display_draw_bitmap(bitmap_mk_logo_32x24, 8, 8);
        display_draw_string("400W DC Load", font_6x8, 48, 8, 0);
        display_draw_string("Martin Kopka 2024", font_6x8, 13, 32, 0);

        if (load_get_module_count() > 0) {
            
            display_draw_bitmap(bitmap_progress_bar_32x8[(kernel_get_time_ms() >> 8) & 0x7], 48, 48);

            if (kernel_get_time_ms() > 5000) __print_error_window(bitmap_sad_emoji_32x32, "ERROR", "Load stuck", "in selftest", "");

        } else if (kernel_get_time_ms() > 3000) {

            __print_error_window(bitmap_sad_emoji_32x32, "ERROR", "No load", "modules found", "");
            display_draw_string("retrying", font_6x8, 28, 56, 0);

            uint8_t state = (kernel_get_time_ms() & (0x3 << 8)) >> 8;
            display_draw_string(".", font_6x8, 82 + (3 * state), 56, 0); 
        }

        display_render_frame();
        kernel_sleep_ms(17);
    }

    while (1) {

        //---- WAIT FOR DATA CAPTURE -----------------------------------------------------------------------------------------------------------------------------

        // wait for all the data to be read. Check communication fault
        while (!load_get_data_capture_done()) {

            if (load_get_checksum_fault()) {

                current_screen = SCREEN_COM_FAULT;
                break;
            }
            
            kernel_yield();
        }

        if (load_get_fault() && current_screen != SCREEN_COM_FAULT) current_screen = SCREEN_FAULT;

        display_flush_frame_buffer();

        //---- GUI -----------------------------------------------------------------------------------------------------------------------------------------------

        bool load_enabled = load_get_enable();
        keypad_set_led(load_enabled);

        switch (current_screen) {

            //==== MAIN SCREEN ===================================================================================================================================

            case SCREEN_MAIN: {

                //---- PARSE DATA --------------------------------------------------------------------------------------------------------------------------------

                load_mode_t load_mode = load_get_mode();
                uint32_t load_voltage_mv = load_get_voltage_mv();
                uint32_t load_current_ma = load_get_total_current_ma();
                uint32_t load_power_mw = load_get_power_mw();
                uint8_t load_temp = load_get_temp();
                uint32_t iset_ma = load_get_cc_level_ma();
                uint32_t vset_mv = load_get_cv_level_mv();
                uint32_t rset_mr = load_get_cr_level_mr();
                uint32_t pset_mw = load_get_cp_level_mw();
                uint32_t total_mah = load_get_total_mah();
                uint32_t total_mwh = load_get_total_mwh();
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

                //---- HANDLE CONTROLS AND SCREEN SWITCHES -------------------------------------------------------------------------------------------------------

                static uint32_t setting_increment = 0;
                static uint32_t last_setpoint_change_time = 0;

                if (setting_increment > 0 && kernel_get_time_since(last_setpoint_change_time) > 4000) setting_increment = 0;

                // buttons
                if (keypad_is_pressed(KEY_MODE, true) && !load_enabled) {

                    if      (load_mode == LOAD_MODE_CC) load_set_mode(LOAD_MODE_CV);
                    else if (load_mode == LOAD_MODE_CV) load_set_mode(LOAD_MODE_CR);
                    else if (load_mode == LOAD_MODE_CR) load_set_mode(LOAD_MODE_CP);
                    else                                load_set_mode(LOAD_MODE_CC);

                    setting_increment = 0;
                }

                if (keypad_is_pressed(KEY_SET, true) || keypad_is_pressed(KEY_ENCODER, true)) {

                    if (setting_increment == 0) {

                        if (load_mode == LOAD_MODE_CC) setting_increment = 1000;
                        if (load_mode == LOAD_MODE_CV) setting_increment = 1000;
                        if (load_mode == LOAD_MODE_CR) setting_increment = 1000;
                        if (load_mode == LOAD_MODE_CP) setting_increment = 10000;

                    } else {

                        setting_increment /= 10;

                        if ((load_mode == LOAD_MODE_CC) && (setting_increment < 100)) setting_increment = 1000;
                        if ((load_mode == LOAD_MODE_CV) && (setting_increment < 100)) setting_increment = 10000;
                        if ((load_mode == LOAD_MODE_CR) && (setting_increment < 100)) setting_increment = 10000;
                        if ((load_mode == LOAD_MODE_CP) && (setting_increment < 1000)) setting_increment = 10000;
                    }

                    last_setpoint_change_time = kernel_get_time_ms();
                }

                if (keypad_is_pressed(KEY_EN, true)) load_set_enable(!load_enabled);
                if (keypad_is_pressed_for_ms(KEY_MENU, 1000, true)) bug_enabled = !bug_enabled;

                // rotary encoder
                int16_t encoder_delta = keypad_get_encoder_pos();

                if (encoder_delta != 0) {

                    last_setpoint_change_time = kernel_get_time_ms();

                    if      (load_mode == LOAD_MODE_CC) load_set_cc_level(iset_ma + setting_increment * encoder_delta);
                    else if (load_mode == LOAD_MODE_CV) load_set_cv_level(vset_mv + setting_increment * encoder_delta);
                    else if (load_mode == LOAD_MODE_CR) load_set_cr_level(rset_mr + setting_increment * encoder_delta);
                    else if (load_mode == LOAD_MODE_CP) load_set_cp_level(pset_mw + setting_increment * encoder_delta);
                }

                //---- RENDER UI ---------------------------------------------------------------------------------------------------------------------------------

                // enable time
                uint8_t cursor_pos = 0;

                if (hours > 0) {

                    display_draw_int(hours, font_6x8, cursor_pos, 0, &cursor_pos);
                    display_draw_char(':', font_6x8, cursor_pos, 0, &cursor_pos);
                }

                display_draw_int(minutes, font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_char(':', font_6x8, cursor_pos, 0, &cursor_pos);

                if (seconds < 10) display_draw_char('0', font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_int(seconds, font_6x8, cursor_pos, 0, &cursor_pos);

                // total mAh
                __print_decimal_with_unit(total_mah * 1000, "mAh", font_6x8, 0, 8, 0, 0);

                // total mWh
                if (total_mwh < 1000)           __print_decimal_with_unit(total_mwh * 1000, "mWh", font_6x8, 0, 16, 0, 0);      // 999mWh
                else if (total_mwh < 10000)     __print_decimal_with_unit(total_mwh, "Wh", font_6x8, 0, 16, 2, 0);              // 9.99Wh
                else if (total_mwh < 100000)    __print_decimal_with_unit(total_mwh, "Wh", font_6x8, 0, 16, 1, 0);              // 99.9Wh
                else if (total_mwh < 10000000)  __print_decimal_with_unit(total_mwh, "Wh", font_6x8, 0, 16, 0, 0);              // 9999Wh
                else if (total_mwh < 100000000) __print_decimal_with_unit(total_mwh / 1000, "kWh", font_6x8, 0, 16, 0, 0);      // 99kWh
                else                            display_draw_string(">99kWh", font_6x8, 0, 16, 0);                              // >99kWh

                // NO_REG flag
                if (load_get_not_in_reg()) display_draw_string("REG!", font_6x8, 48, 0, 0);

                // temperature
                display_draw_bitmap(bitmap_temp_icon_9x8[!!(kernel_get_time_ms() & (1 << 9)) & load_enabled], 92, 0);
                
                cursor_pos = 104;
                if (load_temp < 10) display_draw_char(' ', font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_int(load_temp, font_6x8, cursor_pos, 0, &cursor_pos);
                display_draw_string("\'C", font_6x8, cursor_pos, 0, &cursor_pos);

                //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                // load mode and setpoint
                if (load_mode == LOAD_MODE_CC) {
                        
                    display_draw_string("CC", font_6x8, 0, 56, 0);

                    // draw cursor icon
                    if (setting_increment != 0) {

                        uint8_t arrow_pos = 0;

                        if (setting_increment == 100) arrow_pos = 10;
                        if (setting_increment == 1000) arrow_pos = 0;

                        if (iset_ma >= 10000) arrow_pos += 6;

                        display_draw_bitmap(bitmap_cursor_arrow_6x8, arrow_pos, 40);
                    }

                    __print_decimal_with_unit(iset_ma, "A", font_6x8, 0, 48, 1, 0);

                //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                } else if (load_mode == LOAD_MODE_CV) {
                    
                    display_draw_string("CV", font_6x8, 0, 56, 0);

                    // draw cursor icon
                    if (setting_increment != 0) {

                        uint8_t arrow_pos = 0;

                        if (setting_increment == 100) arrow_pos = 16;
                        if (setting_increment == 1000) arrow_pos = 6;
                        if (setting_increment == 10000) arrow_pos = 0;

                        if (vset_mv < 10000 && setting_increment < 10000) arrow_pos -= 6;

                        display_draw_bitmap(bitmap_cursor_arrow_6x8, arrow_pos, 40);
                    }

                    uint8_t cursor_pos = 0;
                    if (setting_increment >= 10000 && vset_mv < 10000) display_draw_char(' ', font_6x8, 0, 48, &cursor_pos);
                    __print_decimal_with_unit(vset_mv, "V", font_6x8, cursor_pos, 48, 1, &cursor_pos);

                //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                } else if (load_mode == LOAD_MODE_CR) {
                    
                    display_draw_string("CR", font_6x8, 0, 56, 0);

                    // draw cursor icon
                    if (setting_increment != 0) {

                        uint8_t arrow_pos = 0;

                        if (setting_increment == 100) arrow_pos = 22;
                        if (setting_increment == 1000) arrow_pos = 12;
                        if (setting_increment == 10000) arrow_pos = 6;

                        if (rset_mr < 100000) arrow_pos -= 6;
                        if (rset_mr < 10000 && arrow_pos != 0) arrow_pos -= 6;

                        display_draw_bitmap(bitmap_cursor_arrow_6x8, arrow_pos, 40);
                    }

                    uint8_t cursor_pos = 0;
                    if (setting_increment >= 10000 && rset_mr < 10000) display_draw_char(' ', font_6x8, 0, 48, &cursor_pos);
                    __print_decimal(rset_mr, font_6x8, cursor_pos, 48, 1, &cursor_pos);
                    display_draw_bitmap(bitmap_omega_6x8, cursor_pos, 48);

                //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                } else if (load_mode == LOAD_MODE_CP) {
                    
                    display_draw_string("CP", font_6x8, 0, 56, 0);

                    // draw cursor icon
                    if (setting_increment != 0) {

                        uint8_t arrow_pos = 0;

                        if (setting_increment == 1000) arrow_pos = 12;
                        if (setting_increment == 10000) arrow_pos = 6;

                        if (pset_mw < 100000) arrow_pos -= 6;
                        if (pset_mw < 10000 && setting_increment < 10000) arrow_pos -= 6;

                        display_draw_bitmap(bitmap_cursor_arrow_6x8, arrow_pos, 40);
                    }

                    uint8_t cursor_pos = 0;
                    if (setting_increment >= 10000 && pset_mw < 10000) display_draw_char(' ', font_6x8, 0, 48, &cursor_pos);
                    __print_decimal_with_unit(pset_mw, "W", font_6x8, cursor_pos, 48, 0, &cursor_pos);
                }

                //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                // total current
                cursor_pos = 44;
                if (load_current_ma >= 10000) __print_decimal(load_current_ma, font_16x24, cursor_pos, 16, 1, &cursor_pos);
                else __print_decimal(load_current_ma, font_16x24, cursor_pos, 16, 2, &cursor_pos);
                display_draw_string(" A", font_6x8, cursor_pos, 32, &cursor_pos);

                // voltage and power
                __print_decimal_with_unit(load_voltage_mv, "V", font_6x8, 48, 48, 2, 0);
                __print_decimal_with_unit(load_power_mw, "W", font_6x8, 48, 56, 1, 0);

                // load ON/OFF icon
                if (keypad_is_pressed(KEY_MODE, false) && load_enabled && keypad_get_hold_time(KEY_MODE) < 500) {

                    display_draw_bitmap_not_aligned(load_enabled ? bitmap_on_button_32x16 : bitmap_off_button_32x16, 96, 50, true);
                
                } else display_draw_bitmap(load_enabled ? bitmap_on_button_32x16 : bitmap_off_button_32x16, 96, 48);

                //------------------------------------------------------------------------------------------------------------------------------------------------

            } break;

            //==== COMMUNICATION TIMEOUT SCREEN ==================================================================================================================

            case SCREEN_COM_FAULT: {

                if (!load_get_checksum_fault()) current_screen = SCREEN_MAIN;

                __print_error_window(bitmap_sad_emoji_32x32, "ERROR", "Load module", "not responding", "");
                display_draw_string("retrying", font_6x8, 28, 56, 0);

                uint8_t state = (kernel_get_time_ms() & (0x3 << 8)) >> 8;
                display_draw_string(".", font_6x8, 82 + (3 * state), 56, 0); 

            } break;

            //==== LOAD FAULT SCREEN =============================================================================================================================

            case SCREEN_FAULT: {

                //---- PARSE DATA --------------------------------------------------------------------------------------------------------------------------------

                load_fault_t fault_register = load_get_fault_flags() & load_get_fault_mask();
                load_fault_t current_fault = 0;

                // find the first set fault flag
                for (int fault = 0; fault < 16; fault++) {
                    
                    if (fault_register & (1 << fault)) {

                        current_fault = 1 << fault;
                        break;
                    }
                }

                //---- HANDLE CONTROLS AND SCREEN SWITCHES -------------------------------------------------------------------------------------------------------

                if (!load_get_fault()) current_screen = SCREEN_MAIN;

                if (keypad_is_pressed_for_ms(KEY_SET, 500, true)) load_clear_fault(current_fault);

                //---- RENDER UI ---------------------------------------------------------------------------------------------------------------------------------

                if      (current_fault == LOAD_FAULT_COM) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Interface", "panel", "disconnected");
                else if (current_fault == LOAD_FAULT_REG) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Regulator", "in saturation", "");
                else if (current_fault == LOAD_FAULT_OTP) __print_error_window(bitmap_fault_transistor_32x48, "LOAD FAULT", "Overtemperature", "protection", "triggered");
                else if (current_fault == LOAD_FAULT_TEMP_L) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Left temp", "sensor not", "working");
                else if (current_fault == LOAD_FAULT_TEMP_R) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Right temp", "sensor not", "working");
                else if (current_fault == LOAD_FAULT_FAN1) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Bottom fan", "not spinning", "");
                else if (current_fault == LOAD_FAULT_FAN2) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Top fan", "not spinning", "");
                else if (current_fault == LOAD_FAULT_OCP) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "Overcurrent", "protection", "triggered");
                else if (current_fault == LOAD_FAULT_OPP) __print_error_window(bitmap_fault_transistor_32x48, "LOAD FAULT", "Overpower", "protection", "triggered");
                else if (current_fault == LOAD_FAULT_FUSE_L1) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "L1 sink", "no current", "check fuse");
                else if (current_fault == LOAD_FAULT_FUSE_L2) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "L2 sink", "no current", "check fuse");
                else if (current_fault == LOAD_FAULT_FUSE_R1) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "R1 sink", "no current", "check fuse");
                else if (current_fault == LOAD_FAULT_FUSE_R2) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "R2 sink", "no current", "check fuse");
                else if (current_fault == LOAD_FAULT_EXTERNAL) __print_error_window(bitmap_sad_emoji_32x32, "LOAD FAULT", "EXTERNAL fault", "triggered", "");

                display_draw_string("clear ", font_6x8, 48, 56, 0);

                if (keypad_is_pressed(KEY_SET, false) && keypad_get_hold_time(KEY_SET) < 500) {

                    display_draw_bitmap(bitmap_progress_bar_32x8[(keypad_get_hold_time(KEY_SET) >> 6) & 0x7], 48, 56);
                }

            } break;

            //====================================================================================================================================================

            default:
                break;
        }

        //---- OVERLAY -------------------------------------------------------------------------------------------------------------------------------------------

        if (bug_enabled) {

            display_draw_bitmap_not_aligned(bitmap_bug_16x18[!!(kernel_get_time_ms() & (1 << 8))], bug_pos_x, bug_pos_y, true);      // ANIMATED

            bug_pos_x += (bug_dir_x) ? 1 : -1;
            bug_pos_y += (bug_dir_y) ? 1 : -1;

            if (bug_pos_x == 0) bug_dir_x = 1;
            if (bug_pos_x >= DISPLAY_WIDTH - 16) bug_dir_x = 0;
            if (bug_pos_y == 0) bug_dir_y = 1;
            if (bug_pos_y >= DISPLAY_HEIGHT - 18) bug_dir_y = 0;
        }

        //--------------------------------------------------------------------------------------------------------------------------------------------------------

        display_render_frame();
        load_start_data_capture();

        kernel_sleep_ms(17);
    }
}

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

void __print_error_window(const uint8_t *icon, char *header, char *line1, char *line2, char *line3) {

    display_flush_frame_buffer();
    display_draw_bitmap(icon, 8, 8);
    display_draw_string(header, font_6x8, 48, 8, 0);
    display_draw_string(line1, font_6x8, 48, 24, 0);
    display_draw_string(line2, font_6x8, 48, 32, 0);
    display_draw_string(line3, font_6x8, 48, 40, 0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void __print_decimal_with_unit(int value, char *unit, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor) {

    uint8_t cursor_pos = x_pos;
    __print_decimal(value, font, cursor_pos, y_pos, dec_places, &cursor_pos);
    display_draw_string(unit, font, cursor_pos, y_pos, &cursor_pos);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void __print_decimal(int value, const uint8_t *font, uint8_t x_pos, uint8_t y_pos, uint8_t dec_places, uint8_t *cursor) {

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
        if (i == 3 && dec_places > 0) string[buff_index++] = '.';
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
