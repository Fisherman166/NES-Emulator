//*****************************************************************************
// Filename: main.c
// Author: Fisherman166
//
// Main functions that glues everything together
//
// ****************************************************************************

#include <unistd.h>
#include <stdlib.h>
#include "common.h"
#include "common_cpu.h"
#include "cpu.h"
#include "cpu_basic_operations_unit_tests.h"
#include "cpu_decode_logic_unit_tests.h"
#include "cpu_unit_tests.h"
#include "memory_operations.h"
#include "sdl_interface.h"

bool parse_cmdline(int, char **);

int main(int argc, char *argv[]) {
    bool run_unit_tests = parse_cmdline(argc, argv);
    if(run_unit_tests) {
        run_all_basic_cpu_operations_tests();
        run_all_cpu_decode_logic_tests();
        run_all_cpu_tests();
        exit(EXIT_SUCCESS);
    }

    #ifdef DEBUG
        open_cpu_debug_logfile();
    #endif

    if( init_SDL() ) return 1;
    bool game_loaded = load_game();
    printf("LOADED GAME\n");
    if(!game_loaded) {
        printf("FATAL ERROR occurred while attempting to load game.\n");
        return -1;
    }
    cpu_registers registers;
    cold_boot_init(&registers);

    for(;;) {
        execute_interpreter_cycle(&registers);
        if( check_input(JOYPAD1) ) break;
    }
    printf("Ending Emulation!\n");
    exit_SDL();

    #ifdef DEBUG
        close_cpu_debug_logfile();
    #endif

    return 0;
}

bool parse_cmdline(int argc, char **argv) {
    bool run_unit_tests = false;
    int option;

    while( (option = getopt(argc, argv, "u")) != -1 ) {
        switch(option) {
            case 'u': run_unit_tests = true; break;
            default: run_unit_tests = false; break;
        }
    }
    return run_unit_tests;
} 
