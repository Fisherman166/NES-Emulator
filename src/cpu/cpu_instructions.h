//*****************************************************************************
// Filename: cpu_instructions.h
// Author: Fisherman166
//
// Implements the actual cpu_instructions
//
//*****************************************************************************

#ifndef CPU_INSTRUCTIONS_H
#define CPU_INSTRUCTIONS_H

#include "common.h"
#include "common_cpu.h"

//*****************************************************************************
// ADC
//*****************************************************************************
uint8_t immediate_ADC(cpu_registers*);
uint8_t zeropage_ADC(cpu_registers*);
uint8_t zeropageX_ADC(cpu_registers*);
uint8_t absolute_ADC(cpu_registers*);
uint8_t absoluteX_ADC(cpu_registers*);
uint8_t absoluteY_ADC(cpu_registers*);
uint8_t indirectX_ADC(cpu_registers*);
uint8_t indirectY_ADC(cpu_registers*);

//*****************************************************************************
// AND
//*****************************************************************************
uint8_t immediate_AND(cpu_registers*);
uint8_t zeropage_AND(cpu_registers*);
uint8_t zeropageX_AND(cpu_registers*);
uint8_t absolute_AND(cpu_registers*);
uint8_t absoluteX_AND(cpu_registers*);
uint8_t absoluteY_AND(cpu_registers*);
uint8_t indirectX_AND(cpu_registers*);
uint8_t indirectY_AND(cpu_registers*);

//*****************************************************************************
// ASL
//*****************************************************************************
uint8_t accumulator_ASL(cpu_registers*);
uint8_t zeropage_ASL(cpu_registers*);
uint8_t zeropageX_ASL(cpu_registers*);
uint8_t absolute_ASL(cpu_registers*);
uint8_t absoluteX_ASL(cpu_registers*);

//*****************************************************************************
// Relative branches based on flags
//*****************************************************************************
uint8_t relative_BCC(cpu_registers*);
uint8_t relative_BCS(cpu_registers*);
uint8_t relative_BEQ(cpu_registers*);
uint8_t relative_BMI(cpu_registers*);
uint8_t relative_BNE(cpu_registers*);
uint8_t relative_BPL(cpu_registers*);
uint8_t relative_BVC(cpu_registers*);
uint8_t relative_BVS(cpu_registers*);

//*****************************************************************************
// Interrupts
//*****************************************************************************
void push_PC_onto_stack(cpu_registers*, uint16_t);
uint8_t implied_BRK(cpu_registers*);

//*****************************************************************************
// BIT tests
//*****************************************************************************
uint8_t zeropage_BIT(cpu_registers*);
uint8_t absolute_BIT(cpu_registers*);

//*****************************************************************************
// Flag Clearers
//*****************************************************************************
uint8_t implied_CLC(cpu_registers*);
uint8_t implied_CLD(cpu_registers*);
uint8_t implied_CLI(cpu_registers*);
uint8_t implied_CLV(cpu_registers*);

//*****************************************************************************
// CMP
//*****************************************************************************
uint8_t immediate_CMP(cpu_registers*);
uint8_t zeropage_CMP(cpu_registers*);
uint8_t zeropageX_CMP(cpu_registers*);
uint8_t absolute_CMP(cpu_registers*);
uint8_t absoluteX_CMP(cpu_registers*);
uint8_t absoluteY_CMP(cpu_registers*);
uint8_t indirectX_CMP(cpu_registers*);
uint8_t indirectY_CMP(cpu_registers*);

//*****************************************************************************
// CPX
//*****************************************************************************
uint8_t immediate_CPX(cpu_registers*);
uint8_t zeropage_CPX(cpu_registers*);
uint8_t absolute_CPX(cpu_registers*);

//*****************************************************************************
// CPY
//*****************************************************************************
uint8_t immediate_CPY(cpu_registers*);
uint8_t zeropage_CPY(cpu_registers*);
uint8_t absolute_CPY(cpu_registers*);

//*****************************************************************************
// DEC
//*****************************************************************************
uint8_t zeropage_DEC(cpu_registers*);
uint8_t zeropageX_DEC(cpu_registers*);
uint8_t absolute_DEC(cpu_registers*);
uint8_t absoluteX_DEC(cpu_registers*);

//*****************************************************************************
// DEX and DEY
//*****************************************************************************
uint8_t implied_DEX(cpu_registers*);
uint8_t implied_DEY(cpu_registers*);

//*****************************************************************************
// EOR
//*****************************************************************************
uint8_t immediate_EOR(cpu_registers*);
uint8_t zeropage_EOR(cpu_registers*);
uint8_t zeropageX_EOR(cpu_registers*);
uint8_t absolute_EOR(cpu_registers*);
uint8_t absoluteX_EOR(cpu_registers*);
uint8_t absoluteY_EOR(cpu_registers*);
uint8_t indirectX_EOR(cpu_registers*);
uint8_t indirectY_EOR(cpu_registers*);

//*****************************************************************************
// INC
//*****************************************************************************
uint8_t zeropage_INC(cpu_registers*);
uint8_t zeropageX_INC(cpu_registers*);
uint8_t absolute_INC(cpu_registers*);
uint8_t absoluteX_INC(cpu_registers*);

//*****************************************************************************
// INX and INY
//*****************************************************************************
uint8_t implied_INX(cpu_registers*);
uint8_t implied_INY(cpu_registers*);

//*****************************************************************************
// JMP, JSR
//*****************************************************************************
uint8_t absolute_JMP(cpu_registers*);
uint8_t indirect_JMP(cpu_registers*);
uint8_t absolute_JSR(cpu_registers*);

//*****************************************************************************
// LDA
//*****************************************************************************
uint8_t immediate_LDA(cpu_registers*);
uint8_t zeropage_LDA(cpu_registers*);
uint8_t zeropageX_LDA(cpu_registers*);
uint8_t absolute_LDA(cpu_registers*);
uint8_t absoluteX_LDA(cpu_registers*);
uint8_t absoluteY_LDA(cpu_registers*);
uint8_t indirectX_LDA(cpu_registers*);
uint8_t indirectY_LDA(cpu_registers*);

//*****************************************************************************
// LDX
//*****************************************************************************
uint8_t immediate_LDX(cpu_registers*);
uint8_t zeropage_LDX(cpu_registers*);
uint8_t zeropageY_LDX(cpu_registers*);
uint8_t absolute_LDX(cpu_registers*);
uint8_t absoluteY_LDX(cpu_registers*);

//*****************************************************************************
// LDY
//*****************************************************************************
uint8_t immediate_LDY(cpu_registers*);
uint8_t zeropage_LDY(cpu_registers*);
uint8_t zeropageX_LDY(cpu_registers*);
uint8_t absolute_LDY(cpu_registers*);
uint8_t absoluteX_LDY(cpu_registers*);

//*****************************************************************************
// LSR
//*****************************************************************************
uint8_t accumulator_LSR(cpu_registers*);
uint8_t zeropage_LSR(cpu_registers*);
uint8_t zeropageX_LSR(cpu_registers*);
uint8_t absolute_LSR(cpu_registers*);
uint8_t absoluteX_LSR(cpu_registers*);

uint8_t implied_NOP(cpu_registers*);

//*****************************************************************************
// ORA
//*****************************************************************************
uint8_t immediate_ORA(cpu_registers*);
uint8_t zeropage_ORA(cpu_registers*);
uint8_t zeropageX_ORA(cpu_registers*);
uint8_t absolute_ORA(cpu_registers*);
uint8_t absoluteX_ORA(cpu_registers*);
uint8_t absoluteY_ORA(cpu_registers*);
uint8_t indirectX_ORA(cpu_registers*);
uint8_t indirectY_ORA(cpu_registers*);

//*****************************************************************************
// Stack instructions
//*****************************************************************************
uint8_t implied_PHA(cpu_registers*);
uint8_t implied_PHP(cpu_registers*);
uint8_t implied_PLA(cpu_registers*);
uint8_t implied_PLP(cpu_registers*);

//*****************************************************************************
// ROL
//*****************************************************************************
uint8_t accumulator_ROL(cpu_registers*);
uint8_t zeropage_ROL(cpu_registers*);
uint8_t zeropageX_ROL(cpu_registers*);
uint8_t absolute_ROL(cpu_registers*);
uint8_t absoluteX_ROL(cpu_registers*);

//*****************************************************************************
// ROR
//*****************************************************************************
uint8_t accumulator_ROR(cpu_registers*);
uint8_t zeropage_ROR(cpu_registers*);
uint8_t zeropageX_ROR(cpu_registers*);
uint8_t absolute_ROR(cpu_registers*);
uint8_t absoluteX_ROR(cpu_registers*);

//*****************************************************************************
// Return instructions
//*****************************************************************************
uint8_t implied_RTI(cpu_registers*);
uint8_t implied_RTS(cpu_registers*);

//*****************************************************************************
// SBC
//*****************************************************************************
uint8_t immediate_SBC(cpu_registers*);
uint8_t zeropage_SBC(cpu_registers*);
uint8_t zeropageX_SBC(cpu_registers*);
uint8_t absolute_SBC(cpu_registers*);
uint8_t absoluteX_SBC(cpu_registers*);
uint8_t absoluteY_SBC(cpu_registers*);
uint8_t indirectX_SBC(cpu_registers*);
uint8_t indirectY_SBC(cpu_registers*);

//*****************************************************************************
// Flag setters
//*****************************************************************************
uint8_t implied_SEC(cpu_registers*);
uint8_t implied_SED(cpu_registers*);
uint8_t implied_SEI(cpu_registers*);

//*****************************************************************************
// STA
//*****************************************************************************
uint8_t zeropage_STA(cpu_registers*);
uint8_t zeropageX_STA(cpu_registers*);
uint8_t absolute_STA(cpu_registers*);
uint8_t absoluteX_STA(cpu_registers*);
uint8_t absoluteY_STA(cpu_registers*);
uint8_t indirectX_STA(cpu_registers*);
uint8_t indirectY_STA(cpu_registers*);

//*****************************************************************************
// STX
//*****************************************************************************
uint8_t zeropage_STX(cpu_registers*);
uint8_t zeropageY_STX(cpu_registers*);
uint8_t absolute_STX(cpu_registers*);

//*****************************************************************************
// STY
//*****************************************************************************
uint8_t zeropage_STY(cpu_registers*);
uint8_t zeropageX_STY(cpu_registers*);
uint8_t absolute_STY(cpu_registers*);

//*****************************************************************************
// Transfer instructions
//*****************************************************************************
uint8_t implied_TAX(cpu_registers*);
uint8_t implied_TAY(cpu_registers*);
uint8_t implied_TSX(cpu_registers*);
uint8_t implied_TXA(cpu_registers*);
uint8_t implied_TXS(cpu_registers*);
uint8_t implied_TYA(cpu_registers*);

#endif

