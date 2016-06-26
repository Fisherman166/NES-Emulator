//*****************************************************************************
// Filename: cpu_instructions.c
// Author: Fisherman166
//
// Implements the actual cpu_instructions
//
//*****************************************************************************

#include "cpu_instructions.h"
#include "cpu_basic_operations.h"
#include "cpu_decode_logic.h"

//*****************************************************************************
// ADC
//*****************************************************************************
void immediate_ADC(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_add(registers, data);
}


void zeropage_ADC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_add(registers, data);
}


void zeropageX_ADC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_add(registers, data);
}


void absolute_ADC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_add(registers, data);
}


bool absoluteX_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}


bool absoluteY_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}


void indirectX_ADC(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_add(registers, data);
}


bool indirectY_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}

//*****************************************************************************
// AND
//*****************************************************************************
void immediate_AND(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_and(registers, data);
}


void zeropage_AND(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_and(registers, data);
}


void zeropageX_AND(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_and(registers, data);
}


void absolute_AND(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_and(registers, data);
}


bool absoluteX_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}


bool absoluteY_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}


void indirectX_AND(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_and(registers, data);
}


bool indirectY_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}
