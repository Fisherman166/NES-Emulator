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

#define BIT0_BITMASK 0x1
#define BIT1_BITMASK 0x2
#define BIT2_BITMASK 0x4
#define BIT3_BITMASK 0x8
#define BIT4_BITMASK 0x10
#define BIT5_BITMASK 0x20
#define BIT6_BITMASK 0x40
#define BIT7_BITMASK 0x80

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

void determine_zero_flag(cpu_registers* registers, uint8_t value) {
    if(!value) set_cpu_flag(registers, ZERO_FLAG);
    else clear_cpu_flag(registers, ZERO_FLAG);
}

void determine_negative_flag(cpu_registers* registers, uint8_t value) {
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
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
}

void base_and(cpu_registers* registers, uint8_t operand) {
    registers->A = registers->A & operand;
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
}

void base_xor(cpu_registers* registers, uint8_t memory_value) {
    registers->A = registers->A ^ memory_value;
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
}

void base_or(cpu_registers* registers, uint8_t memory_value) {
    registers->A = registers->A | memory_value;
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
}

// This function has to be able to shift both the A register and
// memory values.
void base_shift_left(cpu_registers* registers, uint8_t* value_to_shift) {
    const uint16_t carry_bit_position = 0x100;
    uint16_t result = *value_to_shift << 1;
    *value_to_shift = result & BYTE_MASK;

    if(result & carry_bit_position) set_cpu_flag(registers, CARRY_FLAG);
    else clear_cpu_flag(registers, CARRY_FLAG);
    determine_zero_flag(registers, *value_to_shift);
    determine_negative_flag(registers, *value_to_shift);
}

void base_shift_right(cpu_registers* registers, uint8_t* value_to_shift) {
    const uint16_t carry_bit_position = 0x1;
    uint8_t result = *value_to_shift >> 1;

    if(*value_to_shift & carry_bit_position) set_cpu_flag(registers, CARRY_FLAG);
    else clear_cpu_flag(registers, CARRY_FLAG);
    *value_to_shift = result;
    determine_zero_flag(registers, *value_to_shift);
    determine_negative_flag(registers, *value_to_shift);
}

    
void base_bit_test(cpu_registers* registers, uint8_t memory_value) {
    uint8_t and_result = registers->A & memory_value;
    determine_zero_flag(registers, and_result);
    if(memory_value & BIT6_BITMASK) set_cpu_flag(registers, OVERFLOW_FLAG);
    else clear_cpu_flag(registers, OVERFLOW_FLAG);
    if(memory_value & BIT7_BITMASK) set_cpu_flag(registers, NEGATIVE_FLAG);
    else clear_cpu_flag(registers, NEGATIVE_FLAG);
}

void base_compare(cpu_registers* registers, uint8_t register_value,
                  uint8_t memory_value)
{
    int8_t compare_result = (int8_t)(register_value) - (int8_t)memory_value;
    determine_zero_flag(registers, (uint8_t)compare_result);
    determine_negative_flag(registers, (uint8_t)compare_result);
    if(compare_result >= 0) set_cpu_flag(registers, CARRY_FLAG);
    else clear_cpu_flag(registers, CARRY_FLAG);
}

void base_decrement(cpu_registers* registers, uint8_t* value_to_decrement) {
    *value_to_decrement = *value_to_decrement - 1;
    determine_zero_flag(registers, *value_to_decrement);
    determine_negative_flag(registers, *value_to_decrement);
}

void base_increment(cpu_registers* registers, uint8_t* value_to_increment) {
    *value_to_increment = *value_to_increment + 1;
    determine_zero_flag(registers, *value_to_increment);
    determine_negative_flag(registers, *value_to_increment);
}

void base_load_register(cpu_registers* registers, uint8_t* register_to_load,
                        uint8_t memory_value)
{
    *register_to_load = memory_value;
    determine_zero_flag(registers, *register_to_load);
    determine_negative_flag(registers, *register_to_load);
}

void base_rotate_left(cpu_registers* registers, uint8_t* value_to_rotate) {
    uint16_t result = *value_to_rotate << 1;
    if(*value_to_rotate & BIT7_BITMASK) set_cpu_flag(registers, CARRY_FLAG);
    else clear_cpu_flag(registers, CARRY_FLAG);
    *value_to_rotate = result & BYTE_MASK;
    determine_zero_flag(registers, *value_to_rotate);
    determine_negative_flag(registers, *value_to_rotate);
}
