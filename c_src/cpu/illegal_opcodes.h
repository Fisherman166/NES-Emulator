#ifndef ILLEGAL_OPCODES
#define ILLEGAL_OPCODES

#include "common_cpu.h"

uint8_t immediate_NOP(cpu_registers*);
uint8_t zeropage_NOP(cpu_registers*);
uint8_t zeropageX_NOP(cpu_registers*);
uint8_t absolute_NOP(cpu_registers*);
uint8_t absoluteX_NOP(cpu_registers*);

uint8_t zeropage_LAX(cpu_registers*);
uint8_t zeropageY_LAX(cpu_registers*);
uint8_t absolute_LAX(cpu_registers*);
uint8_t absoluteY_LAX(cpu_registers*);
uint8_t indirectX_LAX(cpu_registers*);
uint8_t indirectY_LAX(cpu_registers*);

#endif
