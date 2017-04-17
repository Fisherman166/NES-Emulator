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

void parse_cmdline(int, char **);

static char* game_file;

int main(int argc, char *argv[]) {
    parse_cmdline(argc, argv);

    #ifdef DEBUG
        open_cpu_debug_logfile();
    #endif

    if( init_SDL() ) return 1;
    bool game_loaded = load_game(game_file);
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

void parse_cmdline(int argc, char **argv) {
    if(argc < 2) {
        printf("ERROR: No game path given. First argument should be game file path\n");
        exit(1);
    }
    game_file = argv[1];
} 
