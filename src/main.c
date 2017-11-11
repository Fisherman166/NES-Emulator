//*****************************************************************************
// Filename: main.c
// Author: Fisherman166
//
// Main functions that glues everything together
//
// ****************************************************************************

#include <unistd.h>
#include <stdio.h>
#include "cpu.h"
#include "sdl_interface.h"
#include "ppu.h"
#include "game_loader.h"
#include "RAM.h"
#include "VRAM.h"

static void parse_cmdline(int, char **);
static void init_system(cpu_registers*);
static void run_system(cpu_registers*);
static void cleanup();

static char* game_file;

int main(int argc, char *argv[]) {
    cpu_registers registers;

    parse_cmdline(argc, argv);
    init_system(&registers);
    run_system(&registers);
    cleanup();

    return 0;
}

static void parse_cmdline(int argc, char **argv) {
    if(argc < 2) {
        printf("ERROR: No game path given. First argument should be game file path\n");
        exit(1);
    }
    game_file = argv[1];
} 

static void init_system(cpu_registers* registers) {
    if( init_SDL() ) {
        printf("ERROR: Failed to init SDL");
        exit(1);
    }
    if( load_game(game_file) ) {
        printf("FATAL ERROR occurred while attempting to load game.\n");
        exit(1);
    }
    cold_boot_init(registers);
#ifdef DEBUG
    open_cpu_debug_logfile();
#endif
}

static void run_system(cpu_registers* registers) {
    uint8_t cycles_executed;
    bool vblank;
    bool old_vblank = false;
    uint32_t* pixel_data = get_pixel_data_ptr();
    init_RAM();
    init_VRAM();

    printf("Entering main loop\n");
    for(;;) {
        cycles_executed = execute_interpreter_cycle(registers, get_NMI_flag());
        for(uint8_t ppu_cycles = cycles_executed * 3; ppu_cycles > 0; ppu_cycles--) {
            vblank = run_PPU_cycle();
            if( vblank & !old_vblank ) render_frame(pixel_data);
            old_vblank = vblank;
        }
        if( check_input(JOYPAD1) ) break;
    }
}

static void cleanup() {
    printf("Ending Emulation!\n");
    exit_SDL();

    #ifdef DEBUG
        close_cpu_debug_logfile();
    #endif
}

