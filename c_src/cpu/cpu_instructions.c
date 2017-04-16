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

#define ZERO_EXTRA_CYCLES 0
#define ONE_EXTRA_CYCLE 1
#define TWO_EXTRA_CYCLES 2

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
uint8_t immediate_ADC(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_add(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ADC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_add(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ADC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_add(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ADC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_add(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}

uint8_t absoluteY_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}

uint8_t indirectX_ADC(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_add(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_ADC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_add(registers, data);
    return page_crossed;
}

//*****************************************************************************
// AND
//*****************************************************************************
uint8_t immediate_AND(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_and(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_AND(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_and(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_AND(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_and(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_AND(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_and(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}

uint8_t absoluteY_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}

uint8_t indirectX_AND(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_and(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_AND(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_and(registers, data);
    return page_crossed;
}

//*****************************************************************************
// ASL
//*****************************************************************************
uint8_t accumulator_ASL(cpu_registers* registers) {
    base_shift_left(registers, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ASL(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_shift_left(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ASL(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_shift_left(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ASL(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_shift_left(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ASL(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_shift_left(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// Relative branches based on flags
//*****************************************************************************
uint8_t relative_BCC(cpu_registers* registers) {
    return branch_on_flag_clear(registers, CARRY_FLAG);
}

uint8_t relative_BCS(cpu_registers* registers) {
    return branch_on_flag_set(registers, CARRY_FLAG);
}

uint8_t relative_BEQ(cpu_registers* registers) {
    return branch_on_flag_set(registers, ZERO_FLAG);
}

uint8_t relative_BMI(cpu_registers* registers) {
    return branch_on_flag_set(registers, NEGATIVE_FLAG);
}

uint8_t relative_BNE(cpu_registers* registers) {
    return branch_on_flag_clear(registers, ZERO_FLAG);
}

uint8_t relative_BPL(cpu_registers* registers) {
    return branch_on_flag_clear(registers, NEGATIVE_FLAG);
}

uint8_t relative_BVC(cpu_registers* registers) {
    return branch_on_flag_clear(registers, OVERFLOW_FLAG);
}

uint8_t relative_BVS(cpu_registers* registers) {
    return branch_on_flag_set(registers, OVERFLOW_FLAG);
}

//*****************************************************************************
// Interrupts
//*****************************************************************************
void push_PC_onto_stack(cpu_registers* registers, uint16_t PC) {
    const uint16_t high_byte_mask = 0xFF00;
    uint8_t low_byte = PC & BYTE_MASK;
    uint8_t high_byte = ((PC & high_byte_mask) >> 8) & BYTE_MASK;
    push_stack(registers, high_byte);
    push_stack(registers, low_byte);
}

static void pop_PC_off_stack(cpu_registers* registers) {
    uint8_t low_byte = pop_stack(registers);
    uint8_t high_byte = pop_stack(registers);
    registers->PC = (high_byte << 8) | low_byte;
}

static void load_IRQ_vector_into_PC(cpu_registers* registers) {
    const uint16_t low_byte_IRQ_vector = 0xFFFE;
    const uint16_t high_byte_IRQ_vector = 0xFFFF;
    registers->PC = read_RAM(high_byte_IRQ_vector) << 8;
    registers->PC |= read_RAM(low_byte_IRQ_vector);
}

uint8_t implied_BRK(cpu_registers* registers) {
    // BRK sets these two bits for some reason
    uint8_t BRK_special_flags = 0x30;
    // Pushes PC + 2 from instruction fetch for some reason
    push_PC_onto_stack(registers, registers->PC + 1);
    push_stack(registers, registers->flags | BRK_special_flags);
    load_IRQ_vector_into_PC(registers);
    set_cpu_flag(registers, INTERRUPT_FLAG);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// BIT tests
//*****************************************************************************
uint8_t zeropage_BIT(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_bit_test(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_BIT(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_bit_test(registers, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// Flag Clearers
//*****************************************************************************
uint8_t implied_CLC(cpu_registers* registers) {
    clear_cpu_flag(registers, CARRY_FLAG);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_CLD(cpu_registers* registers) {
    clear_cpu_flag(registers, DECIMAL_FLAG);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_CLI(cpu_registers* registers) {
    clear_cpu_flag(registers, INTERRUPT_FLAG);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_CLV(cpu_registers* registers) {
    clear_cpu_flag(registers, OVERFLOW_FLAG);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// CMP
//*****************************************************************************
uint8_t immediate_CMP(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_CMP(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_CMP(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_CMP(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

uint8_t absoluteY_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

uint8_t indirectX_CMP(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_CMP(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_compare(registers, registers->A, data);
    return page_crossed;
}

//*****************************************************************************
// CPX
//*****************************************************************************
uint8_t immediate_CPX(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->X, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_CPX(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->X, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_CPX(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->X, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// CPY
//*****************************************************************************
uint8_t immediate_CPY(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_compare(registers, registers->Y, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_CPY(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_compare(registers, registers->Y, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_CPY(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_compare(registers, registers->Y, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// DEC
//*****************************************************************************
uint8_t zeropage_DEC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_decrement(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_DEC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_decrement(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_DEC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_decrement(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_DEC(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_decrement(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// DEX and DEY
//*****************************************************************************
uint8_t implied_DEX(cpu_registers* registers) {
    base_decrement(registers, &(registers->X));
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_DEY(cpu_registers* registers) {
    base_decrement(registers, &(registers->Y));
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// EOR
//*****************************************************************************
uint8_t immediate_EOR(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_xor(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_EOR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_xor(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_EOR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_xor(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_EOR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_xor(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}

uint8_t absoluteY_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}

uint8_t indirectX_EOR(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_xor(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_EOR(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_xor(registers, data);
    return page_crossed;
}

//*****************************************************************************
// INC
//*****************************************************************************
uint8_t zeropage_INC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_increment(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_INC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_increment(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_INC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_increment(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_INC(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_increment(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// INX and INY
//*****************************************************************************
uint8_t implied_INX(cpu_registers* registers) {
    base_increment(registers, &(registers->X));
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_INY(cpu_registers* registers) {
    base_increment(registers, &(registers->Y));
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// JMP, JSR
//*****************************************************************************
uint8_t absolute_JMP(cpu_registers* registers) {
    registers->PC = calc_absolute_address(registers);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_JSR(cpu_registers* registers) {
    push_PC_onto_stack(registers, registers->PC + 1);
    registers->PC = calc_absolute_address(registers);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// LDA
//*****************************************************************************
uint8_t immediate_LDA(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_LDA(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_LDA(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_LDA(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

uint8_t absoluteY_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

uint8_t indirectX_LDA(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

//*****************************************************************************
// LDX
//*****************************************************************************
uint8_t immediate_LDX(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->X), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_LDX(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->X), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageY_LDX(cpu_registers* registers) {
    uint8_t data = fetch_zeropageY(registers);
    base_load_register(registers, &(registers->X), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_LDX(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->X), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteY_LDX(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_load_register(registers, &(registers->X), data);
    return page_crossed;
}

//*****************************************************************************
// LDY
//*****************************************************************************
uint8_t immediate_LDY(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->Y), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_LDY(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->Y), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_LDY(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_load_register(registers, &(registers->Y), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_LDY(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->Y), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_LDY(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_load_register(registers, &(registers->Y), data);
    return page_crossed;
}

//*****************************************************************************
// LSR
//*****************************************************************************
uint8_t accumulator_LSR(cpu_registers* registers) {
    base_shift_right(registers, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_LSR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_shift_right(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_LSR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_shift_right(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_LSR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_shift_right(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_LSR(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_shift_right(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_NOP(cpu_registers* registers) {
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// ORA
//*****************************************************************************
uint8_t immediate_ORA(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_or(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ORA(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_or(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ORA(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_or(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ORA(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_or(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

uint8_t absoluteY_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

uint8_t indirectX_ORA(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_or(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

//*****************************************************************************
// Stack instructions
//*****************************************************************************
uint8_t implied_PHA(cpu_registers* registers) {
    push_stack(registers, registers->A);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_PHP(cpu_registers* registers) {
    // PHP sets these two bits for some reason
    uint8_t PHP_special_flags = 0x30;
    push_stack(registers, registers->flags | PHP_special_flags);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_PLA(cpu_registers* registers) {
    registers->A = pop_stack(registers);
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_PLP(cpu_registers* registers) {
    // Need to preserve bits 4/5 from original flags and get other bits from stack
    uint8_t preserve_bits = registers->flags & 0x30;
    uint8_t stack_bits = pop_stack(registers) & 0xCF;
    registers->flags = preserve_bits | stack_bits;
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// ROL
//*****************************************************************************
uint8_t accumulator_ROL(cpu_registers* registers) {
    base_rotate_left(registers, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ROL(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_rotate_left(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ROL(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_rotate_left(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ROL(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_rotate_left(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ROL(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_rotate_left(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// ROR
//*****************************************************************************
uint8_t accumulator_ROR(cpu_registers* registers) {
    base_rotate_right(registers, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ROR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_rotate_right(registers, &data);
    write_zeropage(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ROR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_rotate_right(registers, &data);
    write_zeropageX(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ROR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_rotate_right(registers, &data);
    write_absolute(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ROR(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_rotate_right(registers, &data);
    write_absoluteX(registers, data);
    return ZERO_EXTRA_CYCLES;
}
//*****************************************************************************
// Return instructions
//*****************************************************************************
uint8_t implied_RTI(cpu_registers* registers) {
    registers->flags = pop_stack(registers);
    pop_PC_off_stack(registers);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_RTS(cpu_registers* registers) {
    pop_PC_off_stack(registers);
    return ZERO_EXTRA_CYCLES;
}


//*****************************************************************************
// SBC
//*****************************************************************************
uint8_t immediate_SBC(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_SBC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_SBC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_SBC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

uint8_t absoluteY_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

uint8_t indirectX_SBC(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

//*****************************************************************************
// Flag setters
//*****************************************************************************
uint8_t implied_SEC(cpu_registers* registers) {
    set_cpu_flag(registers, CARRY_FLAG);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_SED(cpu_registers* registers) {
    set_cpu_flag(registers, DECIMAL_FLAG);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_SEI(cpu_registers* registers) {
    set_cpu_flag(registers, INTERRUPT_FLAG);
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// STA
//*****************************************************************************
uint8_t zeropage_STA(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_STA(cpu_registers* registers) {
    uint8_t address = calc_zeropageX_address(registers);
    base_store(address, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_STA(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_STA(cpu_registers* registers) {
    uint16_t address = calc_absoluteX_address(registers);
    base_store(address, &(registers->A));
    return 0;
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteY_STA(cpu_registers* registers) {
    uint16_t address = calc_absoluteY_address(registers);
    base_store(address, &(registers->A));
    return 0;
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectX_STA(cpu_registers* registers) {
    uint16_t address = calc_indirectX_address(registers);
    base_store(address, &(registers->A));
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_STA(cpu_registers* registers) {
    uint16_t address = calc_indirectY_address(registers);
    base_store(address, &(registers->A));
    return 0;
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// STX
//*****************************************************************************
uint8_t zeropage_STX(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->X));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageY_STX(cpu_registers* registers) {
    uint8_t address = calc_zeropageY_address(registers);
    base_store(address, &(registers->X));
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_STX(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->X));
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// STY
//*****************************************************************************
uint8_t zeropage_STY(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->Y));
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_STY(cpu_registers* registers) {
    uint8_t address = calc_zeropageX_address(registers);
    base_store(address, &(registers->Y));
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_STY(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->Y));
    return ZERO_EXTRA_CYCLES;
}

//*****************************************************************************
// Transfer instructions
//*****************************************************************************
uint8_t implied_TAX(cpu_registers* registers) {
    registers->X = registers->A;
    determine_zero_flag(registers, registers->X);
    determine_negative_flag(registers, registers->X);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_TAY(cpu_registers* registers) {
    registers->Y = registers->A;
    determine_zero_flag(registers, registers->Y);
    determine_negative_flag(registers, registers->Y);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_TSX(cpu_registers* registers) {
    registers->X = registers->S;
    determine_zero_flag(registers, registers->X);
    determine_negative_flag(registers, registers->X);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_TXA(cpu_registers* registers) {
    registers->A = registers->X;
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_TXS(cpu_registers* registers) {
    registers->S = registers->X;
    return ZERO_EXTRA_CYCLES;
}

uint8_t implied_TYA(cpu_registers* registers) {
    registers->A = registers->Y;
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
    return ZERO_EXTRA_CYCLES;
}
