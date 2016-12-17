//*****************************************************************************
// Filename: memory_operations.h
// Author: Fisherman166
//
// Implements the memory operations for the CPU and PPU
//
//*****************************************************************************

#ifndef MEMORY_OPERATIONS_H
#define MEMORY_OPERATIONS_H

#include "common.h"
#include "common_cpu.h"

#define JOYPAD1_ADDRESS 0x4016
#define JOYPAD2_ADDRESS 0x4017

void init_RAM();
bool load_game();
uint8_t read_RAM(uint16_t);
void write_RAM(uint16_t, uint8_t);

//*****************************************************************************
// Specific address mode writes
//*****************************************************************************
void write_zeropage(cpu_registers*, uint8_t);
void write_zeropageX(cpu_registers*, uint8_t);
void write_absolute(cpu_registers*, uint8_t);
void write_absoluteX(cpu_registers*, uint8_t);

#endif

