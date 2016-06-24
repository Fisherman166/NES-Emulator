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

