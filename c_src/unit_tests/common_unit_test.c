//*****************************************************************************
// Filename: common_unit_test.c
// Author: Fisherman166
//
// Common functions used in all unit tests
//
// ****************************************************************************

#include "common_unit_test.h"

//*****************************************************************************
// Helper functions
//*****************************************************************************
void my_print(char* string) {
    printf("BASIC_CPU_OP_UT: %s", string);
}

bool compare_registers(cpu_registers* registers, uint8_t A, uint8_t X,
                              uint8_t Y, uint16_t PC, uint16_t S,
                              uint16_t flags)
{
    bool registers_match = 1;
    if(registers->A != A) registers_match = 0;
    else if(registers->X != X) registers_match = 0;
    else if(registers->Y != Y) registers_match = 0;
    else if(registers->PC != PC) registers_match = 0;
    else if(registers->S != S) registers_match = 0;
    else if(registers->flags != flags) registers_match = 0;
    return registers_match;
}

