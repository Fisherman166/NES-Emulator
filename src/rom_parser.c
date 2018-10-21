//*****************************************************************************
// Filename: rom_parser.c
// Author: Fisherman166
//
// Parses rom into various data structures
//
//*****************************************************************************

#include <stdlib.h>
#include "rom_parser.h"

#define GAME_DATA_START 0x10 // End of iNes Header in rom
#define PRG_BANK_SIZE 0x4000
#define CHR_BANK_SIZE 0x2000

static iNES_1_0_header parsed_header;
static uint8_t** PRG_rom_banks = NULL;
static uint8_t** CHR_rom_banks = NULL;

void parse_iNes_1_0_header(uint8_t* rom_data) {
    const uint32_t iNES_header_constant = 0x1A53454E;

    parsed_header.header_found      = *((uint32_t*)rom_data) == iNES_header_constant;
    parsed_header.PRG_ROM_chunks    = rom_data[4];
    parsed_header.CHR_ROM_chunks    = rom_data[5];
    parsed_header.flags6.data       = rom_data[6];
    parsed_header.flags7.data       = rom_data[7];
    parsed_header.CHR_RAM_chunks    = rom_data[8];
    parsed_header.is_PAL            = rom_data[9] & 0x1;
    parsed_header.mapper            = (parsed_header.flags7.fields.high_mapper_nibble << 4) | parsed_header.flags6.fields.low_mapper_nibble;
}

void print_iNes_1_0_header(iNES_1_0_header header) {
    printf("\nPrinting iNes 1.0 header for rom\n");
    printf("    header_found              = %X\n", header.header_found);
    printf("    PRG_ROM_chunks            = %X\n", header.PRG_ROM_chunks);
    printf("    CHR_ROM_chunks            = %X\n", header.CHR_ROM_chunks);
    printf("    flags6.mirroring          = %X\n", header.flags6.fields.mirroring);
    printf("    flags6.has_PRG_RAM        = %X\n", header.flags6.fields.has_PRG_RAM);
    printf("    flags6.has_trainer        = %X\n", header.flags6.fields.has_trainer);
    printf("    flags6.ignore_mirror      = %X\n", header.flags6.fields.ignore_mirror);
    printf("    flags6.low_mapper_nibble  = %X\n", header.flags6.fields.low_mapper_nibble);
    printf("    flags7.VS_Unisystem       = %X\n", header.flags7.fields.VS_Unisystem);
    printf("    flags7.PlayChoice_10      = %X\n", header.flags7.fields.PlayChoice_10);
    printf("    flags7.is_NES_2_0         = %X\n", header.flags7.fields.is_NES_2_0);
    printf("    flags7.high_mapper_nibble = %X\n", header.flags7.fields.high_mapper_nibble);
    printf("    CHR_RAM_chunks            = %X\n", header.CHR_RAM_chunks);
    printf("    is_PAL                    = %X\n", header.is_PAL);
    printf("    mapper                    = %X\n", header.mapper);
    printf("\n");
}

iNES_1_0_header get_iNes_1_0_header() {
    return parsed_header;
}

// Return true if error
bool parse_PRG_rom(uint8_t* rom_data) {
    PRG_rom_banks = malloc(parsed_header.PRG_ROM_chunks * sizeof(uint8_t*));
    if(PRG_rom_banks == NULL) {
        printf("ERROR: Failed to malloc pointers for PRG_ROM banks\n");
        return true;
    }

    for(uint8_t bank_num = 0; bank_num < parsed_header.PRG_ROM_chunks; bank_num++) {
        PRG_rom_banks[bank_num] = malloc(PRG_BANK_SIZE * sizeof(uint8_t));
        if(PRG_rom_banks[bank_num] == NULL) {
            printf("ERROR: Failed to malloc space for PRG_ROM bank\n");
            return true;
        }
    }

    uint32_t bank_starting_rom_address;
    uint32_t bank_ending_rom_address;
    uint8_t* bank_to_fill;
    uint16_t bank_address;
    for(uint8_t bank_num = 0; bank_num < parsed_header.PRG_ROM_chunks; bank_num++) {
        bank_starting_rom_address = GAME_DATA_START + (PRG_BANK_SIZE * bank_num);
        bank_ending_rom_address   = GAME_DATA_START + (PRG_BANK_SIZE * (bank_num + 1));
        bank_to_fill              = PRG_rom_banks[bank_num];
        bank_address              = 0;

        for(uint32_t rom_address = bank_starting_rom_address; rom_address < bank_ending_rom_address; rom_address++) {
            bank_to_fill[bank_address++] = rom_data[rom_address];
        }
    }

    return false;
}

bool parse_CHR_rom(uint8_t* rom_data) {
    CHR_rom_banks = malloc(parsed_header.CHR_ROM_chunks * sizeof(uint8_t*));
    if(CHR_rom_banks == NULL) {
        printf("ERROR: Failed to malloc pointers for CHR_ROM banks\n");
        return true;
    }

    for(uint8_t bank_num = 0; bank_num < parsed_header.CHR_ROM_chunks; bank_num++) {
        CHR_rom_banks[bank_num] = malloc(CHR_BANK_SIZE * sizeof(uint8_t));
        if(CHR_rom_banks[bank_num] == NULL) {
            printf("ERROR: Failed to malloc space for CHR_ROM bank\n");
            return true;
        }
    }

    uint32_t bank_starting_rom_address;
    uint32_t bank_ending_rom_address;
    uint8_t* bank_to_fill;
    uint16_t bank_address;
    for(uint8_t bank_num = 0; bank_num < parsed_header.CHR_ROM_chunks; bank_num++) {
        bank_starting_rom_address = GAME_DATA_START + (PRG_BANK_SIZE * parsed_header.PRG_ROM_chunks) + (CHR_BANK_SIZE * bank_num);
        bank_ending_rom_address   = GAME_DATA_START + (PRG_BANK_SIZE * parsed_header.PRG_ROM_chunks) + (CHR_BANK_SIZE * (bank_num + 1));
        bank_to_fill              = CHR_rom_banks[bank_num];
        bank_address              = 0;

        for(uint32_t rom_address = bank_starting_rom_address; rom_address < bank_ending_rom_address; rom_address++) {
            bank_to_fill[bank_address++] = rom_data[rom_address];
        }
    }

    return false;
}

uint8_t* get_PRG_bank(uint8_t bank_number) {
    // Account for the -1 in array indexing
    if(bank_number >= parsed_header.PRG_ROM_chunks) {
        printf("ERROR: attempted to access PRG rom bank number %u when the max bank number is %u\n",
               bank_number, parsed_header.PRG_ROM_chunks - 1);
        return NULL;
    }
    return PRG_rom_banks[bank_number];
}

uint8_t* get_CHR_bank(uint8_t bank_number) {
    // Account for the -1 in array indexing
    if(bank_number >= parsed_header.CHR_ROM_chunks) {
        printf("ERROR: attempted to access CHR rom bank number %u when the max bank number is %u\n",
               bank_number, parsed_header.CHR_ROM_chunks - 1);
        return NULL;
    }
    return CHR_rom_banks[bank_number];
}

void cleanup_PRG_and_CHR_banks() {
    for(uint8_t bank_num = 0; bank_num < parsed_header.PRG_ROM_chunks; bank_num++) {
        free(PRG_rom_banks[bank_num]);
    }

    for(uint8_t bank_num = 0; bank_num < parsed_header.CHR_ROM_chunks; bank_num++) {
        free(CHR_rom_banks[bank_num]);
    }

    free(PRG_rom_banks);
    free(CHR_rom_banks);
}
