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
uint8_t BCC(cpu_registers*);
uint8_t BSC(cpu_registers*);
uint8_t BEQ(cpu_registers*);
uint8_t BMI(cpu_registers*);
uint8_t BNE(cpu_registers*);
uint8_t BPL(cpu_registers*);
uint8_t BVC(cpu_registers*);
uint8_t BVS(cpu_registers*);

void zeropage_BIT(cpu_registers*);
void absolute_BIT(cpu_registers*);



#endif

