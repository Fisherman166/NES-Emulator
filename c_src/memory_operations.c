//*****************************************************************************
// Filename: memory_operations.c
// Author: Fisherman166
//
// Implements the memory operations for the CPU and PPU
//
//*****************************************************************************

#include "memory_operations.h"
#include "cpu_decode_logic.h"

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

//*****************************************************************************
// Specific address mode writes
//*****************************************************************************
void write_zeropage(cpu_registers* registers, uint8_t data) {
    uint8_t zeropage_address = calc_zeropage_address(registers);
    write_RAM(zeropage_address, data);
}


void write_zeropageX(cpu_registers* registers, uint8_t data) {
    uint8_t zeropageX_address = calc_zeropageX_address(registers);
    write_RAM(zeropageX_address, data);
}


void write_absolute(cpu_registers* registers, uint8_t data) {
    uint16_t absolute_address = calc_absolute_address(registers);
    write_RAM(absolute_address, data);
}


void write_absoluteX(cpu_registers* registers, uint8_t data) {
    uint16_t absoluteX_address = calc_absoluteX_address(registers);
    write_RAM(absoluteX_address, data);
}

