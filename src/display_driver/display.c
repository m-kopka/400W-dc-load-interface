#include "display_driver/display.h"
#include "display_driver/sh1106.h"

//---- CONSTANTS -------------------------------------------------------------------------------------------------------------------------------------------------

#define FRAME_BUFFER_SIZE ((DISPLAY_WIDTH) * (DISPLAY_HEIGHT) / 8)      // frame buffer size in bytes
#define DISPLAY_PAGES ((DISPLAY_HEIGHT) / 8)                            // number of display RAM pages. One page is DISPLAY_WIDTHx8 pixels
#define DISPLAY_PAGE_SEGMENT_SIZE 16                                    // horizontal size of one page segment. Pages are divided into segments and compared to segments of previous frame
#define DISPLAY_PAGE_SEGMENTS_PER_PAGE ((DISPLAY_WIDTH) / (DISPLAY_PAGE_SEGMENT_SIZE))      // number of page segments per page

#define BITMAP_HEADER_SIZE  2   // first 2 bytes of a bitmap are width and height
#define FONT_HEADER_SIZE    2   // font header also contains first character and last character

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

// frame buffers are swapped each frame. Rendering logic uses inactive buffer to detect pixel data changes since last frame
static uint8_t frame_buffer_0[FRAME_BUFFER_SIZE];
static uint8_t frame_buffer_1[FRAME_BUFFER_SIZE];
static uint8_t *active_frame_buffer;      // currently active frame buffer used for rendering (swapped after current frame is done rendering)

//---- PRIVATE FUNCTIONS -----------------------------------------------------------------------------------------------------------------------------------------

static inline uint8_t bitmap_width(const uint8_t *bitmap) {return (bitmap[0]);}
static inline uint8_t bitmap_height(const uint8_t *bitmap) {return (bitmap[1]);}
static inline    char font_first_char(const uint8_t *font) {return (*((char*)font + 2));}
static inline    char font_last_char(const uint8_t *font) {return (*((char*)font + 3));}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

static inline void __draw_bitmap(const uint8_t* bitmap, uint8_t x_pos, uint8_t y_pos, uint8_t width, uint8_t height) {

    uint32_t frame_buffer_start = DISPLAY_WIDTH * (y_pos >> 3);                         // destination buffer start offset
    uint32_t frame_buffer_end = DISPLAY_WIDTH * ((height + y_pos) >> 3);                // destination buffer end offset
    if (frame_buffer_end > FRAME_BUFFER_SIZE) frame_buffer_end = FRAME_BUFFER_SIZE;     // prevent writing outside the frame buffer area

    uint8_t offset_start = x_pos;                                   // frame buffer starting x axis offset
    uint8_t offset_end = x_pos + width;                             // frame buffer end x axis offset
    if (offset_end > DISPLAY_WIDTH) offset_end = DISPLAY_WIDTH;     // prevent writing outside the frame buffer area

    uint32_t bitmap_offset = 0;     // source buffer offset

    for (uint32_t frame_buffer_pos = frame_buffer_start; frame_buffer_pos < frame_buffer_end; frame_buffer_pos += DISPLAY_WIDTH) {

        for (int offset = offset_start; offset < offset_end; offset++) {
            
            active_frame_buffer[frame_buffer_pos + offset] = bitmap[bitmap_offset++];
        }
    }
}

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// initialize the display
void display_init(void) {

    active_frame_buffer = frame_buffer_1;
    display_flush_frame_buffer();
    active_frame_buffer = frame_buffer_0;
    display_flush_frame_buffer();

    sh1106_init();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// flush active frame buffer
void display_flush_frame_buffer(void) {

    for (uint32_t i = 0; i < FRAME_BUFFER_SIZE; i++) active_frame_buffer[i] = 0x00;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// check for differences with last frame and send new data to display RAM
void display_render_frame(void) {

    uint8_t* prev_frame_buffer = (active_frame_buffer == frame_buffer_0) ? frame_buffer_1 : frame_buffer_0;     // get previous frame pixel data

    // iterate through pixel data and detect changes
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {

        // compare page segments of 16 bytes (16x8 pixels)
        for (uint8_t page_segment = 0; page_segment < DISPLAY_PAGE_SEGMENTS_PER_PAGE; page_segment++) {

            bool page_segment_same = true;

            for (uint8_t i = 0; i < DISPLAY_PAGE_SEGMENT_SIZE; i++) {

                // compare pixel data with last frame. If difference is detected, stop comparing and print page segment
                if (active_frame_buffer[DISPLAY_WIDTH * page + DISPLAY_PAGE_SEGMENT_SIZE * page_segment + i]
                 !=   prev_frame_buffer[DISPLAY_WIDTH * page + DISPLAY_PAGE_SEGMENT_SIZE * page_segment + i]) {page_segment_same = false; break;}
            }

            if (page_segment_same) continue;    // skip rendering of this page segment if same as last frame

            sh1106_send_pixel_data(active_frame_buffer + DISPLAY_WIDTH * page + DISPLAY_PAGE_SEGMENT_SIZE * page_segment, DISPLAY_PAGE_SEGMENT_SIZE, page, DISPLAY_PAGE_SEGMENT_SIZE * page_segment);
            kernel_yield();     // yield resources to other tasks while pixel data is being sent
        }
    }

    active_frame_buffer = (active_frame_buffer == frame_buffer_0) ? frame_buffer_1 : frame_buffer_0;    // swap frame buffers
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// clear specified area in active screen buffer - fast (y coordinate must be page aligned i.e. divisible by 8)
void display_clear_area(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height) {

    uint8_t start_page = start_y >> 3;
    uint8_t end_page = start_page + (height >> 3);
    if (end_page > DISPLAY_PAGES) end_page = DISPLAY_PAGES;         // prevent writing outside the frame buffer area

    uint8_t end_x = start_x + width;
    if (end_x > DISPLAY_WIDTH) end_x = DISPLAY_WIDTH;               // prevent writing outside the frame buffer area

    for (uint8_t page = start_page; page < end_page; page++) {

        for (uint8_t column_address = start_x; column_address < end_x; column_address++) {

            active_frame_buffer[DISPLAY_WIDTH * page + column_address] = 0x00;     // mask out pixels
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// draw bitmap on specified coordinates - fast (y coordinate must be page aligned i.e. divisible by 8) (0, 0 = top left corner)
void display_draw_bitmap(const uint8_t* bitmap, uint8_t x_pos, uint8_t y_pos) {

    __draw_bitmap(bitmap + BITMAP_HEADER_SIZE, x_pos, y_pos, bitmap_width(bitmap), bitmap_height(bitmap));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// draw bitmap on specified coordinates (0,0 = top left corner)
void display_draw_bitmap_not_aligned(const uint8_t* bitmap, uint8_t x_pos, uint8_t y_pos, bool transparency) {

    uint8_t width = bitmap_width(bitmap);
    uint8_t height = bitmap_height(bitmap);

    uint8_t layers = ((y_pos % 8) > 0) ? (height >> 3) + 1 : height >> 3;     // page aligned bitmap spans (area height / 8) pages. non-aligned area spans one page more (first and last pages are covered partialy)

    for (int layer = 0; layer < layers; layer++) {

        if ((y_pos >> 3) + layer >= DISPLAY_PAGES) break;

        for (int x_offset = 0; x_offset < width; x_offset++) {

            if (x_pos + x_offset >= DISPLAY_WIDTH) break;

            uint8_t pixels = 0;
            if (layer == 0) pixels = bitmap[width * layer + x_offset + BITMAP_HEADER_SIZE] << (y_pos % 8);
            else if (layer == ((height >> 3) - 1) && (layers == height >> 3)) pixels = bitmap[width * layer + x_offset + BITMAP_HEADER_SIZE] >> ((y_pos % 8));
            else if (layer == layers - 1) pixels = bitmap[width * (layer - 1) + x_offset + BITMAP_HEADER_SIZE] >> (8 - (y_pos % 8));
            else pixels = (bitmap[width * layer + x_offset + BITMAP_HEADER_SIZE] << (y_pos % 8)) | (bitmap[width * (layer - 1) + x_offset + BITMAP_HEADER_SIZE] >> (8 - (y_pos % 8)));

            if (transparency) active_frame_buffer[DISPLAY_WIDTH * ((y_pos >> 3) + layer) + x_pos + x_offset] |= pixels;
            else              active_frame_buffer[DISPLAY_WIDTH * ((y_pos >> 3) + layer) + x_pos + x_offset]  = pixels;
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// print single character on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_char(char c, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor) {

    if (c < font_first_char(font) || c > font_last_char(font)) return;      // font doesn't contain this character, skip
    if (cursor != 0) *cursor += bitmap_width(font);                         // if a cursor is used, increment it by char width

    __draw_bitmap(font + (bitmap_height(font) >> 3) * bitmap_width(font) * (c - font_first_char(font)) + 4, x_pos, y_pos, bitmap_width(font), bitmap_height(font));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// print string on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_string(char *str, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor) {

    while (*str != '\0') {

        display_draw_char(*str++, font, x_pos, y_pos, cursor);
        x_pos += bitmap_width(font);       // increment x position by character width
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// print integer on screen (y coordinate must be page aligned i.e. divisible by 8)
void display_draw_int(int num, const uint8_t* font, uint8_t x_pos, uint8_t y_pos, uint8_t *cursor) {

    char temp_buff[16];
    itoa(num, temp_buff, 16, 10);

    display_draw_string(temp_buff, font, x_pos, y_pos, cursor);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
