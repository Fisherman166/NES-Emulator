//*****************************************************************************
// Filename: cpu_basic_operations.c
// Author: Fisherman166
//
// Implements the basic CPU operations. Things like adds, subtracts, setting
// flags, pushing data onto the stack, etc
//
//*****************************************************************************

#include "cpu_basic_operations.h"


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
