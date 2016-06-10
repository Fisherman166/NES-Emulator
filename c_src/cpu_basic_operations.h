//*****************************************************************************
// Filename: cpu_basic_operations.h
// Author: Fisherman166
//
// Implements the basic CPU operations. Things like adds, subtracts, setting
// flags, pushing data onto the stack, etc
//
//*****************************************************************************

#ifndef CPU_BASIC_OPERATIONS_H
#define CPU_BASIC_OPERATIONS_H

#include "common.h"
#include "common_cpu.h"

//*****************************************************************************
// Status flag functions
//*****************************************************************************
void set_cpu_flag(cpu_registers*, uint8_t);
void clear_cpu_flag(cpu_registers*, uint8_t);

#endif

