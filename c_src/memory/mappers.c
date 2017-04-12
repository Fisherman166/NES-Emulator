//*****************************************************************************
// Filename: mappers.c
// Author: Fisherman166
//
// Implements the different mappers used in the NES
//
//*****************************************************************************

#include "memory_operations.h"
#include "VRAM.h"
#include "mappers.h"

#define CHR_BANK_SIZE 0x4000

static bool is_dual_bank_NROM(uint8_t CHR_rom_size) {
    if(CHR_rom_size > 1) return true;
    else return false;
}

void load_NROM(uint8_t* game_data) {
    const uint16_t game_data_start = 0x10;
    const uint16_t single_bank_end = CHR_BANK_SIZE + game_data_start;
    const uint16_t dual_bank_end = (CHR_BANK_SIZE * 2) + game_data_start;

    uint8_t CHR_rom_size = game_data[4];
    uint16_t NES_rom_address = 0x8000;

    if(game_data[6] & 1) set_horizontal_mirroring();
    else set_vertical_mirroring();

    // Fill both rom banks without mirroring
    if(is_dual_bank_NROM(CHR_rom_size)) {
        for(uint32_t game_address = game_data_start; game_address < dual_bank_end; game_address++) {
            write_RAM(NES_rom_address, game_data[game_address]);
            NES_rom_address++;
        }
    }
    else { // First bank mirrored in second bank
        for(uint32_t game_address = game_data_start; game_address < single_bank_end; game_address++) {
            write_RAM(NES_rom_address, game_data[game_address]);
            write_RAM(NES_rom_address + CHR_BANK_SIZE, game_data[game_address]);
            NES_rom_address++;
        }
    }
}

