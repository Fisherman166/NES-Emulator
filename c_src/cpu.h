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

void open_cpu_debug_logfile();
void close_cpu_debug_logfile();
void cold_boot_init(cpu_registers*);
void execute_interpreter_cycle(cpu_registers*);

#endif


