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

static void pop_PC_off_stack(cpu_registers* registers) {
    uint8_t high_byte = pop_stack(registers);
    uint8_t low_byte = pop_stack(registers);
    registers->PC = (high_byte << 8) | low_byte;
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

//*****************************************************************************
// INC
//*****************************************************************************
void zeropage_INC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_increment(registers, &data);
    write_zeropage(registers, data);
}

void zeropageX_INC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_increment(registers, &data);
    write_zeropageX(registers, data);
}

void absolute_INC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_increment(registers, &data);
    write_absolute(registers, data);
}

void absoluteX_INC(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_increment(registers, &data);
    write_absoluteX(registers, data);
}

//*****************************************************************************
// INX and INY
//*****************************************************************************
void implied_INX(cpu_registers* registers) {
    base_increment(registers, &(registers->X));
}

void implied_INY(cpu_registers* registers) {
    base_increment(registers, &(registers->Y));
}

//*****************************************************************************
// JMP, JSR
//*****************************************************************************
void absolute_JMP(cpu_registers* registers) {
    registers->PC = calc_absolute_address(registers);
}

void absolute_JSR(cpu_registers* registers) {
    push_PC_onto_stack(registers, registers->PC + 1);
    registers->PC = calc_absolute_address(registers);
}

//*****************************************************************************
// LDA
//*****************************************************************************
void immediate_LDA(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->A), data);
}

void zeropage_LDA(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->A), data);
}

void zeropageX_LDA(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_load_register(registers, &(registers->A), data);
}

void absolute_LDA(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->A), data);
}

bool absoluteX_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

bool absoluteY_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

void indirectX_LDA(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_load_register(registers, &(registers->A), data);
}

bool indirectY_LDA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

//*****************************************************************************
// LDX
//*****************************************************************************
void immediate_LDX(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->X), data);
}

void zeropage_LDX(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->X), data);
}

void zeropageY_LDX(cpu_registers* registers) {
    uint8_t data = fetch_zeropageY(registers);
    base_load_register(registers, &(registers->X), data);
}

void absolute_LDX(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->X), data);
}

bool absoluteY_LDX(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_load_register(registers, &(registers->X), data);
    return page_crossed;
}

//*****************************************************************************
// LDY
//*****************************************************************************
void immediate_LDY(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_load_register(registers, &(registers->Y), data);
}

void zeropage_LDY(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->Y), data);
}

void zeropageX_LDY(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_load_register(registers, &(registers->Y), data);
}

void absolute_LDY(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->Y), data);
}

bool absoluteX_LDY(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_load_register(registers, &(registers->Y), data);
    return page_crossed;
}

//*****************************************************************************
// LSR
//*****************************************************************************
void accumulator_LSR(cpu_registers* registers) {
    base_shift_right(registers, &(registers->A));
}

void zeropage_LSR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_shift_right(registers, &data);
    write_zeropage(registers, data);
}

void zeropageX_LSR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_shift_right(registers, &data);
    write_zeropageX(registers, data);
}

void absolute_LSR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_shift_right(registers, &data);
    write_absolute(registers, data);
}

void absoluteX_LSR(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_shift_right(registers, &data);
    write_absoluteX(registers, data);
}

void implied_NOP() {
    return;
}

//*****************************************************************************
// ORA
//*****************************************************************************
void immediate_ORA(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_or(registers, data);
}

void zeropage_ORA(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_or(registers, data);
}

void zeropageX_ORA(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_or(registers, data);
}

void absolute_ORA(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_or(registers, data);
}

bool absoluteX_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

bool absoluteY_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

void indirectX_ORA(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_or(registers, data);
}

bool indirectY_ORA(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_or(registers, data);
    return page_crossed;
}

//*****************************************************************************
// Stack instructions
//*****************************************************************************
void implied_PHA(cpu_registers* registers) {
    push_stack(registers, registers->A);
}

void implied_PHP(cpu_registers* registers) {
    push_stack(registers, registers->flags);
}

void implied_PLA(cpu_registers* registers) {
    registers->A = pop_stack(registers);
    determine_zero_flag(registers, registers->A);
    determine_negative_flag(registers, registers->A);
}

void implied_PLP(cpu_registers* registers) {
    registers->flags = pop_stack(registers);
}

//*****************************************************************************
// ROL
//*****************************************************************************
void accumulator_ROL(cpu_registers* registers) {
    base_rotate_left(registers, &(registers->A));
}

void zeropage_ROL(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_rotate_left(registers, &data);
    write_zeropage(registers, data);
}

void zeropageX_ROL(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_rotate_left(registers, &data);
    write_zeropageX(registers, data);
}

void absolute_ROL(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_rotate_left(registers, &data);
    write_absolute(registers, data);
}

void absoluteX_ROL(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_rotate_left(registers, &data);
    write_absoluteX(registers, data);
}

//*****************************************************************************
// ROR
//*****************************************************************************
void accumulator_ROR(cpu_registers* registers) {
    base_rotate_right(registers, &(registers->A));
}

void zeropage_ROR(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_rotate_right(registers, &data);
    write_zeropage(registers, data);
}

void zeropageX_ROR(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_rotate_right(registers, &data);
    write_zeropageX(registers, data);
}

void absolute_ROR(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_rotate_right(registers, &data);
    write_absolute(registers, data);
}

void absoluteX_ROR(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL);
    base_rotate_right(registers, &data);
    write_absoluteX(registers, data);
}
//*****************************************************************************
// Return instructions
//*****************************************************************************
void implied_RTI(cpu_registers* registers) {
    registers->flags = pop_stack(registers);
    pop_PC_off_stack(registers);
}

void implied_RTS(cpu_registers* registers) {
    pop_PC_off_stack(registers);
}


//*****************************************************************************
// SBC
//*****************************************************************************
void immediate_SBC(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_subtract(registers, data);
}

void zeropage_SBC(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_subtract(registers, data);
}

void zeropageX_SBC(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers);
    base_subtract(registers, data);
}

void absolute_SBC(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_subtract(registers, data);
}

bool absoluteX_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteX(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

bool absoluteY_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

void indirectX_SBC(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_subtract(registers, data);
}

bool indirectY_SBC(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_subtract(registers, data);
    return page_crossed;
}

//*****************************************************************************
// Flag setters
//*****************************************************************************
void implied_SEC(cpu_registers* registers) {
    set_cpu_flag(registers, CARRY_FLAG);
}

void implied_SED(cpu_registers* registers) {
    set_cpu_flag(registers, DECIMAL_FLAG);
}

void implied_SEI(cpu_registers* registers) {
    set_cpu_flag(registers, INTERRUPT_FLAG);
}

//*****************************************************************************
// STA
//*****************************************************************************
void zeropage_STA(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->A));
}

void zeropageX_STA(cpu_registers* registers) {
    uint8_t address = calc_zeropageX_address(registers);
    base_store(address, &(registers->A));
}

void absolute_STA(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->A));
}

bool absoluteX_STA(cpu_registers* registers) {
    uint16_t address = calc_absoluteX_address(registers);
    base_store(address, &(registers->A));
    return 0;
}

bool absoluteY_STA(cpu_registers* registers) {
    uint16_t address = calc_absoluteY_address(registers);
    base_store(address, &(registers->A));
    return 0;
}

void indirectX_STA(cpu_registers* registers) {
    uint16_t address = calc_indirectX_address(registers);
    base_store(address, &(registers->A));
}

bool indirectY_STA(cpu_registers* registers) {
    uint16_t address = calc_indirectY_address(registers);
    base_store(address, &(registers->A));
    return 0;
}

//*****************************************************************************
// STX
//*****************************************************************************
void zeropage_STX(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->X));
}

void zeropageY_STX(cpu_registers* registers) {
    uint8_t address = calc_zeropageY_address(registers);
    base_store(address, &(registers->X));
}

void absolute_STX(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->X));
}

//*****************************************************************************
// STY
//*****************************************************************************
void zeropage_STY(cpu_registers* registers) {
    uint8_t address = calc_zeropage_address(registers);
    base_store(address, &(registers->Y));
}

void zeropageX_STY(cpu_registers* registers) {
    uint8_t address = calc_zeropageX_address(registers);
    base_store(address, &(registers->Y));
}

void absolute_STY(cpu_registers* registers) {
    uint16_t address = calc_absolute_address(registers);
    base_store(address, &(registers->Y));
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
