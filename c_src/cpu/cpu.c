//*****************************************************************************
// Filename: cpu.c
// Author: Fisherman166
//
// Implements the top level CPU functions
//
//****************************************************************************

#include <stdlib.h>
#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_basic_operations.h"
#include "illegal_opcodes.h"
#include "cpu_decode_logic.h"
#include "RAM.h"
#include "ppu.h"

#define IMM 1
#define ZRP 2
#define ZPX 3
#define ZPY 4
#define ABS 5
#define ABX 6
#define ABY 7
#define INX 8
#define INY 9
#define REL 10
#define IMP 11
#define ACC 12
#define IND 13
#define ERR 0xFF

static bool odd_cpu_cycle = false;
static uint16_t DMA_cycle_count = 0;

static const uint8_t instruction_byte_length[] = { //Opcode included
  //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
    2,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 0
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 1
    3,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 2
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 3
    2,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 4
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 5
    1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 6
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 7
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// 8
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 9
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// A
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// B
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// C
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// D
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// E
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// F
};

static const uint8_t instruction_cycle_length[] = {
  //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
    7,6,1,8,3,3,5,5,3,2,2,2,4,4,6,6,// 0
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 1
    6,6,1,8,3,3,5,5,4,2,2,2,4,4,6,6,// 2
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 3
    6,6,1,8,3,3,5,5,3,2,2,2,3,4,6,6,// 4
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 5
    6,6,1,8,3,3,5,5,4,2,2,2,5,4,6,6,// 6
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 7
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// 8
    2,6,1,6,4,4,4,4,2,5,2,3,3,5,3,5,// 9
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// A
    2,5,1,5,4,4,4,4,2,4,2,4,4,4,4,4,// B
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// C
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// D
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// E
    2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// F
};

static const char* instruction_text[] = {
    "BRK", "ORA", "ERR", "ERR", "*NOP", "ORA", "ASL", "ERR", "PHP", "ORA", "ASL", "ERR", "*NOP", "ORA", "ASL", "ERR", 
    "BPL", "ORA", "ERR", "ERR", "*NOP", "ORA", "ASL", "ERR", "CLC", "ORA", "ERR", "ERR", "*NOP", "ORA", "ASL", "ERR", 
    "JSR", "AND", "ERR", "ERR", "BIT", "AND", "ROL", "ERR", "PLP", "AND", "ROL", "ERR", "BIT", "AND", "ROL", "ERR", 
    "BMI", "AND", "ERR", "ERR", "*NOP", "AND", "ROL", "ERR", "SEC", "AND", "ERR", "ERR", "*NOP", "AND", "ROL", "ERR", 
    "RTI", "EOR", "ERR", "ERR", "*NOP", "EOR", "LSR", "ERR", "PHA", "EOR", "LSR", "ERR", "JMP", "EOR", "LSR", "ERR", 
    "BVC", "EOR", "ERR", "ERR", "*NOP", "EOR", "LSR", "ERR", "CLI", "EOR", "ERR", "ERR", "*NOP", "EOR", "LSR", "ERR", 
    "RTS", "ADC", "ERR", "ERR", "*NOP", "ADC", "ROR", "ERR", "PLA", "ADC", "ROR", "ERR", "JMP", "ADC", "ROR", "ERR", 
    "BVS", "ADC", "ERR", "ERR", "*NOP", "ADC", "ROR", "ERR", "SEI", "ADC", "ERR", "ERR", "*NOP", "ADC", "ROR", "ERR", 
    "*NOP", "STA", "*NOP", "ERR", "STY", "STA", "STX", "ERR", "DEY", "*NOP", "TXA", "ERR", "STY", "STA", "STX", "ERR", 
    "BCC", "STA", "ERR", "ERR", "STY", "STA", "STX", "ERR", "TYA", "STA", "TXS", "ERR", "ERR", "STA", "ERR", "ERR", 
    "LDY", "LDA", "LDX", "ERR", "LDY", "LDA", "LDX", "ERR", "TAY", "LDA", "TAX", "ERR", "LDY", "LDA", "LDX", "ERR", 
    "BCS", "LDA", "ERR", "ERR", "LDY", "LDA", "LDX", "ERR", "CLV", "LDA", "TSX", "ERR", "LDY", "LDA", "LDX", "ERR", 
    "CPY", "CMP", "*NOP", "ERR", "CPY", "CMP", "DEC", "ERR", "INY", "CMP", "DEX", "ERR", "CPY", "CMP", "DEC", "ERR", 
    "BNE", "CMP", "ERR", "ERR", "*NOP", "CMP", "DEC", "ERR", "CLD", "CMP", "ERR", "ERR", "*NOP", "CMP", "DEC", "ERR", 
    "CPX", "SBC", "*NOP", "ERR", "CPX", "SBC", "INC", "ERR", "INX", "SBC", "NOP", "ERR", "CPX", "SBC", "INC", "ERR", 
    "BEQ", "SBC", "ERR", "ERR", "*NOP", "SBC", "INC", "ERR", "SED", "SBC", "ERR", "ERR", "*NOP", "SBC", "INC", "ERR", 
};

static const uint8_t instruction_addressing_mode[] = {
    IMP, INX, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
    ABS, INX, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
    IMP, INX, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
    IMP, INX, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, IND, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
    IMP, INX, IMP, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMP, IMP, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPY, ERR, IMP, ABY, IMP, ERR, ERR, ABX, ERR, ERR, 
    IMM, INX, IMM, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPY, ERR, IMP, ABY, IMP, ERR, ABX, ABX, ABY, ERR, 
    IMM, INX, IMP, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
    IMM, INX, IMP, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ABX, ABX, ABX, ERR, 
};

static uint8_t (*instructions[]) (cpu_registers*) = {
    &implied_BRK, &indirectX_ORA, NULL, NULL, &zeropage_NOP, &zeropage_ORA, &zeropage_ASL, NULL, &implied_PHP, &immediate_ORA, &accumulator_ASL, NULL, &absolute_NOP, &absolute_ORA, &absolute_ASL, NULL, 
    &relative_BPL, &indirectY_ORA, NULL, NULL, &zeropageX_NOP, &zeropageX_ORA, &zeropageX_ASL, NULL, &implied_CLC, &absoluteY_ORA, NULL, NULL, &absoluteX_NOP, &absoluteX_ORA, &absoluteX_ASL, NULL, 
    &absolute_JSR, &indirectX_AND, NULL, NULL, &zeropage_BIT, &zeropage_AND, &zeropage_ROL, NULL, &implied_PLP, &immediate_AND, &accumulator_ROL, NULL, &absolute_BIT, &absolute_AND, &absolute_ROL, NULL, 
    &relative_BMI, &indirectY_AND, NULL, NULL, &zeropageX_NOP, &zeropageX_AND, &zeropageX_ROL, NULL, &implied_SEC, &absoluteY_AND, NULL, NULL, &absoluteX_NOP, &absoluteX_AND, &absoluteX_ROL, NULL, 
    &implied_RTI, &indirectX_EOR, NULL, NULL, &zeropage_NOP, &zeropage_EOR, &zeropage_LSR, NULL, &implied_PHA, &immediate_EOR, &accumulator_LSR, NULL, &absolute_JMP, &absolute_EOR, &absolute_LSR, NULL, 
    &relative_BVC, &indirectY_EOR, NULL, NULL, &zeropageX_NOP, &zeropageX_EOR, &zeropageX_LSR, NULL, &implied_CLI, &absoluteY_EOR, NULL, NULL, &absoluteX_NOP, &absoluteX_EOR, &absoluteX_LSR, NULL, 
    &implied_RTS, &indirectX_ADC, NULL, NULL, &zeropage_NOP, &zeropage_ADC, &zeropage_ROR, NULL, &implied_PLA, &immediate_ADC, &accumulator_ROR, NULL, &indirect_JMP, &absolute_ADC, &absolute_ROR, NULL, 
    &relative_BVS, &indirectY_ADC, NULL, NULL, &zeropageX_NOP, &zeropageX_ADC, &zeropageX_ROR, NULL, &implied_SEI, &absoluteY_ADC, NULL, NULL, &absoluteX_NOP, &absoluteX_ADC, &absoluteX_ROR, NULL, 
    &implied_NOP, &indirectX_STA, &implied_NOP, NULL, &zeropage_STY, &zeropage_STA, &zeropage_STX, NULL, &implied_DEY, &implied_NOP, &implied_TXA, NULL, &absolute_STY, &absolute_STA, &absolute_STX, NULL, 
    &relative_BCC, &indirectY_STA, NULL, NULL, &zeropageX_STY, &zeropageX_STA, &zeropageY_STX, NULL, &implied_TYA, &absoluteY_STA, &implied_TXS, NULL, NULL, &absoluteX_STA, NULL, NULL, 
    &immediate_LDY, &indirectX_LDA, &immediate_LDX, NULL, &zeropage_LDY, &zeropage_LDA, &zeropage_LDX, NULL, &implied_TAY, &immediate_LDA, &implied_TAX, NULL, &absolute_LDY, &absolute_LDA, &absolute_LDX, NULL, 
    &relative_BCS, &indirectY_LDA, NULL, NULL, &zeropageX_LDY, &zeropageX_LDA, &zeropageY_LDX, NULL, &implied_CLV, &absoluteY_LDA, &implied_TSX, NULL, &absoluteX_LDY, &absoluteX_LDA, &absoluteY_LDX, NULL, 
    &immediate_CPY, &indirectX_CMP, &implied_NOP, NULL, &zeropage_CPY, &zeropage_CMP, &zeropage_DEC, NULL, &implied_INY, &immediate_CMP, &implied_DEX, NULL, &absolute_CPY, &absolute_CMP, &absolute_DEC, NULL, 
    &relative_BNE, &indirectY_CMP, NULL, NULL, &zeropageX_NOP, &zeropageX_CMP, &zeropageX_DEC, NULL, &implied_CLD, &absoluteY_CMP, NULL, NULL, &absoluteX_NOP, &absoluteX_CMP, &absoluteX_DEC, NULL, 
    &immediate_CPX, &indirectX_SBC, &implied_NOP, NULL, &zeropage_CPX, &zeropage_SBC, &zeropage_INC, NULL, &implied_INX, &immediate_SBC, &implied_NOP, NULL, &absolute_CPX, &absolute_SBC, &absolute_INC, NULL, 
    &relative_BEQ, &indirectY_SBC, NULL, NULL, &zeropageX_NOP, &zeropageX_SBC, &zeropageX_INC, NULL, &implied_SED, &absoluteY_SBC, NULL, NULL, &absoluteX_NOP, &absoluteX_SBC, &absoluteX_INC, NULL, 
};

//*****************************************************************************
// Local functions
//*****************************************************************************
static uint16_t fetch_reset_vector() {
    const uint16_t reset_vector_low_byte = 0xFFFC;
    const uint16_t reset_vector_high_byte = 0xFFFD;
    return (read_RAM(reset_vector_high_byte) << 8) | read_RAM(reset_vector_low_byte);
}

static void increment_PC(cpu_registers* registers) {
    registers->PC++;
}

static bool is_JSR_or_JMP(uint8_t opcode) {
    const uint8_t JSR_opcode = 0x20;
    const uint8_t JMP_abs_opcode = 0x4C;
    const uint8_t JMP_indir_opcode = 0x6C;
    if(opcode == JSR_opcode) return true;
    if(opcode == JMP_abs_opcode) return true;
    if(opcode == JMP_indir_opcode) return true;
    return false;
}

static bool is_RTS(uint8_t opcode) {
    if(opcode == 0x60) return true;
    return false;
}

static bool is_RTI(uint8_t opcode) {
    if(opcode == 0x40) return true;
    return false;
}

static void increment_PC_instruction_length(cpu_registers* registers, uint8_t opcode) {
    // Have to subtract 1 to account for the increment done after the
    // opcode fetch
    registers->PC += instruction_byte_length[opcode] - 1;

    // RTS cancels out the PC increment done after the opcode fetch. So add 1 here
    // to get the right return address
    if(is_RTS(opcode)) increment_PC(registers);
}

static uint8_t execute_instruction(cpu_registers* registers, uint8_t opcode) {
    if(instructions[opcode] == NULL) {
        printf("ERROR: Illegal opcode 0x%X attempted to execute. Exiting\n", opcode);
        exit(-1);
    }
    uint8_t extra_cycles = (*instructions[opcode])(registers);
    return extra_cycles;
}

//*****************************************************************************
// Debug file functions
//*****************************************************************************
static FILE* cpu_logfile = NULL;

static void print_accumulator_debug_info(cpu_registers* registers, uint8_t opcode) {
    fprintf(cpu_logfile, "%8s", " ");
    fprintf(cpu_logfile, "%3s A", instruction_text[opcode]);
    fprintf(cpu_logfile, "%27s", " ");
}

static void print_immediate_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t data_read = debug_read_RAM(registers->PC);
    fprintf(cpu_logfile, " %02X", data_read);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "#$%02X", data_read);
    fprintf(cpu_logfile, "%24s", " ");
}

static void print_implied_debug_info(cpu_registers* registers, uint8_t opcode) {
    fprintf(cpu_logfile, "%8s", " ");
    fprintf(cpu_logfile, "%3s", instruction_text[opcode]);
    fprintf(cpu_logfile, "%29s", " ");
}

static void print_zeropage_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address  = debug_read_RAM(registers->PC);
    uint8_t data_read = debug_read_RAM(zeropage_address);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X = %02X", zeropage_address, data_read);
    fprintf(cpu_logfile, "%20s", " ");
}

static void print_zeropageX_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address  = debug_read_RAM(registers->PC);
    uint8_t zeropageX_address = calc_zeropageX_address(registers);
    uint8_t data_read = debug_read_RAM(zeropageX_address);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X,X @ %02X = %02X", zeropage_address, zeropageX_address, 
            data_read);
    fprintf(cpu_logfile, "%13s", " ");
}

static void print_zeropageY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address = debug_read_RAM(registers->PC);
    uint8_t zeropageY_address = calc_zeropageY_address(registers);
    uint8_t data_read = debug_read_RAM(zeropageY_address);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X,Y @ %02X = %02X", zeropage_address, zeropageY_address, 
            data_read);
    fprintf(cpu_logfile, "%13s", " ");
}

static void print_absolute_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = debug_read_RAM(registers->PC);
    uint8_t high_address_byte = debug_read_RAM(registers->PC + 1);
    uint16_t absolute_address = calc_absolute_address(registers);
    uint8_t data_read = debug_read_RAM(absolute_address);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    if(is_JSR_or_JMP(opcode)) {
        fprintf(cpu_logfile, "$%04X", absolute_address);
        fprintf(cpu_logfile, "%23s", " ");
    }
    else {
        fprintf(cpu_logfile, "$%04X = %02X", absolute_address, data_read);
        fprintf(cpu_logfile, "%18s", " ");
    }
}

static void print_absoluteX_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = debug_read_RAM(registers->PC);
    uint8_t high_address_byte = debug_read_RAM(registers->PC + 1);
    uint16_t absoluteX_address = calc_absoluteX_address(registers);
    uint8_t data_read = debug_read_RAM(absoluteX_address);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X%02X,X @ %04X = %02X", high_address_byte, low_address_byte,
            absoluteX_address, data_read);
    fprintf(cpu_logfile, "%9s", " ");
}

static void print_absoluteY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = debug_read_RAM(registers->PC);
    uint8_t high_address_byte = debug_read_RAM(registers->PC + 1);
    uint16_t absoluteY_address = calc_absoluteY_address(registers);
    uint8_t data_read = debug_read_RAM(absoluteY_address);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X%02X,Y @ %04X = %02X", high_address_byte, low_address_byte,
            absoluteY_address, data_read);
    fprintf(cpu_logfile, "%9s", " ");
}

static void print_indirectX_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address = debug_read_RAM(registers->PC);
    uint16_t indirect_address = calc_indirectX_address(registers);
    uint8_t data_read = debug_read_RAM(indirect_address);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "($%02X,X) @ %02X = %04X = %02X", zeropage_address,
            (zeropage_address + registers->X) & BYTE_MASK, indirect_address,
            data_read);
    fprintf(cpu_logfile, "%4s", " ");
}

static void print_indirectY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address = debug_read_RAM(registers->PC);
    uint16_t indirect_address = calc_indirectY_address(registers);
    uint8_t data_read = debug_read_RAM(indirect_address);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "($%02X),Y = %04X @ %04X = %02X", zeropage_address,
            (indirect_address - registers->Y) & 0xFFFF, indirect_address, data_read);
    fprintf(cpu_logfile, "%2s", " ");
}

static void print_relative_debug_info(cpu_registers* registers, uint8_t opcode) {
    char offset = debug_read_RAM(registers->PC) & BYTE_MASK;
    uint16_t target_address = registers->PC + offset + instruction_byte_length[opcode] - 1;
    fprintf(cpu_logfile, " %02X", offset & BYTE_MASK);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%04X", target_address);
    fprintf(cpu_logfile, "%23s", " ");
}

static void print_indirect_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint16_t target_address;
    uint8_t low_address_byte = debug_read_RAM(registers->PC);
    uint8_t high_address_byte = debug_read_RAM(registers->PC + 1);
    uint16_t indirect_address = calc_absolute_address(registers);
    // Bug where the MSB is is taken from xx00 instead of the next page if on
    // page boundry
    if((indirect_address & 0xFF) == 0xFF) 
        target_address = (debug_read_RAM(indirect_address & 0xFF00) << 8) | debug_read_RAM(indirect_address);
    else
        target_address = (debug_read_RAM(indirect_address + 1) << 8) | debug_read_RAM(indirect_address);

    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "($%04X) = %04X", indirect_address, target_address);
    fprintf(cpu_logfile, "%14s", " ");
}

static void print_common_debug_info(cpu_registers* registers) {
    uint16_t scanline = get_scanline();
    fprintf(cpu_logfile, "A:%02X ", registers->A);
    fprintf(cpu_logfile, "X:%02X ", registers->X);
    fprintf(cpu_logfile, "Y:%02X ", registers->Y);
    fprintf(cpu_logfile, "P:%02X ", registers->flags);
    fprintf(cpu_logfile, "SP:%02X ", registers->S);
    fprintf(cpu_logfile, "CYC:%3d ", get_dot());
    if(scanline == 261)
        fprintf(cpu_logfile, "SL:%d", -1);
    else
        fprintf(cpu_logfile, "SL:%d", scanline);
    fprintf(cpu_logfile, "\n");
}

static void print_debug_info(cpu_registers* registers, uint8_t opcode) {
    if(cpu_logfile == NULL) printf("ERROR: cpu_logfile not open when attempting to write to file\n");

    fprintf(cpu_logfile, "%04X  ", registers->PC - 1); // Decrement to get original PC
    fprintf(cpu_logfile, "%02X", opcode);

    if(instruction_addressing_mode[opcode] == ACC) print_accumulator_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == IMM) print_immediate_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == IMP) print_implied_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ZRP) print_zeropage_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ZPX) print_zeropageX_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ZPY) print_zeropageY_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ABS) print_absolute_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ABX) print_absoluteX_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == ABY) print_absoluteY_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == INX) print_indirectX_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == INY) print_indirectY_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == REL) print_relative_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == IND) print_indirect_debug_info(registers, opcode);
    else printf("ERROR: Addressing mode %d not recogonized for opcode 0x%02X\n",
                instruction_addressing_mode[opcode], opcode);

    print_common_debug_info(registers);
}

static uint8_t execute_NMI(cpu_registers* registers) {
    // NMI clears bit 4 and sets bit 5 for flags push
    uint8_t NMI_flags = registers->flags | 0x20;
    NMI_flags &= ~0x10;
    push_PC_onto_stack(registers, registers->PC);
    push_stack(registers, NMI_flags);
    registers->PC = (read_RAM(0xFFFB) << 8) | read_RAM(0xFFFA);
    set_cpu_flag(registers, INTERRUPT_FLAG);
    return 7;
}

static bool DMA_is_executing() {
    if(DMA_cycle_count) return true;
    else return false;
}

// FIXME - Just a stub for now that does no real work
static uint8_t execute_DMA() {
    DMA_cycle_count -= 1;
    return 1;
}


//*****************************************************************************
// Public functions
//*****************************************************************************
void open_cpu_debug_logfile() {
    const char* logfile_name = "cpu_instrunctions.log";
    cpu_logfile = fopen(logfile_name, "w");
    if(cpu_logfile == NULL) {
        printf("ERROR: Failed to open %s.\n", logfile_name);
    }
}

void close_cpu_debug_logfile() {
    fclose(cpu_logfile);
}

void cold_boot_init(cpu_registers* registers) {
    const uint8_t initial_A = 0;
    const uint8_t initial_X = 0;
    const uint8_t initial_Y = 0;
    const uint8_t initial_S = 0xFD;
    const uint8_t initial_flags = 0x24;

    //To run NESTEST
    uint16_t initial_PC = 0xC000;
    //uint16_t initial_PC = fetch_reset_vector();
    init_cpu_registers(registers, initial_A, initial_X, initial_Y, initial_PC,
                       initial_S, initial_flags);
}

uint8_t execute_interpreter_cycle(cpu_registers* registers, bool nmi_flag) {
    static bool previous_nmi_flag = false;
    uint8_t cpu_cycles_executed = 0;
    uint8_t extra_cycles= 0;

    if(DMA_is_executing()) cpu_cycles_executed = execute_DMA();
    else {
        uint8_t opcode = fetch_opcode(registers);
        increment_PC(registers);

        if(nmi_flag && !previous_nmi_flag) cpu_cycles_executed = execute_NMI(registers);
        else {
            #ifdef DEBUG
                print_debug_info(registers, opcode);
            #endif
            extra_cycles = execute_instruction(registers, opcode);
            cpu_cycles_executed = instruction_cycle_length[opcode] + extra_cycles;
        }
        // Do not increment PC if we jump because it will skip the first instruction at least
        if(!is_JSR_or_JMP(opcode) && !is_RTI(opcode)) increment_PC_instruction_length(registers, opcode);
    }

    previous_nmi_flag = nmi_flag;
    odd_cpu_cycle = cpu_cycles_executed & 0x1;

    return cpu_cycles_executed;
}

void start_DMA(uint8_t DMA_address) {
    // uint16_t full_start_address = 0 | (DMA_address << 8);
    if(odd_cpu_cycle) DMA_cycle_count = 515;
    else DMA_cycle_count = 514;
}
