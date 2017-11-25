//*****************************************************************************
// Filename: mappers.c
// Author: Fisherman166
//
// Implements the different mappers used in the NES
//
//*****************************************************************************

#include "RAM.h"
#include "VRAM.h"
#include "mappers.h"

#define CHR_BANK_SIZE 0x4000
#define GAME_DATA_START 0x10
#define NES_CHR_BANK_BASE 0x8000

static bool is_dual_bank_NROM(uint8_t CHR_rom_size) {
    if(CHR_rom_size > 1) return true;
    else return false;
}

static uint16_t load_dual_bank(uint8_t* game_data) {
    const uint16_t dual_bank_end = (CHR_BANK_SIZE * 2) + GAME_DATA_START;
    uint16_t rom_address = NES_CHR_BANK_BASE;

    for(uint32_t game_address = GAME_DATA_START; game_address < dual_bank_end; game_address++) {
        write_RAM(rom_address, game_data[game_address]);
        rom_address++;
    }
    return dual_bank_end;
}

static uint16_t load_single_bank(uint8_t* game_data) {
    const uint16_t single_bank_end = CHR_BANK_SIZE + GAME_DATA_START;
    uint16_t rom_address = NES_CHR_BANK_BASE;

    for(uint32_t game_address = GAME_DATA_START; game_address < single_bank_end; game_address++) {
        write_RAM(rom_address, game_data[game_address]);
        write_RAM(rom_address + CHR_BANK_SIZE, game_data[game_address]);
        rom_address++;
    }
    return single_bank_end;
}

static void load_pattern_table(
    uint8_t* game_data,
    uint16_t CHR_BANK_end,
    long game_size
) {
    uint16_t VRAM_address = 0;
    for(uint32_t game_address = CHR_BANK_end; game_address < game_size; game_address++) {
        write_VRAM(VRAM_address, game_data[game_address]);
        VRAM_address++;
    }
}

void load_NROM(uint8_t* game_data, long game_size) {
    uint16_t CHR_BANK_end = 0;
    uint8_t CHR_rom_size = game_data[4];

    if(game_data[6] & 1) set_vertical_mirroring();
    else set_horizontal_mirroring();

    if(is_dual_bank_NROM(CHR_rom_size))
        CHR_BANK_end = load_dual_bank(game_data);
    else
        CHR_BANK_end = load_single_bank(game_data);
    
    load_pattern_table(game_data, CHR_BANK_end, game_size);
}

