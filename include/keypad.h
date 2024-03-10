#ifndef _KEYPAD_H_
#define _KEYPAD_H_

/*
 * Keypad and rotary encoder driver
 * Martin Kopka 2024
*/

#include "common_defs.h"

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// kernel task for handling push buttons and their debouncing
void keypad_buttons_task(void);

// returns key state. If do_once flag is used, function returns true only first time, key press is registered
bool keypad_is_pressed(uint8_t key, bool do_once);

// returns key hold time [ms]
uint32_t keypad_get_hold_time(uint8_t key);

// returns true if key is pressed and was held for specified ammount of time
bool keypad_is_pressed_for_ms(uint8_t key, uint32_t time_ms, bool do_once);

// sets key backlight LED state
void keypad_set_led(bool state);

// returns encoder position change since last call of this function
int32_t keypad_get_encoder_pos(void);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _KEYPAD_H_ */