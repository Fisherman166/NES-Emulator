//*****************************************************************************
// Filename: RAM.h
// Author: Fisherman166
//
// Implements the RAM memory operations
//
//*****************************************************************************

#ifndef RAM_H
#define RAM_H

#include "common.h"
#include "common_cpu.h"

#define JOYPAD1_ADDRESS 0x4016
#define JOYPAD2_ADDRESS 0x4017

#define PPUCTRL_ADDRESS     0x2000
#define PPUMASK_ADDRESS     0x2001
#define PPUSTATUS_ADDRESS   0x2002
#define OAMADDR_ADDRESS     0x2003
#define OAMDATA_ADDRESS     0x2004
#define PPUSCROLL_ADDRESS   0x2005
#define PPUADDR_ADDRESS     0x2006
#define PPUDATA_ADDRESS     0x2007

void init_RAM();

uint8_t read_RAM(uint16_t);
uint8_t debug_read_RAM(uint16_t);
void write_RAM(uint16_t, uint8_t);

void clear_vblank_bit();
void set_vblank_bit();
bool NMI_and_vblank_set();

void write_zeropage(cpu_registers*, uint8_t);
void write_zeropageX(cpu_registers*, uint8_t);
void write_absolute(cpu_registers*, uint8_t);
void write_absoluteX(cpu_registers*, uint8_t);

#endif

