//*****************************************************************************
// Filename: cpu_decode_logic.h
// Author: Fisherman166
//
// Implements all address decoding functions of the CPU
//
// ****************************************************************************

#ifndef CPU_DECODE_LOGIC_H
#define CPU_DECODE_LOGIC_H

#include "common.h"
#include "common_cpu.h"
#include "memory_operations.h"

uint8_t fetch_immediate(cpu_registers*);
uint8_t fetch_zeropage(cpu_registers*);

#endif
