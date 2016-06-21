//*****************************************************************************
// Filename: memory_operations.c
// Author: Fisherman166
//
// Implements the memory operations for the CPU and PPU
//
//*****************************************************************************

#include "memory_operations.h"

#define RAM_locations 0x10000

uint8_t RAM[RAM_locations];

void init_RAM() {
    for(int i = 0; i < 0x10000; i++) {
        RAM[i] = 0x00;
    }
}

uint8_t read_RAM(uint16_t address_to_read) {
    return RAM[address_to_read];
}

void write_RAM(uint16_t address_to_write, uint8_t value_to_write) {
   RAM[address_to_write] = value_to_write;
}

