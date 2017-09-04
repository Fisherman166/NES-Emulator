#ifndef ILLEGAL_OPCODES
#define ILLEGAL_OPCODES

#include "common_cpu.h"

uint8_t zeropage_NOP(cpu_registers*);
uint8_t zeropageX_NOP(cpu_registers*);
uint8_t absolute_NOP(cpu_registers*);
uint8_t absoluteX_NOP(cpu_registers*);

#endif
