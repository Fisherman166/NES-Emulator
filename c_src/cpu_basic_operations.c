//*****************************************************************************
// Filename: cpu_basic_operations.c
// Author: Fisherman166
//
// Implements the basic CPU operations. Things like adds, subtracts, setting
// flags, pushing data onto the stack, etc
//
//*****************************************************************************

#include "cpu_basic_operations.h"

#define BYTE_MASK 0xFF

//*****************************************************************************
// General functions
//*****************************************************************************
void init_cpu_registers(cpu_registers* registers, uint8_t A, uint8_t X,
                        uint8_t Y, uint16_t PC, uint8_t S, uint8_t flags)
{
    registers->A = A;
    registers->X = X;
    registers->Y = Y;
    registers->PC = PC;
    registers->S = S;
    registers->flags = flags;
}

//*****************************************************************************
// Status flag functions
//*****************************************************************************
void set_cpu_flag(cpu_registers* registers, uint8_t flag_to_set) {
    registers->flags |= flag_to_set;
}

void clear_cpu_flag(cpu_registers* registers, uint8_t flag_to_clear) {
    registers->flags &= ~flag_to_clear;
}

bool get_cpu_flag(cpu_registers* registers, uint8_t flag_to_get) {
    return registers->flags & flag_to_get;
}

void check_value_for_zero_flag(cpu_registers* registers, uint8_t value) {
    if(!value) set_cpu_flag(registers, ZERO_FLAG);
    else clear_cpu_flag(registers, ZERO_FLAG);
}

void check_value_for_negative_flag(cpu_registers* registers, uint8_t value) {
    const uint8_t negative_bit_position = 0x80;
    if(value & negative_bit_position) set_cpu_flag(registers, NEGATIVE_FLAG);
    else clear_cpu_flag(registers, NEGATIVE_FLAG);
}

//*****************************************************************************
// Basic opcode operations
//*****************************************************************************
void base_add(cpu_registers* registers, uint8_t operand) {
    const uint16_t result_carry_bit_position = 0x100;
    bool carry_bit = get_cpu_flag(registers, CARRY_FLAG);
    uint16_t result = registers->A + operand + (uint8_t)carry_bit;

    if( (registers->A ^ result) & (operand ^ result) & 0x80 ) set_cpu_flag(registers, OVERFLOW_FLAG);
    else clear_cpu_flag(registers, OVERFLOW_FLAG);
    if(result & result_carry_bit_position) set_cpu_flag(registers, CARRY_FLAG);
    else clear_cpu_flag(registers, CARRY_FLAG);

    registers->A = result & BYTE_MASK;
    check_value_for_zero_flag(registers, registers->A);
    check_value_for_negative_flag(registers, registers->A);
}
    
