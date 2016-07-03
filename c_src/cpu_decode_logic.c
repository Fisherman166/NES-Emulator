//*****************************************************************************
// Filename: cpu_decode_logic.c
// Author: Fisherman166
//
// Implements all address decoding functions of the CPU
//
// ****************************************************************************

#include "cpu_decode_logic.h"

static bool is_page_crossed(uint16_t original_address, uint16_t branch_address) {
    const uint16_t page_mask = 0xFF00;
    if((original_address & page_mask) == (branch_address & page_mask)) return false;
    else return true;
}

uint8_t fetch_opcode(cpu_registers* registers) {
    return fetch_immediate(registers);
}

uint8_t fetch_immediate(cpu_registers* registers) {
    return read_RAM(registers->PC);
}

//*****************************************************************************
// Address calculation functions
//*****************************************************************************
uint8_t calc_zeropage_address(cpu_registers* registers) {
    return fetch_immediate(registers);
}


uint8_t calc_zeropageX_address(cpu_registers* registers) {
    return (fetch_immediate(registers) + registers->X) & BYTE_MASK;
}


uint8_t calc_zeropageY_address(cpu_registers* registers) {
    return (fetch_immediate(registers) + registers->Y) & BYTE_MASK;
}


uint16_t calc_absolute_address(cpu_registers* registers) {
    uint8_t low_byte = read_RAM(registers->PC);
    uint8_t high_byte = read_RAM(registers->PC + 1);
    uint16_t absolute_address = (high_byte << 8) | low_byte;
    return absolute_address;
}


uint16_t calc_absoluteX_address(cpu_registers* registers) {
    uint16_t absolute_address = calc_absolute_address(registers);
    return (absolute_address + registers->X);
}


uint16_t calc_absoluteY_address(cpu_registers* registers) {
    uint16_t absolute_address = calc_absolute_address(registers);
    return (absolute_address + registers->Y);
}


uint16_t calc_indirectX_address(cpu_registers* registers) {
    uint8_t low_byte_address = calc_zeropageX_address(registers);
    uint8_t high_byte_address = (low_byte_address + 1) & BYTE_MASK;

    uint8_t low_byte = read_RAM(low_byte_address);
    uint8_t high_byte = read_RAM(high_byte_address);
    uint16_t indirectX_address = (high_byte << 8) | low_byte;
    return indirectX_address;
}

uint16_t calc_indirectY_address(cpu_registers* registers) {
    uint8_t zeropage_address = fetch_immediate(registers);
    uint8_t low_byte_address = read_RAM(zeropage_address);
    uint8_t high_byte_address = read_RAM((zeropage_address + 1) & BYTE_MASK);
    uint8_t low_byte = read_RAM(low_byte_address);
    uint8_t high_byte = read_RAM(high_byte_address);
    uint16_t indirectY_address = ((high_byte << 8) | low_byte) + registers->Y;
    return indirectY_address;
}


//*****************************************************************************
// Zeropage decode functions
//*****************************************************************************
uint8_t fetch_zeropage(cpu_registers* registers) {
    uint8_t zeropage_address = calc_zeropage_address(registers);
    return read_RAM(zeropage_address);
}


uint8_t fetch_zeropageX(cpu_registers* registers) {
    uint8_t zeropage_address = calc_zeropageX_address(registers);
    return read_RAM(zeropage_address);
}


uint8_t fetch_zeropageY(cpu_registers* registers) {
    uint8_t zeropage_address = calc_zeropageY_address(registers);
    return read_RAM(zeropage_address);
}


//*****************************************************************************
// Absolute decode functions
//*****************************************************************************
uint8_t fetch_absolute(cpu_registers* registers) {
    uint16_t data_address = calc_absolute_address(registers);
    return read_RAM(data_address);
}


uint8_t fetch_absoluteX(cpu_registers* registers, bool* page_crossed) {
    uint16_t absoluteX_address = calc_absoluteX_address(registers);
    if(page_crossed) *page_crossed = is_page_crossed(absoluteX_address - registers->X,
                                                     absoluteX_address);
    return read_RAM(absoluteX_address);
}


uint8_t fetch_absoluteY(cpu_registers* registers, bool* page_crossed) {
    uint16_t absoluteY_address = calc_absoluteY_address(registers);
    if(page_crossed) *page_crossed = is_page_crossed(absoluteY_address - registers->Y,
                                                     absoluteY_address);
    return read_RAM(absoluteY_address);
}

//*****************************************************************************
// Indirect decode functions
//*****************************************************************************
uint8_t fetch_indirectX(cpu_registers* registers) {
    uint16_t indirectX_address = calc_indirectX_address(registers);
    return read_RAM(indirectX_address);
}

uint8_t fetch_indirectY(cpu_registers* registers, bool* page_crossed) {
    uint16_t indirectY_address = calc_indirectY_address(registers);
    if(page_crossed) *page_crossed = is_page_crossed(indirectY_address - registers->Y,
                                                     indirectY_address);
    return read_RAM(indirectY_address);
}


bool branch_relative(cpu_registers* registers) {
    int8_t offset = read_RAM(registers->PC);
    bool page_crossed = is_page_crossed(registers->PC, registers->PC + offset);
    registers->PC += offset;
    return page_crossed;
}

