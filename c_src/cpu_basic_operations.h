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
// General functions
//*****************************************************************************
void init_cpu_registers(cpu_registers*, uint8_t, uint8_t, uint8_t, uint16_t,
                        uint8_t, uint8_t);

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

void set_cpu_flag(cpu_registers*, uint8_t);
void clear_cpu_flag(cpu_registers*, uint8_t);
bool get_cpu_flag(cpu_registers*, uint8_t);
void check_value_for_zero_flag(cpu_registers*, uint8_t);

#endif

