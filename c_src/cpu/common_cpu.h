//*****************************************************************************
// Filename: common_cpu.h
// Author: Fisherman166
//
// Common datastructures that originate with the CPU
//
//*****************************************************************************

#ifndef COMMON_CPU_H
#define COMMON_CPU_H

#include "common.h"

typedef struct {
    uint8_t A;      // Accumulator
    uint8_t X;
    uint8_t Y;
    uint16_t PC;    // Program counter
    uint8_t S;      // Stack pointer
    uint8_t flags;  // Status flags
} cpu_registers;

#endif

