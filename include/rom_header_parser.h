//*****************************************************************************
// Filename: rom_header_parser.h
// Author: Fisherman166
//
// Parses the various iNes header formats
//
//*****************************************************************************

#ifndef ROM_HEADER_PARSER_H
#define ROM_HEADER_PARSER_H

#include "common.h"

typedef struct {
    bool header_found;
    uint8_t PRG_ROM_chunks; // 16KBs per chunk
    uint8_t CHR_ROM_chunks; // 8KBs per chunk
    union {
        struct {
            uint8_t mirroring           : 1;
            uint8_t has_PRG_RAM         : 1;
            uint8_t has_trainer         : 1;
            uint8_t ignore_mirror       : 1;
            uint8_t low_mapper_nibble   : 4;
        } fields;
        uint8_t data;
    } flags6;
    union {
        struct {
            uint8_t VS_Unisystem        : 1;
            uint8_t PlayChoice_10       : 1;
            uint8_t is_NES_2_0          : 2;
            uint8_t high_mapper_nibble  : 4;
        } fields;
        uint8_t data;
    } flags7;
    uint8_t CHR_RAM_chunks; // 8KBs per chunk
    bool is_PAL;
} iNES_1_0_header;

iNES_1_0_header parse_iNes_1_0_header(uint8_t*);
void print_iNes_1_0_header(iNES_1_0_header);

#endif // ROM_HEADER_PARSER_H
