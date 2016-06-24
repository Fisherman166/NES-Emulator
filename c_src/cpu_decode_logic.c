//*****************************************************************************
// Filename: cpu_decode_logic.c
// Author: Fisherman166
//
// Implements all address decoding functions of the CPU
//
// ****************************************************************************

#include "cpu_decode_logic.h"

uint8_t fetch_immediate(cpu_registers* registers) {
    return read_RAM(registers->PC);
}


//*****************************************************************************
// Zeropage decode functions
//*****************************************************************************
uint8_t fetch_zeropage(cpu_registers* registers) {
    uint8_t zeropage_address = fetch_immediate(registers);
    return read_RAM(zeropage_address);
}


uint8_t fetch_zeropageX(cpu_registers* registers) {
    uint8_t zeropage_address = (fetch_immediate(registers) + registers->X) & BYTE_MASK;
    return read_RAM(zeropage_address);
}


uint8_t fetch_zeropageY(cpu_registers* registers) {
    uint8_t zeropage_address = (fetch_immediate(registers) + registers->Y) & BYTE_MASK;
    return read_RAM(zeropage_address);
}


//*****************************************************************************
// Absolute decode functions
//*****************************************************************************
static uint16_t decode_absolute_base_address(cpu_registers* registers) {
    uint8_t low_byte = read_RAM(registers->PC);
    uint8_t high_byte = read_RAM(registers->PC + 1);
    uint16_t data_address = (high_byte << 8) | low_byte;
    return data_address;
}


uint8_t fetch_absolute(cpu_registers* registers) {
    uint16_t data_address = decode_absolute_base_address(registers);
    return read_RAM(data_address);
}


uint8_t fetch_absoluteX(cpu_registers* registers) {
    uint16_t data_address = decode_absolute_base_address(registers);
    data_address += registers->X;
    return read_RAM(data_address);
}

