//*****************************************************************************
// Filename: mapper_0.c
// Author: Fisherman166
//
// Implements NROM mapper
//
//*****************************************************************************

#include "mapper_0.h"
#include "bank_switcher.h"
#include "rom_parser.h"
#include "VRAM.h"

void init_NROM() {
    iNES_1_0_header parsed_header = get_iNes_1_0_header();

    if(parsed_header.flags6.fields.mirroring) set_vertical_mirroring();
    else set_horizontal_mirroring();

    if(parsed_header.PRG_ROM_chunks == 2) {
        write_32KB_PRG_bank(get_PRG_bank(0), get_PRG_bank(1));
    }
    else {
        // Data is mirrored
        write_32KB_PRG_bank(get_PRG_bank(0), get_PRG_bank(0));
    }

    write_8KB_CHR_bank(get_CHR_bank(0));
}

