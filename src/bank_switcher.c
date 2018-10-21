//*****************************************************************************
// Filename: bank_switcher.c
// Author: Fisherman166
//
// Functions for switching PRG and CHR banks
//
//*****************************************************************************

#include "bank_switcher.h"
#include "RAM.h"
#include "VRAM.h"

void write_lower_16KB_PRG_bank(uint8_t* bank_data) {
    uint16_t bank_address = 0;
    for(uint16_t NES_address = PRG_ROM_BANK1_ADDRESS; NES_address < PRG_ROM_BANK2_ADDRESS; NES_address++) {
        write_RAM(NES_address, bank_data[bank_address++]);
    }
}

void write_upper_16KB_PRG_bank(uint8_t* bank_data) {
    uint16_t bank_address = 0;
    for(uint32_t NES_address = PRG_ROM_BANK2_ADDRESS; NES_address < PRG_ROM_BANK2_ADDRESS + PRG_BANK_SIZE; NES_address++) {
        write_RAM((uint16_t)NES_address, bank_data[bank_address++]);
    }
}

void write_32KB_PRG_bank(uint8_t* lower_bank_data, uint8_t* upper_bank_data) {
    write_lower_16KB_PRG_bank(lower_bank_data);
    write_upper_16KB_PRG_bank(upper_bank_data);
}


void write_lower_4KB_CHR_bank(uint8_t* bank_data) {
    uint16_t bank_address = 0;
    for(uint16_t NES_address = CHR_ROM_BANK1_ADDRESS; NES_address < CHR_ROM_BANK2_ADDRESS; NES_address++) {
        write_VRAM(NES_address, bank_data[bank_address++]);
    }
}

void write_upper_4KB_CHR_bank(uint8_t* bank_data) {
    // CHR banks are stored as all 8KB. So to get the top half, start halfway in
    uint16_t bank_address = CHR_BANK_SIZE / 2;
    const uint16_t end_address = CHR_ROM_BANK2_ADDRESS + (CHR_BANK_SIZE / 2);
    for(uint16_t NES_address = CHR_ROM_BANK2_ADDRESS; NES_address < end_address; NES_address++) {
        write_VRAM(NES_address, bank_data[bank_address++]);
    }
}

void write_8KB_CHR_bank(uint8_t* bank_data) {
    write_lower_4KB_CHR_bank(bank_data);
    write_upper_4KB_CHR_bank(bank_data);
}
