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
#define CARRY_FLAG       0x01
#define ZERO_FLAG        0x02
#define INTERRUPT_FLAG   0x04
#define DECIMAL_FLAG     0x08
#define B1_FLAG          0x10
#define B2_FLAG          0x20
#define OVERFLOW_FLAG    0x40
#define NEGATIVE_FLAG    0x80

void set_cpu_flag(cpu_registers*, uint8_t);
void clear_cpu_flag(cpu_registers*, uint8_t);
bool get_cpu_flag(cpu_registers*, uint8_t);
void determine_zero_flag(cpu_registers*, uint8_t);
void determine_negative_flag(cpu_registers*, uint8_t);

//*****************************************************************************
// Basic opcode operations
//*****************************************************************************
void base_add(cpu_registers*, uint8_t);
void base_and(cpu_registers*, uint8_t);
void base_shift_left(cpu_registers*, uint8_t*);
void base_bit_test(cpu_registers*, uint8_t);

#endif

