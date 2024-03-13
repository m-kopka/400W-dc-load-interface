#include "keypad.h"
#include "hal/pwm.h"

//---- INTERNAL DATA ---------------------------------------------------------------------------------------------------------------------------------------------

static uint32_t key_state = 0;                          // bit field containing debounced states of push buttons
static uint32_t key_press_time_ms[KEY_COUNT] = {0};     // time of key press [ms]
static uint32_t key_press_acknowledged = 0;

volatile int32_t  encoder_pos = 0;              // encoder knob position, reset after position is read via keypad_get_encoder_pos() function call
volatile bool     prev_a = false;               
volatile bool     debounced_a = false;
volatile uint32_t prev_turn_time_ms = 0;

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

static inline bool get_key_state(uint32_t key) {return (key_state & (1 << key));}
static inline bool get_key_acknowledged(uint32_t key) {return (key_press_acknowledged & (1 << key));}

//---- KERNEL TASKS ----------------------------------------------------------------------------------------------------------------------------------------------

// kernel task for handling push buttons and their debouncing
void keypad_buttons_task(void) {

    uint8_t key_debounce_states[7] = {0};

    gpio_set_dir(KEY0_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY0_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY1_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY1_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY2_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY2_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY3_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY3_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY4_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY4_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY5_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY5_GPIO, GPIO_PULLUP);
    gpio_set_dir(KEY6_GPIO, GPIO_DIR_INPUT);
    gpio_set_pull(KEY6_GPIO, GPIO_PULLUP);

    /*pwm_init();
    pwm_set_resolution(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), 12);
    pwm_set_frequency(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), 1000);
    keypad_set_led(false);
    gpio_set_function(KEY_BACKLIGHT_LED_GPIO, GPIO_FUNC_PWM);
    pwm_set_enable(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), true);*/

    gpio_set_dir(ENCODER_A_GPIO, GPIO_DIR_INPUT);
    gpio_set_dir(ENCODER_B_GPIO, GPIO_DIR_INPUT);
    gpio_set_irq(ENCODER_A_GPIO, GPIO_IRQ_EDGE_LOW | GPIO_IRQ_EDGE_HIGH, true);

    while (1) {

        // shift in current key states
        key_debounce_states[0] = (key_debounce_states[0] << 1) | !gpio_get(KEY0_GPIO);
        key_debounce_states[1] = (key_debounce_states[1] << 1) | !gpio_get(KEY1_GPIO);
        key_debounce_states[2] = (key_debounce_states[2] << 1) | !gpio_get(KEY2_GPIO);
        key_debounce_states[3] = (key_debounce_states[3] << 1) | !gpio_get(KEY3_GPIO);
        key_debounce_states[4] = (key_debounce_states[4] << 1) | !gpio_get(KEY4_GPIO);
        key_debounce_states[5] = (key_debounce_states[5] << 1) | !gpio_get(KEY5_GPIO);
        key_debounce_states[6] = (key_debounce_states[6] << 1) | !gpio_get(KEY6_GPIO);

        for (uint8_t key = 0; key < KEY_COUNT; key++) {

            // 8 consecutive zeroes, key released and debounced
            if ((key_debounce_states[key] == 0x00) && get_key_state(key)) {

                clear_bits(key_state, (1 << key));
                clear_bits(key_press_acknowledged, (1 << key));
            }

            // 8 consecutive ones, key pressed and debounced
            else if ((key_debounce_states[key] == 0xff) && !get_key_state(key)) {

                set_bits(key_state, (1 << key));
                clear_bits(key_press_acknowledged, (1 << key));
                key_press_time_ms[key] = kernel_get_time_ms();
            }
        }

        kernel_sleep_ms(10);
    }
}

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// returns key state. If do_once flag is used, function returns true only first time, key press is registered
bool keypad_is_pressed(uint8_t key, bool do_once) {

    if (get_key_state(key)) {

        if (!(do_once && get_key_acknowledged(key))) {

            if (do_once) set_bits(key_press_acknowledged, (1 << key));
            return true;

        } else return false;

    } else return false;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns key hold time [ms]
uint32_t keypad_get_hold_time(uint8_t key) {

    if (!keypad_is_pressed(key, false)) return 0;
    else return (kernel_get_time_ms() - key_press_time_ms[key]);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns true if key is pressed and was held for specified ammount of time
bool keypad_is_pressed_for_ms(uint8_t key, uint32_t time, bool do_once) {

    if ((keypad_get_hold_time(key) >= time) && keypad_is_pressed(key, do_once)) return true;
    else return false;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// sets key backlight LED state
void keypad_set_led(bool state) {

    pwm_set_duty(pwm_gpio_to_slice(KEY_BACKLIGHT_LED_GPIO), pwm_gpio_to_channel(KEY_BACKLIGHT_LED_GPIO), (~(state * KEY_BACKLIGHT_LED_BRIGHTNESS) & 0xfff));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// returns encoder position change since last call of this function
int32_t keypad_get_encoder_pos(void) {

    int32_t position = encoder_pos;
    encoder_pos = 0;

    return position;
}

//---- INTERRUPT HANDLERS ----------------------------------------------------------------------------------------------------------------------------------------

// GPIO interrupt handles encoder rotation detection
void IO_Bank0_Handler(void) {

    bool a_state = !gpio_get(ENCODER_A_GPIO);
    bool b_state = !gpio_get(ENCODER_B_GPIO);

    if (a_state != prev_a) {

        prev_a = a_state;

        if (b_state != debounced_a) {

            debounced_a = b_state;

            if (kernel_get_time_ms() - prev_turn_time_ms >= ENCODER_DEBOUNCE_TIME_MS) {

                encoder_pos += (a_state == b_state) ? -1 : 1;
                prev_turn_time_ms = kernel_get_time_ms();
            }
        }
    }

    gpio_acknowledge_irq(ENCODER_A_GPIO);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
