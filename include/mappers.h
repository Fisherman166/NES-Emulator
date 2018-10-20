//*****************************************************************************
// Filename: mappers.h
// Author: Fisherman166
//
// Implements the different mappers used in the NES
//
//*****************************************************************************

#ifndef MAPPERS_H
#define MAPPERS_H

#include "common.h"
#include "rom_header_parser.h"

void load_NROM(uint8_t*, iNES_1_0_header);

#endif

