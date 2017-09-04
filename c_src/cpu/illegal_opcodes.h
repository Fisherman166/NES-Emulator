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

uint8_t zeropage_SAX(cpu_registers*);
uint8_t zeropageY_SAX(cpu_registers*);
uint8_t absolute_SAX(cpu_registers*);
uint8_t indirectX_SAX(cpu_registers*);

uint8_t zeropage_DCP(cpu_registers*);
uint8_t zeropageX_DCP(cpu_registers*);
uint8_t absolute_DCP(cpu_registers*);
uint8_t absoluteX_DCP(cpu_registers*);
uint8_t absoluteY_DCP(cpu_registers*);
uint8_t indirectX_DCP(cpu_registers*);
uint8_t indirectY_DCP(cpu_registers*);

uint8_t zeropage_ISB(cpu_registers*);
uint8_t zeropageX_ISB(cpu_registers*);
uint8_t absolute_ISB(cpu_registers*);
uint8_t absoluteX_ISB(cpu_registers*);
uint8_t absoluteY_ISB(cpu_registers*);
uint8_t indirectX_ISB(cpu_registers*);
uint8_t indirectY_ISB(cpu_registers*);

#endif
