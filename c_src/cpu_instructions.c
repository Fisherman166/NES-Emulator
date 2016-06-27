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

static uint8_t branch_on_flag_set(cpu_registers* registers, uint8_t flag) {
    uint8_t additional_cycles = 0;
    if(get_cpu_flag(registers, flag)) {
        additional_cycles++;
        additional_cycles += branch_relative(registers);
    }
    return additional_cycles;
}


static uint8_t branch_on_flag_clear(cpu_registers* registers, uint8_t flag) {
    uint8_t additional_cycles = 0;
    if(!get_cpu_flag(registers, flag)) {
        additional_cycles++;
        additional_cycles += branch_relative(registers);
    }
    return additional_cycles;
}

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

//*****************************************************************************
// ASL
//*****************************************************************************
void accumulator_ASL(cpu_registers* registers) {
    base_shift_left(registers, &(registers->A));
}


void zeropage_ASL(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_shift_left(registers, &data);
    write_zeropage(registers, data);
}


void zeropageX_ASL(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_shift_left(registers, &data);
    write_zeropageX(registers, data);
}


void absolute_ASL(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_shift_left(registers, &data);
    write_absolute(registers, data);
}


void absoluteX_ASL(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_shift_left(registers, &data);
    write_absoluteX(registers, data);
}


//*****************************************************************************
// Relative branches based on flags
//*****************************************************************************
uint8_t BCC(cpu_registers* registers) {
    return branch_on_flag_clear(registers, CARRY_FLAG);
}


uint8_t BSC(cpu_registers* registers) {
    return branch_on_flag_set(registers, CARRY_FLAG);
}


uint8_t BEQ(cpu_registers* registers) {
    return branch_on_flag_set(registers, ZERO_FLAG);
}


uint8_t BMI(cpu_registers* registers) {
    return branch_on_flag_set(registers, NEGATIVE_FLAG);
}


uint8_t BNE(cpu_registers* registers) {
    return branch_on_flag_clear(registers, ZERO_FLAG);
}

uint8_t BPL(cpu_registers* registers) {
    return branch_on_flag_clear(registers, NEGATIVE_FLAG);
}


uint8_t BCV(cpu_registers* registers) {
    return branch_on_flag_clear(registers, OVERFLOW_FLAG);
}


uint8_t BVS(cpu_registers* registers) {
    return branch_on_flag_set(registers, OVERFLOW_FLAG);
}

void zeropage_BIT(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_bit_test(registers, data);
}

void absolute_BIT(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_bit_test(registers, data);
}
