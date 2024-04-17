#include "common_defs.h"
#include "debug_uart.h"
#include "load.h"

//---- INTERNAL FUNCTIONS ----------------------------------------------------------------------------------------------------------------------------------------

#define SHELL_CMD(command) (strcmp(args[0], (command)) == 0)
#define COMPARE_ARG(argnum, str) (strcmp(args[(argnum)], (str)) == 0)

// prints an error message if the user did't provide enough arguments for the command
#define shell_assert_argc(count) if (argc < count + 1) { \
                                    debug_print("too few arguments for \""); \
                                    debug_print(args[0]); \
                                    debug_print("\"\n"); \
                                    return; \
                                }

uint8_t __parse_args(char *input, char **args, uint8_t argc);
uint32_t __parse_hex_string(char *string);

//---- FUNCTIONS -------------------------------------------------------------------------------------------------------------------------------------------------

// prints the shell header on startup
void shell_print_header(void) {

    debug_print("----------------------------------------------------------\n");
    debug_print("  @@@@&        @@@@     @@@@   | 400W DC Electronic Load\n");
    debug_print("  @@@@@&      @@@@@   @@@@     | Interface Panel\n");
    debug_print("  @@@@@@@    @@@@@@ @@@@       | \n");
    debug_print("  @@@ @@@@  @@@ @@@@@@@@       | \n");
    debug_print("  @@@  @@@@@@@  @@@   @@@@     | \n");
    debug_print("  @@@   @@@@@   @@@    @@@@@   | Martin Kopka 2024\n");
    debug_print("  @@@    @@@    @@@      @@@@  | github.com/m-kopka\n");
    debug_print("----------------------------------------------------------\n");
    debug_print("(i) type \"help\" to show available commands.\n");
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// detects commands from the DEBUG_UART and responds to them
void shell_update(char *buffer) {

    if (buffer[0] == '\0') return;      // no command
    while ((buffer[0] <= ' ') && (buffer[0] != '\0')) buffer++;

    char *args[5];
    uint8_t argc = __parse_args(buffer, args, 5);

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    if (SHELL_CMD("ahoj")) debug_print("zdar jak sviňa, su z Brna\n");

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    else if (SHELL_CMD("repeat")) {

        if (argc == 1) debug_repeat_stop();

        else if (argc >= 3) {

            int period = atoi(args[1]);

            if (period >= 10) debug_repeat_start(args[2], period);
            else debug_print("(!) minimum period is 10 ms.\n");

        } else {

            debug_print("(!) usage: repeat \"<command>\" <period_ms>\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // resets the processor
    else if (SHELL_CMD("reboot")) {

        debug_print("restarting..\n");
        kernel_sleep_ms(200);
        NVIC_SystemReset();
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // enables or disables the load
    else if (SHELL_CMD("en")) {

        shell_assert_argc(1);

        if (COMPARE_ARG(1, "0")) {

            load_set_enable(false);
            debug_print("load disabled.\n");
        }

        else if (COMPARE_ARG(1, "1")) {

            load_set_enable(true);
            debug_print("load enabled.\n");
        }

        else debug_print("invalid argument.\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CC level
    else if (SHELL_CMD("iset")) {

        shell_assert_argc(1);

        int current = atoi(args[1]);
        if (current >= 0 && current <= 10000) {     // limit to 10A for now

            load_set_cc_level((uint16_t)current);

            debug_print("CC level set to ");
            debug_print_int_dec(current, 2);
            debug_print(" mA.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // prints the load voltage
    else if (SHELL_CMD("vsen")) {

        debug_print("load input voltage: ");
        debug_print_int_dec(load_get_voltage_mv(), 2);
        debug_print(" V\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // prints total load current
    else if (SHELL_CMD("isen")) {

        debug_print("load current: ");
        debug_print_int_dec(load_get_total_current_ma(), 2);
        debug_print(" A\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // prints total load current
    else if (SHELL_CMD("psen")) {

        debug_print("load power: ");
        debug_print_int_dec(load_get_voltage_mv() * load_get_total_current_ma() / 1000, 2);
        debug_print(" W\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // prints all internal current sink currents
    else if (SHELL_CMD("help")) {

        debug_print("list of available commands:\n");
        debug_print("reboot - restart the system\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    else debug_print("(!) unknown command.\n");
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// separates an input string into a command and arguments all separated by spaces; arguments with spaces are possible by using quotation marks
// caution: the original string is modified by the function; the function inserts null-terminators after each argument and returns pointers to the arguments in the original string
uint8_t __parse_args(char *input, char **args, uint8_t argc) {

    uint8_t  current_arg = 0;                       // index of current argument
    bool     parsing = false;                       // currently parsing an argument, not looking for a beginning of a new argument
    bool     prev_was_space = true;                 // previous char was a space
    bool     parse_until_quotation_mark = false;    // current argument started with a quotation mark, parse until next quotation mark and ignore spaces

    while (*input != '\0') {

        // detect the end of current argument
        if ((!parse_until_quotation_mark && !prev_was_space && (*input == ' ')) ||      // argument didn't start with a quotation mark, previous char was alphanumerical and current char is a space (argument <--)
             (parse_until_quotation_mark && *input == '\"')) {                          // argument started with a quotation mark and current char is the closing quotation mark ("argument"<--)

            *input = '\0';             // NULL terminate the argument in the input buffer
            parse_until_quotation_mark = false;
            parsing = false;

            if (current_arg == argc) break;     // stop parsing after the end of last argument was detected
        }

        // detect the start of a new argument
        // we are currently not parsing an argument in quotation marks, previous char was a space, current char is alphanumerical
        if (prev_was_space && (*input > ' ')) {

            if (*input == '\"') parse_until_quotation_mark = true;                  // argument started with a quotation mark
            else if (!parsing) {
                
                args[current_arg++] = input;      // mark start of a new argument
                parsing = true;
            }
        }

        prev_was_space = (*input == '\0') || (*input == ' ') || (*input == '\"');
        input++;
    }

    *input = '\0';

    return (current_arg);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// converts a string containing a hexadecimal number to an unsigned integer
uint32_t __parse_hex_string(char *string) {

    uint32_t result = 0;

    for (int i = 0; string[i] != '\0'; ++i) {
        
        if      (string[i] >= '0' && string[i] <= '9') result = result * 16 + string[i] - '0';
        else if (string[i] >= 'a' && string[i] <= 'f') result = result * 16 + 10 + string[i] - 'a';
        else if (string[i] >= 'A' && string[i] <= 'F') result = result * 16 + 10 + string[i] - 'A';
    }

    return result;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
