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
void immediate_ADC(cpu_registers*);
void zeropage_ADC(cpu_registers*);
void zeropageX_ADC(cpu_registers*);
void absolute_ADC(cpu_registers*);
bool absoluteX_ADC(cpu_registers*);
bool absoluteY_ADC(cpu_registers*);
void indirectX_ADC(cpu_registers*);
bool indirectY_ADC(cpu_registers*);


//*****************************************************************************
// AND
//*****************************************************************************
void immediate_AND(cpu_registers*);
void zeropage_AND(cpu_registers*);
void zeropageX_AND(cpu_registers*);
void absolute_AND(cpu_registers*);
bool absoluteX_AND(cpu_registers*);
bool absoluteY_AND(cpu_registers*);
void indirectX_AND(cpu_registers*);
bool indirectY_AND(cpu_registers*);

//*****************************************************************************
// ASL
//*****************************************************************************
void accumulator_ASL(cpu_registers*);
void zeropage_ASL(cpu_registers*);
void zeropageX_ASL(cpu_registers*);
void absolute_ASL(cpu_registers*);
void absoluteX_ASL(cpu_registers*);

//*****************************************************************************
// Relative branches based on flags
//*****************************************************************************
uint8_t implied_BCC(cpu_registers*);
uint8_t implied_BSC(cpu_registers*);
uint8_t implied_BEQ(cpu_registers*);
uint8_t implied_BMI(cpu_registers*);
uint8_t implied_BNE(cpu_registers*);
uint8_t implied_BPL(cpu_registers*);
uint8_t implied_BVC(cpu_registers*);
uint8_t implied_BVS(cpu_registers*);

//*****************************************************************************
// Interrupts
//*****************************************************************************
void implied_BRK(cpu_registers*);

//*****************************************************************************
// BIT tests
//*****************************************************************************
void zeropage_BIT(cpu_registers*);
void absolute_BIT(cpu_registers*);

//*****************************************************************************
// Flag Clearers
//*****************************************************************************
void implied_CLC(cpu_registers*);
void implied_CLD(cpu_registers*);
void implied_CLI(cpu_registers*);
void implied_CLV(cpu_registers*);

//*****************************************************************************
// CMP
//*****************************************************************************
void immediate_CMP(cpu_registers*);
void zeropage_CMP(cpu_registers*);
void zeropageX_CMP(cpu_registers*);
void absolute_CMP(cpu_registers*);
bool absoluteX_CMP(cpu_registers*);
bool absoluteY_CMP(cpu_registers*);
void indirectX_CMP(cpu_registers*);
bool indirectY_CMP(cpu_registers*);

//*****************************************************************************
// CPX
//*****************************************************************************
void immediate_CPX(cpu_registers*);
void zeropage_CPX(cpu_registers*);
void absolute_CPX(cpu_registers*);

//*****************************************************************************
// CPY
//*****************************************************************************
void immediate_CPY(cpu_registers*);
void zeropage_CPY(cpu_registers*);
void absolute_CPY(cpu_registers*);

//*****************************************************************************
// DEC
//*****************************************************************************
void zeropage_DEC(cpu_registers*);
void zeropageX_DEC(cpu_registers*);
void absolute_DEC(cpu_registers*);
void absoluteX_DEC(cpu_registers*);

//*****************************************************************************
// DEX and DEY
//*****************************************************************************
void implied_DEX(cpu_registers*);
void implied_DEY(cpu_registers*);

//*****************************************************************************
// EOR
//*****************************************************************************
void immediate_EOR(cpu_registers*);
void zeropage_EOR(cpu_registers*);
void zeropageX_EOR(cpu_registers*);
void absolute_EOR(cpu_registers*);
bool absoluteX_EOR(cpu_registers*);
bool absoluteY_EOR(cpu_registers*);
void indirectX_EOR(cpu_registers*);
bool indirectY_EOR(cpu_registers*);

//*****************************************************************************
// INC
//*****************************************************************************
void zeropage_INC(cpu_registers*);
void zeropageX_INC(cpu_registers*);
void absolute_INC(cpu_registers*);
void absoluteX_INC(cpu_registers*);

//*****************************************************************************
// INX and INY
//*****************************************************************************
void implied_INX(cpu_registers*);
void implied_INY(cpu_registers*);

//*****************************************************************************
// JMP, JSR
//*****************************************************************************
void absolute_JMP(cpu_registers*); // Use for both JMPS - same thing
void absolute_JSR(cpu_registers*);

//*****************************************************************************
// LDA
//*****************************************************************************
void immediate_LDA(cpu_registers*);
void zeropage_LDA(cpu_registers*);
void zeropageX_LDA(cpu_registers*);
void absolute_LDA(cpu_registers*);
bool absoluteX_LDA(cpu_registers*);
bool absoluteY_LDA(cpu_registers*);
void indirectX_LDA(cpu_registers*);
bool indirectY_LDA(cpu_registers*);

//*****************************************************************************
// LDX
//*****************************************************************************
void immediate_LDX(cpu_registers*);
void zeropage_LDX(cpu_registers*);
void zeropageY_LDX(cpu_registers*);
void absolute_LDX(cpu_registers*);
bool absoluteY_LDX(cpu_registers*);

//*****************************************************************************
// LSR
//*****************************************************************************
void accumulator_LSR(cpu_registers*);
void zeropage_LSR(cpu_registers*);
void zeropageX_LSR(cpu_registers*);
void absolute_LSR(cpu_registers*);
void absoluteX_LSR(cpu_registers*);

void implied_NOP();

#endif

