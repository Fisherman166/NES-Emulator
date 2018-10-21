//*****************************************************************************
// Filename: bank_switcher.h
// Author: Fisherman166
//
// Functions for switching PRG and CHR banks
//
//*****************************************************************************

#ifndef BANK_SWITCHER_H
#define BANK_SWITCHER_H

#include "common.h"

void write_lower_16KB_PRG_bank(uint8_t*);
void write_upper_16KB_PRG_bank(uint8_t*);
void write_32KB_PRG_bank(uint8_t*, uint8_t*);

void write_lower_4KB_CHR_bank(uint8_t*);
void write_upper_4KB_CHR_bank(uint8_t*);
void write_8KB_CHR_bank(uint8_t*);

#endif // BANK_SWITCHER_H
