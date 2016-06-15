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
    uint8_t shifted_flags;
    switch(flag_to_get) {
        case CARRY_BIT:     shifted_flags = registers->flags;      break;
        case ZERO_BIT:      shifted_flags = registers->flags >> 1; break;
        case INTERRUPT_BIT: shifted_flags = registers->flags >> 2; break;
        case DECIMAL_BIT:   shifted_flags = registers->flags >> 3; break;
        case B1_BIT:        shifted_flags = registers->flags >> 4; break;
        case B2_BIT:        shifted_flags = registers->flags >> 5; break;
        case OVERFLOW_BIT:  shifted_flags = registers->flags >> 6; break;
        case NEGATIVE_BIT:  shifted_flags = registers->flags >> 7; break;
        default: printf("ERROR: Trying to get the value of a flag that does not exist\n");
    }
    return shifted_flags & 0x1;
}
