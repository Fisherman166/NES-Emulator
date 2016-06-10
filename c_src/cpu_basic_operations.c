//*****************************************************************************
// Filename: cpu_basic_operations.c
// Author: Fisherman166
//
// Implements the basic CPU operations. Things like adds, subtracts, setting
// flags, pushing data onto the stack, etc
//
//*****************************************************************************

#include "cpu_basic_operations.h"

//*****************************************************************************
// Status flag functions
//*****************************************************************************
#define CARRY_BIT       0x01
#define ZERO_BIT        0x02
#define INTERRUPT_BIT   0x04
#define DECIMAL_BIT     0x08
#define B1_BIT          0x10
#define B2_BIT          0x20
#define OVERFLOW_BIT    0x40
#define NEGATIVE_BIT    0x80

void set_cpu_flag(cpu_registers* registers, uint8_t flag_to_set) {
    registers->flags |= flag_to_set;
}

void clear_cpu_flag(cpu_registers* registers, uint8_t flag_to_clear) {
    registers->flags &= ~flag_to_clear;
}

