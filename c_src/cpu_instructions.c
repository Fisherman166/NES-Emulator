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
uint8_t implied_BCC(cpu_registers* registers) {
    return branch_on_flag_clear(registers, CARRY_FLAG);
}


uint8_t implied_BSC(cpu_registers* registers) {
    return branch_on_flag_set(registers, CARRY_FLAG);
}


uint8_t implied_BEQ(cpu_registers* registers) {
    return branch_on_flag_set(registers, ZERO_FLAG);
}


uint8_t implied_BMI(cpu_registers* registers) {
    return branch_on_flag_set(registers, NEGATIVE_FLAG);
}


uint8_t implied_BNE(cpu_registers* registers) {
    return branch_on_flag_clear(registers, ZERO_FLAG);
}

uint8_t implied_BPL(cpu_registers* registers) {
    return branch_on_flag_clear(registers, NEGATIVE_FLAG);
}


uint8_t implied_BCV(cpu_registers* registers) {
    return branch_on_flag_clear(registers, OVERFLOW_FLAG);
}


uint8_t implied_BVS(cpu_registers* registers) {
    return branch_on_flag_set(registers, OVERFLOW_FLAG);
}

//*****************************************************************************
// Interrupts
//*****************************************************************************
static void push_PC_onto_stack(cpu_registers* registers, uint16_t PC) {
    const uint16_t high_byte_mask = 0xFF00;
    uint8_t low_byte = PC & BYTE_MASK;
    uint8_t high_byte = ((PC & high_byte_mask) >> 8) & BYTE_MASK;
    push_stack(registers, low_byte);
    push_stack(registers, high_byte);
}

static void load_IRQ_vector_into_PC(cpu_registers* registers) {
    const uint16_t low_byte_IRQ_vector = 0xFFFE;
    const uint16_t high_byte_IRQ_vector = 0xFFFF;
    registers->PC = read_RAM(high_byte_IRQ_vector) << 8;
    registers->PC |= read_RAM(low_byte_IRQ_vector);
}

void implied_BRK(cpu_registers* registers) {
    // Pushes PC + 2 from instruction fetch for some reason
    push_PC_onto_stack(registers, registers->PC + 1);
    push_stack(registers, registers->flags);
    load_IRQ_vector_into_PC(registers);
    set_cpu_flag(registers, INTERRUPT_FLAG);
}

//*****************************************************************************
// BIT tests
//*****************************************************************************
void zeropage_BIT(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_bit_test(registers, data);
}

void absolute_BIT(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_bit_test(registers, data);
}

//*****************************************************************************
// Flag Clearers
//*****************************************************************************
void implied_CLC(cpu_registers* registers) {
    clear_cpu_flag(registers, CARRY_FLAG);
}

void implied_CLD(cpu_registers* registers) {
    clear_cpu_flag(registers, DECIMAL_FLAG);
}

void implied_CLI(cpu_registers* registers) {
    clear_cpu_flag(registers, INTERRUPT_FLAG);
}

void implied_CLV(cpu_registers* registers) {
    clear_cpu_flag(registers, OVERFLOW_FLAG);
}

//*****************************************************************************
// CMP
//*****************************************************************************
void immediate_CMP(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->A, data);
}

void zeropage_CMP(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->A, data);
}

void zeropageX_CMP(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_compare(registers, registers->A, data);
}

void absolute_CMP(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->A, data);
}

bool absoluteX_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

bool absoluteY_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

void indirectX_CMP(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_compare(registers, registers->A, data);
}

bool indirectY_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

//*****************************************************************************
// CPX
//*****************************************************************************
void immediate_CPX(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->X, data);
}

void zeropage_CPX(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->X, data);
}

void absolute_CPX(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->X, data);
}

//*****************************************************************************
// CPY
//*****************************************************************************
void immediate_CPY(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->Y, data);
}

void zeropage_CPY(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->Y, data);
}

void absolute_CPY(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->Y, data);
}

//*****************************************************************************
// DEC
//*****************************************************************************
void zeropage_DEC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_decrement(registers, &data);
    write_zeropage(registers, data);
}

void zeropageX_DEC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_decrement(registers, &data);
    write_zeropageX(registers, data);
}

void absolute_DEC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_decrement(registers, &data);
    write_absolute(registers, data);
}

void absoluteX_DEC(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_decrement(registers, &data);
    write_absoluteX(registers, data);
}

//*****************************************************************************
// DEX and DEY
//*****************************************************************************
void implied_DEX(cpu_registers* registers) {
    base_decrement(registers, &(registers->X));
}

void implied_DEY(cpu_registers* registers) {
    base_decrement(registers, &(registers->Y));
}

//*****************************************************************************
// EOR
//*****************************************************************************
void immediate_EOR(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_xor(registers, data);
}

void zeropage_EOR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_xor(registers, data);
}

void zeropageX_EOR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_xor(registers, data);
}

void absolute_EOR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_xor(registers, data);
}

bool absoluteX_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}

bool absoluteY_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}

void indirectX_EOR(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_xor(registers, data);
}

bool indirectY_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}
