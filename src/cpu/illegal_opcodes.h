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

uint8_t zeropage_SLO(cpu_registers*);
uint8_t zeropageX_SLO(cpu_registers*);
uint8_t absolute_SLO(cpu_registers*);
uint8_t absoluteX_SLO(cpu_registers*);
uint8_t absoluteY_SLO(cpu_registers*);
uint8_t indirectX_SLO(cpu_registers*);
uint8_t indirectY_SLO(cpu_registers*);

uint8_t zeropage_RLA(cpu_registers*);
uint8_t zeropageX_RLA(cpu_registers*);
uint8_t absolute_RLA(cpu_registers*);
uint8_t absoluteX_RLA(cpu_registers*);
uint8_t absoluteY_RLA(cpu_registers*);
uint8_t indirectX_RLA(cpu_registers*);
uint8_t indirectY_RLA(cpu_registers*);

uint8_t zeropage_SRE(cpu_registers*);
uint8_t zeropageX_SRE(cpu_registers*);
uint8_t absolute_SRE(cpu_registers*);
uint8_t absoluteX_SRE(cpu_registers*);
uint8_t absoluteY_SRE(cpu_registers*);
uint8_t indirectX_SRE(cpu_registers*);
uint8_t indirectY_SRE(cpu_registers*);

uint8_t zeropage_RRA(cpu_registers*);
uint8_t zeropageX_RRA(cpu_registers*);
uint8_t absolute_RRA(cpu_registers*);
uint8_t absoluteX_RRA(cpu_registers*);
uint8_t absoluteY_RRA(cpu_registers*);
uint8_t indirectX_RRA(cpu_registers*);
uint8_t indirectY_RRA(cpu_registers*);

#endif
