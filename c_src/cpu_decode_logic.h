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

//*****************************************************************************
// Address calculation functions
//*****************************************************************************
uint8_t calc_zeropage_address(cpu_registers*);
uint8_t calc_zeropageX_address(cpu_registers*);
uint8_t calc_zeropageY_address(cpu_registers*);

uint16_t calc_absolute_address(cpu_registers*);
uint16_t calc_absoluteX_address(cpu_registers*);
uint16_t calc_absoluteY_address(cpu_registers*);

uint16_t calc_indirectX_address(cpu_registers*);
uint16_t calc_indirectY_address(cpu_registers*);

//*****************************************************************************
// Data read functions
//*****************************************************************************
uint8_t fetch_immediate(cpu_registers*);
uint8_t fetch_zeropage(cpu_registers*);
uint8_t fetch_zeropageX(cpu_registers*);
uint8_t fetch_zeropageY(cpu_registers*);
uint8_t fetch_absolute(cpu_registers*);
uint8_t fetch_absoluteX(cpu_registers*, bool*);
uint8_t fetch_absoluteY(cpu_registers*, bool*);
uint8_t fetch_indirectX(cpu_registers*);
uint8_t fetch_indirectY(cpu_registers*, bool*);


bool branch_relative(cpu_registers*);


#endif
