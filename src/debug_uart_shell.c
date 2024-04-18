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

    if (SHELL_CMD("ahoj")) debug_print("zdar jak sviňa, su z Brna.\n");

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // periodically repeats a command
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

    // prints the load status
    else if (SHELL_CMD("status")) {

        uint16_t faults = load_get_fault_flags();
        uint16_t fault_mask = load_get_fault_mask();

        debug_print("\nLOAD STATUS\n");
        debug_print("load is ");
        debug_print((load_get_enable()) ? "ENABLED\n" : "DISABLED\n");
        debug_print("fault state is ");
        debug_print((load_get_fault()) ? "ACTIVE\n" : "not active\n");

        debug_print("\nCOM fault\t");
        debug_print((faults & LOAD_FAULT_COM) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_COM) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("REG fault\t");
        debug_print((faults & LOAD_FAULT_REG) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_REG) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("OTP fault\t");
        debug_print((faults & LOAD_FAULT_OTP) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_OTP) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("OCP fault\t");
        debug_print((faults & LOAD_FAULT_OCP) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_OCP) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("OPP fault\t");
        debug_print((faults & LOAD_FAULT_OPP) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_OPP) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("TEMP_L fault\t");
        debug_print((faults & LOAD_FAULT_TEMP_L) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_TEMP_L) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("TEMP_R fault\t");
        debug_print((faults & LOAD_FAULT_TEMP_R) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_TEMP_R) ? "(ENABLED)\n" : "(masked)\n");
        kernel_sleep_ms(50);
        debug_print("FAN1 fault\t");
        debug_print((faults & LOAD_FAULT_FAN1) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FAN1) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("FAN2 fault\t");
        debug_print((faults & LOAD_FAULT_FAN2) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FAN2) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("FUSE_L1 fault\t");
        debug_print((faults & LOAD_FAULT_FUSE_L1) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FUSE_L1) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("FUSE_L2 fault\t");
        debug_print((faults & LOAD_FAULT_FUSE_L2) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FUSE_L2) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("FUSE_R1 fault\t");
        debug_print((faults & LOAD_FAULT_FUSE_R1) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FUSE_R1) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("FUSE_R2 fault\t");
        debug_print((faults & LOAD_FAULT_FUSE_R2) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_FUSE_R2) ? "(ENABLED)\n" : "(masked)\n");
        debug_print("EXTERNAL fault\t");
        debug_print((faults & LOAD_FAULT_EXTERNAL) ? "ACTIVE\t\t" : "not active\t");
        debug_print((fault_mask & LOAD_FAULT_EXTERNAL) ? "(ENABLED)\n" : "(masked)\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // clears all load fault flags
    else if (SHELL_CMD("clearfaults")) {

        load_clear_fault(LOAD_FAULT_ALL);
        debug_print("all faults cleared.\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    else if (SHELL_CMD("faultmask")) {

        shell_assert_argc(1);

        if (COMPARE_ARG(1, "default")) {

            load_set_fault_mask(LOAD_DEFAULT_FAULT_MASK);
            debug_print("fault mask changed to ");
            debug_print_int_hex(load_get_fault_mask(), 4);
            debug_print(".\n");

        } else {

            int mask = __parse_hex_string(args[1]);

            if (mask >= 0 && mask <= 0xffff) {

                load_set_fault_mask((uint16_t)mask);
                debug_print("fault mask changed to ");
                debug_print_int_hex(load_get_fault_mask(), 4);
                debug_print(".\n");
            }
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // enables or disables the load; returns the load enable state if the command is used without an argument
    else if (SHELL_CMD("en")) {

        if (argc == 1) {

            debug_print("load is ");
            debug_print((load_get_enable()) ? "enabled.\n" : "disabled.\n");

        } else {

            if (!load_get_ready()) debug_print("(!) load is not ready.\n");
            else if (load_get_fault()) debug_print("(!) load is in the fault state.\n");

            else if (COMPARE_ARG(1, "0")) {

                if (!load_get_enable()) debug_print("load is already disabled.\n");
                else {

                    load_set_enable(false);
                    debug_print("load disabled.\n");
                }
            }

            else if (COMPARE_ARG(1, "1")) {

                if (load_get_enable()) debug_print("load is already enabled.\n");
                else {

                   load_set_enable(true);
                    debug_print("load enabled.\n"); 
                }
            }

            else debug_print("(!) invalid argument. Use \"1\" to enable or \"0\" to disable.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CC level
    else if (SHELL_CMD("mode")) {

        if (argc == 1) {

            debug_print("load mode is ");
            if      (load_get_mode() == LOAD_MODE_CC) debug_print("CC.\n");
            else if (load_get_mode() == LOAD_MODE_CV) debug_print("CV.\n");
            else if (load_get_mode() == LOAD_MODE_CR) debug_print("CR.\n");
            else if (load_get_mode() == LOAD_MODE_CP) debug_print("CP.\n");

        } else {

            load_mode_t mode = LOAD_MODE_CC;

            if (COMPARE_ARG(1, "cc")) {

                load_set_mode(LOAD_MODE_CC);
                debug_print("load mode set to CC.\n");

            } else if (COMPARE_ARG(1, "cv")) {

                load_set_mode(LOAD_MODE_CV);
                debug_print("load mode set to CV.\n");

            } else if (COMPARE_ARG(1, "cr")) {
                
                load_set_mode(LOAD_MODE_CR);
                debug_print("load mode set to CR.\n");

            } else if (COMPARE_ARG(1, "cp"))  {

                load_set_mode(LOAD_MODE_CP);
                debug_print("load mode set to CP.\n");

            } else debug_print("(!) invalid argument. Use \"cc\", \"cv\", \"cr\" or \"cp\".\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CC level
    else if (SHELL_CMD("iset")) {

        shell_assert_argc(1);
        int current = atoi(args[1]);

        if (current >= LOAD_MIN_CC_LEVEL_MA && current <= LOAD_MAX_CC_LEVEL_MA) {

            load_set_cc_level((uint16_t)current);

            debug_print("CC level set to ");
            debug_print_int(current);
            debug_print(" mA.\n");

        } else {

            debug_print("(!) iset range is <");
            debug_print_int(LOAD_MIN_CC_LEVEL_MA);
            debug_print(" - ");
            debug_print_int(LOAD_MAX_CC_LEVEL_MA);
            debug_print(">.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CV level
    else if (SHELL_CMD("vset")) {

        shell_assert_argc(1);
        int voltage = atoi(args[1]);

        if (voltage >= LOAD_MIN_CV_LEVEL_MV && voltage <= LOAD_MAX_CV_LEVEL_MV) {

            load_set_cv_level((uint32_t)voltage);

            debug_print("CV level set to ");
            debug_print_int(voltage);
            debug_print(" mV.\n");

        } else {

            debug_print("(!) vset range is <");
            debug_print_int(LOAD_MIN_CV_LEVEL_MV);
            debug_print(" - ");
            debug_print_int(LOAD_MAX_CV_LEVEL_MV);
            debug_print(">.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CR level
    else if (SHELL_CMD("rset")) {

        shell_assert_argc(1);
        int resistance = atoi(args[1]);

        if (resistance >= LOAD_MIN_CR_LEVEL_MR && resistance <= LOAD_MAX_CR_LEVEL_MR) {

            load_set_cr_level((uint32_t)resistance);

            debug_print("CR level set to ");
            debug_print_int(resistance);
            debug_print(" mohm.\n");

        } else {

            debug_print("(!) rset range is <");
            debug_print_int(LOAD_MIN_CR_LEVEL_MR);
            debug_print(" - ");
            debug_print_int(LOAD_MAX_CR_LEVEL_MR);
            debug_print(">.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the load CP level
    else if (SHELL_CMD("pset")) {

        shell_assert_argc(1);
        int power = atoi(args[1]);

        if (power >= LOAD_MIN_CP_LEVEL_MW && power <= LOAD_MAX_CP_LEVEL_MW) {

            load_set_cp_level((uint32_t)power);

            debug_print("CP level set to ");
            debug_print_int(power);
            debug_print(" mW.\n");

        } else {

            debug_print("(!) pset range is <");
            debug_print_int(LOAD_MIN_CP_LEVEL_MW);
            debug_print(" - ");
            debug_print_int(LOAD_MAX_CP_LEVEL_MW);
            debug_print(">.\n");
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
        debug_print_int_dec(load_get_power_mw(), 1);
        debug_print(" W\n");
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // selects the voltage sense source
    else if (SHELL_CMD("vsensrc")) {

        if (argc == 1) {

            debug_print("voltage sense source is ");
            debug_print((load_get_vsensrc()) ? "remote" : "internal");
            debug_print(".\n");

        } else {

            if (COMPARE_ARG(1, "internal")) {

                load_set_auto_vsensrc(false);
                load_set_remote_sense(false);
                debug_print("voltage sense source set to internal\n");
        
            } else if (COMPARE_ARG(1, "remote")) {

                load_set_auto_vsensrc(false);
                load_set_remote_sense(true);
                debug_print("voltage sense source set to remote\n");

            } else if (COMPARE_ARG(1, "auto")) {

                load_set_auto_vsensrc(true);
                debug_print("voltage sense source set to auto\n");

            } else debug_print("(!) invalid argument. Use \"internal\", \"remote\" or \"auto\".\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // sets the automatic disable voltage level
    else if (SHELL_CMD("vdis")) {

        shell_assert_argc(1);
        int voltage = atoi(args[1]);

        if (voltage >= LOAD_MIN_CV_LEVEL_MV && voltage <= LOAD_MAX_CV_LEVEL_MV) {

            load_set_discharge_voltage((uint32_t)voltage);

            if (voltage > 0) {

                debug_print("discharge voltage set to: ");
                debug_print_int(voltage);
                debug_print(" mV.\n");

            } else debug_print("loat auto-disable feature disabled.\n");    

        } else {

            debug_print("(!) vdis range is <");
            debug_print_int(LOAD_MIN_CV_LEVEL_MV);
            debug_print(" - ");
            debug_print_int(LOAD_MAX_CV_LEVEL_MV);
            debug_print(">.\n");
        }
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // returns the load temperature
    else if (SHELL_CMD("temp")) {

        if (load_get_fault_flags() & (LOAD_FAULT_TEMP_L | LOAD_FAULT_TEMP_R)) {

            debug_print("Temperature sensor fault.\n");

        } else {

            debug_print("Load temperature: ");
            debug_print_int(load_get_temp());
            debug_print("'C.\n");
        } 
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // prints available commands
    else if (SHELL_CMD("help")) {

        debug_print("\nlist of available commands:\n");
        debug_print("repeat <period_ms> <command> - periodically repeat a command\n");
        debug_print("reboot - restart the system\n");
        debug_print("status - print the load status\n");
        debug_print("clearfaults - clear all load fault flags\n");
        debug_print("faultmask <0000 - ffff> - set a fault mask\n");
        debug_print("en <1 or 0> - enables or disables the load\n");
        debug_print("iset <current_ma> - set the load CC level\n");
        debug_print("vset <voltage_mv> - set the load CV level\n");
        debug_print("rset <resistance_mr> - set the load CR level\n");
        debug_print("pset <power_mw> - set the load CP level\n");
        debug_print("vsen - read the load input voltage\n");
        kernel_sleep_ms(50);
        debug_print("isen - read the total load current\n");
        debug_print("psen - read the load power\n");
        debug_print("vsensrc <internal or remote> - set the voltage sense source\n");
        debug_print("vdis <voltage_mv> - disable the load automatically when the source voltage drops bellow a threshold\n");
        debug_print("temp - read the power transistor temperatures\n");
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
