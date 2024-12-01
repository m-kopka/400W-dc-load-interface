#include "keypad.h"
#include "hal/pwm.h"

//---- CONSTANTS -------------------------------------------------------------------------------------------------------------------------------------------------

// possible encoder detector state machine states
typedef enum {

    ENC_IDLE      = 0x00,   // end position (no step detected)
    ENC_IDLE_CW   = 0x10,   // end position after clockwise step detected (fourth step in CW step 10 -> 11)
    ENC_IDLE_CCW  = 0x20,   // end position after counter-clockwise step detected (fourth step in CCW step 01 -> 11)
    ENC_CW_FIRST  = 0x01,   // first transition in CW step (11 -> 01)
    ENC_CW_HALF   = 0x02,   // second transition in CW step (01 -> 00)
    ENC_CW_LAST   = 0x03,   // third transition in CW step (00 -> 10)
    ENC_CCW_FIRST = 0x04,   // first transition in CW step (11 -> 10)
    ENC_CCW_HALF  = 0x05,   // second transition in CW step (10 -> 00)
    ENC_CCW_LAST  = 0x06    // third transition in CW step (01 -> 01)

} encoder_detector_state_t;

// index table for determining next valid state based on current encoder detector state machine state
// first index is current state, second index is current encoder pin state combination
// for example, valid clockwise rotation is detected by transitioning through folowing states after pin state transitions: IDLE (11), 01 -> CW_FIRST, 00 -> CW_HALF, 10 -> CW_LAST, 11 -> IDLE_CW
const encoder_detector_state_t encoder_valid_states[7][4] = {

  {ENC_IDLE,     ENC_CW_FIRST,  ENC_CCW_FIRST, ENC_IDLE},       // next states from ENC_IDLE, ENC_IDLE_CW or ENC_IDLE_CCW states
  {ENC_CW_HALF,  ENC_CW_FIRST,  ENC_IDLE,      ENC_IDLE},       // next states from ENC_CW_FIRST state
  {ENC_CW_HALF,  ENC_CW_FIRST,  ENC_CW_LAST,   ENC_IDLE},       // next states from ENC_CW_HALF state
  {ENC_CW_HALF,  ENC_IDLE,      ENC_CW_LAST,   ENC_IDLE_CW},    // next states from ENC_CW_LAST state
  {ENC_CCW_HALF, ENC_IDLE,      ENC_CCW_FIRST, ENC_IDLE},       // next states from ENC_CCW_FIRST state
  {ENC_CCW_HALF, ENC_CCW_LAST,  ENC_CCW_FIRST, ENC_IDLE},       // next states from ENC_CCW_HALF state
  {ENC_CCW_HALF, ENC_CCW_LAST,  ENC_IDLE,      ENC_IDLE_CCW},   // next states from ENC_CCW_LAST state
};

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

static uint32_t key_state = 0;                          // bit field containing debounced states of push buttons
static uint32_t key_press_time_ms[KEY_COUNT] = {0};     // time of key press [ms]
static uint32_t key_press_acknowledged = 0;             // bit corresponding to a key is set after keypad_is_pressed() is called with do_once flag set, reset after button release

encoder_detector_state_t encoder_detector_state = ENC_IDLE;     // encoder detector state machine
volatile int32_t encoder_pos = 0;                               // encoder knob position, reset after position is read via keypad_get_encoder_pos() function call

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

static inline bool get_key_state(uint32_t key) {return (key_state & (1 << key));}
static inline bool get_key_acknowledged(uint32_t key) {return (key_press_acknowledged & (1 << key));}

//---- KERNEL TASKS ----------------------------------------------------------------------------------------------------------------------------------------------

// kernel task for handling push buttons and their debouncing
void keypad_buttons_task(void) {

    // index table, returns gpio number for a specific key
    uint8_t key_gpio[KEY_COUNT] = {KEY_ENCODER_GPIO, 
                                   KEY_MODE_GPIO,    KEY_SET_GPIO,
                                   KEY_SEQ_GPIO,     KEY_SEQ_EN_GPIO,
                                   KEY_MENU_GPIO,    KEY_EN_GPIO};

    for (uint8_t key = 0; key < KEY_COUNT; key++) {

        gpio_set_dir(key_gpio[key], GPIO_DIR_INPUT);
        gpio_set_pull(key_gpio[key], GPIO_PULLUP);
    }

    pwm_init();
    pwm_set_resolution(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), 12);
    pwm_set_frequency(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), 1000);
    keypad_set_led(false);
    gpio_set_function(KEY_BACKLIGHT_LED_GPIO, GPIO_FUNC_PWM);
    pwm_set_enable(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), true);

    gpio_set_dir(ENCODER_A_GPIO, GPIO_DIR_INPUT);
    gpio_set_dir(ENCODER_B_GPIO, GPIO_DIR_INPUT);
    gpio_set_irq(ENCODER_A_GPIO, GPIO_IRQ_EDGE_LOW | GPIO_IRQ_EDGE_HIGH, true);
    gpio_set_irq(ENCODER_B_GPIO, GPIO_IRQ_EDGE_LOW | GPIO_IRQ_EDGE_HIGH, true);

    uint8_t debounce_shift_register[KEY_COUNT] = {0xf};

    while (1) {

        // loop through each key
        for (uint8_t key = 0; key < KEY_COUNT; key++) {

            bool debouncer_input = gpio_get(key_gpio[key]);     // sample input state

            // output is not yet updated and 4 consecutive input samples were identical (input settled)
            if ((debouncer_input == get_key_state(key)) && ((debounce_shift_register[key] == 0x0) || (debounce_shift_register[key] == 0xf))) {

                // key released
                if (debouncer_input) {

                    clear_bits(key_state, (1 << key));
                    clear_bits(key_press_acknowledged, (1 << key));

                // key pressed
                } else {
                        
                    set_bits(key_state, (1 << key));
                    clear_bits(key_press_acknowledged, (1 << key));
                    key_press_time_ms[key] = kernel_get_time_ms();
                }
            }

            debounce_shift_register[key] = ((debounce_shift_register[key] << 1) | debouncer_input) & 0xf;   // shift in the new sample
        }

        kernel_sleep_ms(5);
    }
}

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// returns a key state. If do_once flag is used, function returns true only the first time the key press is registered
bool keypad_is_pressed(enum key_t key, bool do_once) {

    if (get_key_state(key)) {

        if (!(do_once && get_key_acknowledged(key))) {

            if (do_once) set_bits(key_press_acknowledged, (1 << key));
            return true;

        } else return false;

    } else return false;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns key hold time [ms]
uint32_t keypad_get_hold_time(enum key_t key) {

    if (!keypad_is_pressed(key, false)) return 0;
    else return (kernel_get_time_ms() - key_press_time_ms[key]);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns true if key is pressed and was held for specified ammount of time
bool keypad_is_pressed_for_ms(enum key_t key, uint32_t time, bool do_once) {

    if ((keypad_get_hold_time(key) >= time) && keypad_is_pressed(key, do_once)) return true;
    else return false;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets key backlight LED state
void keypad_set_led(bool state) {

    pwm_set_duty(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), pwm_gpio_to_channel(KEY_BACKLIGHT_LED_GPIO), (~(state * KEY_BACKLIGHT_LED_BRIGHTNESS) & 0xfff));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns encoder position change since the last call of this function
int32_t keypad_get_encoder_pos(void) {

    int32_t position = encoder_pos;
    encoder_pos = 0;

    return position;
}

//---- INTERRUPT HANDLERS ----------------------------------------------------------------------------------------------------------------------------------------

// GPIO interrupt handles encoder rotation detection
void IO_Bank0_Handler(void) {

    // read pin states
    uint8_t pinstate = (gpio_get(ENCODER_A_GPIO) << 1) | gpio_get(ENCODER_B_GPIO);

    // determine new detector state machine state based on current state and pin states
    encoder_detector_state = encoder_valid_states[encoder_detector_state & 0xf][pinstate];

    // catch transition to idle state after a complete step
    if (encoder_detector_state == ENC_IDLE_CCW) encoder_pos--;
    if (encoder_detector_state == ENC_IDLE_CW ) encoder_pos++;

    gpio_acknowledge_irq(ENCODER_A_GPIO);
    gpio_acknowledge_irq(ENCODER_B_GPIO);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
