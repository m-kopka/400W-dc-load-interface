#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*
 *  Display function library
 *  Martin Kopka 2024
 * 
 *  features:
 *  - double frame buffering algorithm sends only segments that were changed since last frame
 *  - draw bitmaps anywhere on the screen
 *  - printing strings and integers
*/

#include "common_defs.h"
#include "display_driver/fonts.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initialize the display
void display_init(void);

// flush active frame buffer
void display_flush_frame_buffer(void);

// check for differences with last frame and send new data to display RAM
void display_render_frame(void);

// clear specified area in active screen buffer - fast (y coordinate must be page aligned i.e. divisible by 8)
void display_clear_area(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height);

// draw bitmap on specified coordinates - fast (y coordinate must be page aligned i.e. divisible by 8) (0, 0 = top left corner)
void display_draw_bitmap(const uint8_t* bitmap, uint8_t x_pos, uint8_t y_pos);

// draw bitmap on specified coordinates (0,0 = top left corner)
void display_draw_bitmap_not_aligned(const uint8_t* bitmap, uint8_t x_pos, uint8_t y_pos, bool transparency);

// print single character on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_char(char c, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor);

// print string on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_string(char *str, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor);

// print integer on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_int(int num, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _DISPLAY_H_ */