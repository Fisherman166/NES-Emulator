//*****************************************************************************
// Filename: rom_header_parser.c
// Author: Fisherman166
//
// Parses the various iNes header formats
//
//*****************************************************************************

#include "rom_header_parser.h"

iNES_1_0_header parse_iNes_1_0_header(uint8_t* rom_data) {
    const uint32_t iNES_header_constant = 0x1A53454E;
    iNES_1_0_header parsed_header;

    parsed_header.header_found      = *((uint32_t*)rom_data) == iNES_header_constant;
    parsed_header.PRG_ROM_chunks    = rom_data[4];
    parsed_header.CHR_ROM_chunks    = rom_data[5];
    parsed_header.flags6.data       = rom_data[6];
    parsed_header.flags7.data       = rom_data[7];
    parsed_header.CHR_RAM_chunks    = rom_data[8];
    parsed_header.is_PAL            = rom_data[9] & 0x1;

    return parsed_header;
}

void print_iNes_1_0_header(iNES_1_0_header header) {
    printf("\n Printing iNes 1.0 header for rom\n");
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
    printf("\n");
}
