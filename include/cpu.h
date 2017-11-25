//*****************************************************************************
// Filename: cpu.h
// Author: Fisherman166
//
// Implements the top level CPU functions
//
// ****************************************************************************

#ifndef CPU_H
#define CPU_H

#include "common.h"
#include "common_cpu.h"

#ifdef DEBUG
void open_cpu_debug_logfile();
void close_cpu_debug_logfile();
#endif //Debug

void cold_boot_init(cpu_registers*);
uint8_t execute_interpreter_cycle(cpu_registers*, bool);
void start_DMA(uint8_t);

#endif


