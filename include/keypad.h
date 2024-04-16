#ifndef _KEYPAD_H_
#define _KEYPAD_H_

/*
 * Keypad and rotary encoder driver
 * Martin Kopka 2024
*/

#include "common_defs.h"

// keypad key numbers
enum key_t {

    KEY_ENCODER = 0,
    KEY_MODE    = 1,
    KEY_SET     = 2,
    KEY_SEQ     = 3,
    KEY_SEQ_EN  = 4,
    KEY_MENU    = 5,
    KEY_EN      = 6
};

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// kernel task for handling push buttons and their debouncing
void keypad_buttons_task(void);

// returns key state. If do_once flag is used, function returns true only first time, key press is registered
bool keypad_is_pressed(enum key_t key, bool do_once);

// returns key hold time [ms]
uint32_t keypad_get_hold_time(enum key_t key);

// returns true if key is pressed and was held for specified ammount of time
bool keypad_is_pressed_for_ms(enum key_t key, uint32_t time_ms, bool do_once);

// sets key backlight LED state
void keypad_set_led(bool state);

// returns encoder position change since last call of this function
int32_t keypad_get_encoder_pos(void);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _KEYPAD_H_ */