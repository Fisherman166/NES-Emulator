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
// Accumulator Compares
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
// X compares
//*****************************************************************************
void immediate_CPX(cpu_registers*);
void zeropage_CPX(cpu_registers*);
void absolute_CPX(cpu_registers*);

//*****************************************************************************
// Y compares
//*****************************************************************************
void immediate_CPY(cpu_registers*);
void zeropage_CPY(cpu_registers*);
void absolute_CPY(cpu_registers*);

#endif

