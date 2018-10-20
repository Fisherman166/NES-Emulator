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

#define rom_data_START 0x10 // End of iNes Header in rom
#define PRG_BANK_SIZE 0x4000
#define CHR_BANK_SIZE 0x2000

static bool is_dual_bank_NROM(uint8_t PRG_rom_size) {
    return PRG_rom_size == 2;
}

static uint16_t load_dual_bank(uint8_t* rom_data) {
    const uint16_t end_of_PRG_section_in_rom = (PRG_BANK_SIZE * 2) + rom_data_START;
    uint16_t nes_PRG_rom_addr = PRG_ROM_BANK1_ADDRESS;

    for(uint32_t rom_address = rom_data_START; rom_address < end_of_PRG_section_in_rom; rom_address++) {
        write_RAM(nes_PRG_rom_addr, rom_data[rom_address]);
        nes_PRG_rom_addr++;
    }
    return end_of_PRG_section_in_rom;
}

static uint16_t load_single_bank(uint8_t* rom_data) {
    const uint16_t end_of_PRG_section_in_rom = PRG_BANK_SIZE + rom_data_START;
    uint16_t nes_PRG_rom_bank1_addr = PRG_ROM_BANK1_ADDRESS;
    uint16_t nes_PRG_rom_bank2_addr = PRG_ROM_BANK2_ADDRESS;

    for(uint32_t rom_address = rom_data_START; rom_address < end_of_PRG_section_in_rom; rom_address++) {
        write_RAM(nes_PRG_rom_bank1_addr, rom_data[rom_address]);
        write_RAM(nes_PRG_rom_bank2_addr, rom_data[rom_address]);
        nes_PRG_rom_bank1_addr++;
        nes_PRG_rom_bank2_addr++;
    }
    return end_of_PRG_section_in_rom;
}

static void load_pattern_table(
    uint8_t* rom_data,
    uint16_t end_of_PRG_section_in_rom
) {
    const uint32_t end_of_CHR_section_in_rom = end_of_PRG_section_in_rom + CHR_BANK_SIZE;
    uint16_t VRAM_address = 0;
    for(uint32_t rom_address = end_of_PRG_section_in_rom; rom_address < end_of_CHR_section_in_rom; rom_address++) {
        write_VRAM(VRAM_address, rom_data[rom_address]);
        VRAM_address++;
    }
}

void load_NROM(uint8_t* rom_data, iNES_1_0_header parsed_header) {
    uint16_t end_of_PRG_section_in_rom = 0;

    if(parsed_header.flags6.fields.mirroring) set_vertical_mirroring();
    else set_horizontal_mirroring();

    if(is_dual_bank_NROM(parsed_header.PRG_ROM_chunks)) {
        end_of_PRG_section_in_rom = load_dual_bank(rom_data);
    }
    else {
        end_of_PRG_section_in_rom = load_single_bank(rom_data);
    }
    
    load_pattern_table(rom_data, end_of_PRG_section_in_rom);
}

