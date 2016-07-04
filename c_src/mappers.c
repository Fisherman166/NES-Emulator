//*****************************************************************************
// Filename: mappers.c
// Author: Fisherman166
//
// Implements the different mappers used in the NES
//
//*****************************************************************************

#include "memory_operations.h"
#include "mappers.h"

void load_NROM(uint8_t* game_data) {
    uint8_t CHR_rom_size = game_data[4];
    uint32_t NES_address = 0x8000;

    if(CHR_rom_size > 1) {
        for(uint32_t game_address = 0x10; game_address < 0x8010; game_address++) {
            write_RAM(NES_address, game_data[game_address]);
            NES_address++;
        }
    }
    else { // First bank mirrored in second
        for(uint32_t game_address = 0x10; game_address < 0x4010; game_address++) {
            write_RAM(NES_address, game_data[game_address]);
            write_RAM(NES_address + 0x4000, game_data[game_address]);
            NES_address++;
        }
    }
}

