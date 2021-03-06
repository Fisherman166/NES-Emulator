#ifndef VRAM_H
#define VRAM_H

#include "common.h"

void init_VRAM();

void set_horizontal_mirroring();
void set_vertical_mirroring();
void set_single_screen_low_mirroring();
void set_single_screen_high_mirroring();
void set_four_screen_mirroring();

uint8_t read_VRAM(uint16_t);
void write_VRAM(uint16_t, uint8_t);

// Functions called from RAM
void clear_write_toggle();
uint8_t PPUDATA_VRAM_read();
void increment_ppu_address(uint8_t);
void PPUCTRL_update_temp_VRAM_address(uint8_t);
void PPUSCROLL_update_temp_VRAM_address(uint8_t);
void PPUADDR_update_temp_VRAM_address(uint8_t);
void PPUDATA_update_temp_VRAM_address(uint8_t, uint8_t);

// Functions called from ppu
void incrementY();
void incrementX();
void copyX();
void copyY();
uint8_t get_fineX_scroll();
uint16_t get_VRAM_address();
uint16_t get_temp_VRAM_address();
bool get_write_toggle();
void debug_dump_VRAM();

#endif

