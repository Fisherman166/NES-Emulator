//*****************************************************************************
// Filename: main.c
// Author: Fisherman166
//
// Main functions that glues everything together
//
// ****************************************************************************

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "memory_operations.h"
#include "sdl_interface.h"
#include "ppu.h"

bool parse_cmdline(int, char **);

int main(int argc, char *argv[]) {
    #ifdef DEBUG
        open_cpu_debug_logfile();
    #endif

    if( init_SDL() ) return 1;
    bool game_loaded = load_game();
    if(game_loaded) {
        printf("FATAL ERROR occurred while attempting to load game.\n");
        return -1;
    }
    cpu_registers registers;
    cold_boot_init(&registers);

    printf("Entering main loop\n");
    uint8_t cycles_executed;
    bool nmi_flag = false;
    uint32_t* pixel_data = get_pixel_data_ptr();
    bool vblank;
    bool old_vblank = false;

    for(;;) {
        nmi_flag = get_NMI_flag();
        cycles_executed = execute_interpreter_cycle(&registers, nmi_flag);
        for(uint8_t ppu_cycles = cycles_executed * 3; ppu_cycles > 0; ppu_cycles--)
            vblank = run_PPU_cycle();
        if( check_input(JOYPAD1) ) break;

        if( vblank & !old_vblank ) render_frame(pixel_data);
        old_vblank = vblank;
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
