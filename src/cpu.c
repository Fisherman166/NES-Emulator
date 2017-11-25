//*****************************************************************************
// Filename: cpu.c
// Author: Fisherman166
//
// Implements the top level CPU functions
//
//****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_basic_operations.h"
#include "illegal_opcodes.h"
#include "cpu_decode_logic.h"
#include "RAM.h"
#include "ppu.h"
#include "sprites.h"

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
static uint16_t DMA_RAM_address = 0;

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

#ifdef DEBUG
static const char* instruction_text[] = {
    "BRK", "ORA", "ERR", "*SLO", "*NOP", "ORA", "ASL", "*SLO", "PHP", "ORA", "ASL", "ERR", "*NOP", "ORA", "ASL", "*SLO", 
    "BPL", "ORA", "ERR", "*SLO", "*NOP", "ORA", "ASL", "*SLO", "CLC", "ORA", "*NOP", "*SLO", "*NOP", "ORA", "ASL", "*SLO", 
    "JSR", "AND", "ERR", "*RLA", "BIT", "AND", "ROL", "*RLA", "PLP", "AND", "ROL", "ERR", "BIT", "AND", "ROL", "*RLA", 
    "BMI", "AND", "ERR", "*RLA", "*NOP", "AND", "ROL", "*RLA", "SEC", "AND", "*NOP", "*RLA", "*NOP", "AND", "ROL", "*RLA", 
    "RTI", "EOR", "ERR", "*SRE", "*NOP", "EOR", "LSR", "*SRE", "PHA", "EOR", "LSR", "ERR", "JMP", "EOR", "LSR", "*SRE", 
    "BVC", "EOR", "ERR", "*SRE", "*NOP", "EOR", "LSR", "*SRE", "CLI", "EOR", "*NOP", "*SRE", "*NOP", "EOR", "LSR", "*SRE", 
    "RTS", "ADC", "ERR", "*RRA", "*NOP", "ADC", "ROR", "*RRA", "PLA", "ADC", "ROR", "ERR", "JMP", "ADC", "ROR", "*RRA", 
    "BVS", "ADC", "ERR", "*RRA", "*NOP", "ADC", "ROR", "*RRA", "SEI", "ADC", "*NOP", "*RRA", "*NOP", "ADC", "ROR", "*RRA", 
    "*NOP", "STA", "*NOP", "*SAX", "STY", "STA", "STX", "*SAX", "DEY", "*NOP", "TXA", "ERR", "STY", "STA", "STX", "*SAX", 
    "BCC", "STA", "ERR", "ERR", "STY", "STA", "STX", "*SAX", "TYA", "STA", "TXS", "ERR", "ERR", "STA", "ERR", "ERR", 
    "LDY", "LDA", "LDX", "*LAX", "LDY", "LDA", "LDX", "*LAX", "TAY", "LDA", "TAX", "ERR", "LDY", "LDA", "LDX", "*LAX", 
    "BCS", "LDA", "ERR", "*LAX", "LDY", "LDA", "LDX", "*LAX", "CLV", "LDA", "TSX", "ERR", "LDY", "LDA", "LDX", "*LAX", 
    "CPY", "CMP", "*NOP", "*DCP", "CPY", "CMP", "DEC", "*DCP", "INY", "CMP", "DEX", "ERR", "CPY", "CMP", "DEC", "*DCP", 
    "BNE", "CMP", "ERR", "*DCP", "*NOP", "CMP", "DEC", "*DCP", "CLD", "CMP", "*NOP", "*DCP", "*NOP", "CMP", "DEC", "*DCP", 
    "CPX", "SBC", "*NOP", "*ISB", "CPX", "SBC", "INC", "*ISB", "INX", "SBC", "NOP", "*SBC", "CPX", "SBC", "INC", "*ISB", 
    "BEQ", "SBC", "ERR", "*ISB", "*NOP", "SBC", "INC", "*ISB", "SED", "SBC", "*NOP", "*ISB", "*NOP", "SBC", "INC", "*ISB", 
};
#endif //DEBUG

static const uint8_t instruction_addressing_mode[] = {
    IMP, INX, ERR, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
    ABS, INX, ERR, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
    IMP, INX, ERR, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
    IMP, INX, ERR, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, ACC, ERR, IND, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
    IMM, INX, IMM, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, ERR, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ERR, ERR, ABX, ERR, ERR, 
    IMM, INX, IMM, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ERR, ABX, ABX, ABY, ABY, 
    IMM, INX, IMM, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
    IMM, INX, IMM, INX, ZRP, ZRP, ZRP, ZRP, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS, 
    REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX, 
};

static uint8_t (*instructions[]) (cpu_registers*) = {
    &implied_BRK, &indirectX_ORA, NULL, &indirectX_SLO, &zeropage_NOP, &zeropage_ORA, &zeropage_ASL, &zeropage_SLO, &implied_PHP, &immediate_ORA, &accumulator_ASL, NULL, &absolute_NOP, &absolute_ORA, &absolute_ASL, &absolute_SLO, 
    &relative_BPL, &indirectY_ORA, NULL, &indirectY_SLO, &zeropageX_NOP, &zeropageX_ORA, &zeropageX_ASL, &zeropageX_SLO, &implied_CLC, &absoluteY_ORA, &implied_NOP, &absoluteY_SLO, &absoluteX_NOP, &absoluteX_ORA, &absoluteX_ASL, &absoluteX_SLO, 
    &absolute_JSR, &indirectX_AND, NULL, &indirectX_RLA, &zeropage_BIT, &zeropage_AND, &zeropage_ROL, &zeropage_RLA, &implied_PLP, &immediate_AND, &accumulator_ROL, NULL, &absolute_BIT, &absolute_AND, &absolute_ROL, &absolute_RLA, 
    &relative_BMI, &indirectY_AND, NULL, &indirectY_RLA, &zeropageX_NOP, &zeropageX_AND, &zeropageX_ROL, &zeropageX_RLA, &implied_SEC, &absoluteY_AND, &implied_NOP, &absoluteY_RLA, &absoluteX_NOP, &absoluteX_AND, &absoluteX_ROL, &absoluteX_RLA, 
    &implied_RTI, &indirectX_EOR, NULL, &indirectX_SRE, &zeropage_NOP, &zeropage_EOR, &zeropage_LSR, &zeropage_SRE, &implied_PHA, &immediate_EOR, &accumulator_LSR, NULL, &absolute_JMP, &absolute_EOR, &absolute_LSR, &absolute_SRE, 
    &relative_BVC, &indirectY_EOR, NULL, &indirectY_SRE, &zeropageX_NOP, &zeropageX_EOR, &zeropageX_LSR, &zeropageX_SRE, &implied_CLI, &absoluteY_EOR, &implied_NOP, &absoluteY_SRE, &absoluteX_NOP, &absoluteX_EOR, &absoluteX_LSR, &absoluteX_SRE, 
    &implied_RTS, &indirectX_ADC, NULL, &indirectX_RRA, &zeropage_NOP, &zeropage_ADC, &zeropage_ROR, &zeropage_RRA, &implied_PLA, &immediate_ADC, &accumulator_ROR, NULL, &indirect_JMP, &absolute_ADC, &absolute_ROR, &absolute_RRA, 
    &relative_BVS, &indirectY_ADC, NULL, &indirectY_RRA, &zeropageX_NOP, &zeropageX_ADC, &zeropageX_ROR, &zeropageX_RRA, &implied_SEI, &absoluteY_ADC, &implied_NOP, &absoluteY_RRA, &absoluteX_NOP, &absoluteX_ADC, &absoluteX_ROR, &absoluteX_RRA, 
    &immediate_NOP, &indirectX_STA, &immediate_NOP, &indirectX_SAX, &zeropage_STY, &zeropage_STA, &zeropage_STX, &zeropage_SAX, &implied_DEY, &immediate_NOP, &implied_TXA, NULL, &absolute_STY, &absolute_STA, &absolute_STX, &absolute_SAX, 
    &relative_BCC, &indirectY_STA, NULL, NULL, &zeropageX_STY, &zeropageX_STA, &zeropageY_STX, &zeropageY_SAX, &implied_TYA, &absoluteY_STA, &implied_TXS, NULL, NULL, &absoluteX_STA, NULL, NULL, 
    &immediate_LDY, &indirectX_LDA, &immediate_LDX, &indirectX_LAX, &zeropage_LDY, &zeropage_LDA, &zeropage_LDX, &zeropage_LAX, &implied_TAY, &immediate_LDA, &implied_TAX, NULL, &absolute_LDY, &absolute_LDA, &absolute_LDX, &absolute_LAX, 
    &relative_BCS, &indirectY_LDA, NULL, &indirectY_LAX, &zeropageX_LDY, &zeropageX_LDA, &zeropageY_LDX, &zeropageY_LAX, &implied_CLV, &absoluteY_LDA, &implied_TSX, NULL, &absoluteX_LDY, &absoluteX_LDA, &absoluteY_LDX, &absoluteY_LAX, 
    &immediate_CPY, &indirectX_CMP, &immediate_NOP, &indirectX_DCP, &zeropage_CPY, &zeropage_CMP, &zeropage_DEC, &zeropage_DCP, &implied_INY, &immediate_CMP, &implied_DEX, NULL, &absolute_CPY, &absolute_CMP, &absolute_DEC, &absolute_DCP, 
    &relative_BNE, &indirectY_CMP, NULL, &indirectY_DCP, &zeropageX_NOP, &zeropageX_CMP, &zeropageX_DEC, &zeropageX_DCP, &implied_CLD, &absoluteY_CMP, &implied_NOP, &absoluteY_DCP, &absoluteX_NOP, &absoluteX_CMP, &absoluteX_DEC, &absoluteX_DCP, 
    &immediate_CPX, &indirectX_SBC, &immediate_NOP, &indirectX_ISB, &zeropage_CPX, &zeropage_SBC, &zeropage_INC, &zeropage_ISB, &implied_INX, &immediate_SBC, &implied_NOP, &immediate_SBC, &absolute_CPX, &absolute_SBC, &absolute_INC, &absolute_ISB, 
    &relative_BEQ, &indirectY_SBC, NULL, &indirectY_ISB, &zeropageX_NOP, &zeropageX_SBC, &zeropageX_INC, &zeropageX_ISB, &implied_SED, &absoluteY_SBC, &implied_NOP, &absoluteY_ISB, &absoluteX_NOP, &absoluteX_SBC, &absoluteX_INC, &absoluteX_ISB, 
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

#ifdef DEBUG
static FILE* cpu_logfile = NULL;

static void print_accumulator_debug_info(cpu_registers* registers, uint8_t opcode) {
    fprintf(cpu_logfile, "%8s", " ");
    fprintf(cpu_logfile, "%3s A", instruction_text[opcode]);
    fprintf(cpu_logfile, "%27s", " ");
}

static void print_immediate_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t data = debug_read_RAM(registers->PC);
    fprintf(cpu_logfile, " %02X", data);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "#$%02X", data);
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
#endif // DEBUG

static uint8_t execute_NMI(cpu_registers* registers) {
    clear_nmi_flag();
    // NMI clears bit 4 and sets bit 5 for flags push
    uint8_t NMI_flags = registers->flags | 0x20;
    NMI_flags &= ~0x10;
    push_PC_onto_stack(registers, registers->PC - 1); // Sub 1 to get correct PC since it was incremented
    push_stack(registers, NMI_flags);
    registers->PC = (read_RAM(0xFFFB) << 8) | read_RAM(0xFFFA);
    set_cpu_flag(registers, INTERRUPT_FLAG);
    return 7;
}

static bool DMA_is_executing() {
    if(DMA_cycle_count) return true;
    else return false;
}

static uint8_t execute_DMA() {
    static uint8_t temp_data_buffer;

    DMA_cycle_count -= 1;
    // First 1 or 2 cycles are idle
    if(DMA_cycle_count > 512) return 1;

    if(DMA_cycle_count & 1) {
        write_primary_OAM(temp_data_buffer);
    }
    else {
        temp_data_buffer = read_RAM(DMA_RAM_address);
        DMA_RAM_address++;
    }
    return 1;
}


//*****************************************************************************
// Public functions
//*****************************************************************************
#ifdef DEBUG
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
#endif //DEBUG

void cold_boot_init(cpu_registers* registers) {
    const uint8_t initial_A = 0;
    const uint8_t initial_X = 0;
    const uint8_t initial_Y = 0;
    const uint8_t initial_S = 0xFD;
    const uint8_t initial_flags = 0x24;

    //To run NESTEST
    //uint16_t initial_PC = 0xC000;
    uint16_t initial_PC = fetch_reset_vector();
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
            // Do not increment PC if we jump because it will skip the first instruction at least
            if(!is_JSR_or_JMP(opcode) && !is_RTI(opcode)) increment_PC_instruction_length(registers, opcode);
        }
    }

    previous_nmi_flag = nmi_flag;
    odd_cpu_cycle = cpu_cycles_executed & 0x1;

    return cpu_cycles_executed;
}

void start_DMA(uint8_t DMA_address) {
    DMA_RAM_address = 0x0100 * DMA_address;
    DMA_cycle_count = odd_cpu_cycle ? 515 : 514;
}
