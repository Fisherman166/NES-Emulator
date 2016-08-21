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
#include "cpu_decode_logic.h"
#include "memory_operations.h"

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
   //0   , 1    , 2    , 3    , 4    , 5    , 6    , 7    , 8    , 9    , A    , B    , C    , D    , E    , F    ,
    "BRK", "ORA", "ERR", "ERR", "ERR", "ORA", "ASL", "ERR", "PHP", "ORA", "ASL", "ERR", "ERR", "ORA", "ASL", "ERR",
    "BPL", "ORA", "ERR", "ERR", "ERR", "ORA", "ASL", "ERR", "CLC", "ORA", "ERR", "ERR", "ERR", "ORA", "ASL", "ERR",
    "JSR", "AND", "ERR", "ERR", "BIT", "AND", "ROL", "ERR", "PLP", "AND", "ROL", "ERR", "BIT", "AND", "ROL", "ERR",
    "BMI", "AND", "ERR", "ERR", "ERR", "AND", "ROL", "ERR", "CLC", "AND", "ERR", "ERR", "ERR", "AND", "ROL", "ERR",
    "RTI", "EOR", "ERR", "ERR", "ERR", "EOR", "LSR", "ERR", "PHA", "EOR", "LSR", "ERR", "JMP", "EOR", "LSR", "ERR",
    "BVC", "EOR", "ERR", "ERR", "ERR", "EOR", "LSR", "ERR", "CLI", "EOR", "ERR", "ERR", "ERR", "EOR", "LSR", "ERR",
    "RTS", "ADC", "ERR", "ERR", "ERR", "ADC", "ROR", "ERR", "PLA", "ADC", "ROR", "ERR", "JMP", "ADC", "ROR", "ERR",
    "BVS", "ADC", "ERR", "ERR", "ERR", "ADC", "ROR", "ERR", "SEI", "ADC", "ERR", "ERR", "ERR", "ADC", "ROR", "ERR",
    "ERR", "STA", "ERR", "ERR", "STY", "STA", "STX", "ERR", "DEY", "ERR", "TXA", "ERR", "STY", "STA", "STX", "ERR",
    "BCC", "STA", "ERR", "ERR", "STY", "STA", "STX", "ERR", "TYA", "STA", "TXS", "ERR", "ERR", "STA", "ERR", "ERR",
    "LDY", "LDA", "LDX", "ERR", "LDY", "LDA", "LDX", "ERR", "TAY", "LDA", "TAX", "ERR", "LDY", "LDA", "LDX", "ERR",
    "BCS", "LDA", "ERR", "ERR", "LDY", "LDA", "LDX", "ERR", "CLV", "LDA", "TSX", "ERR", "LDY", "LDA", "LDX", "ERR",
    "CPY", "CMP", "ERR", "ERR", "CPY", "CMP", "DEC", "ERR", "INY", "CMP", "DEX", "ERR", "CPY", "CMP", "DEC", "ERR",
    "BNE", "CMP", "ERR", "ERR", "ERR", "CMP", "DEC", "ERR", "CLD", "CMP", "ERR", "ERR", "ERR", "CMP", "DEC", "ERR",
    "CPX", "SBC", "ERR", "ERR", "CPX", "SBC", "INC", "ERR", "INX", "SBC", "NOP", "ERR", "CPX", "SBC", "INC", "ERR",
    "BEQ", "SBC", "ERR", "ERR", "ERR", "SBC", "INC", "ERR", "SED", "SBC", "ERR", "ERR", "ERR", "SBC", "INC", "ERR",
};

static const uint8_t instruction_addressing_mode[] = {
  //0  , 1  , 2  , 3  , 4  , 5  , 6  , 7  , 8  , 9  , A  , B  , C  , D  , E  , F  , 
    IMP, ERR, ERR, ERR, ERR, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ERR, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
    ABS, ERR, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
    IMP, ERR, ERR, ERR, ERR, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
    IMP, ERR, ERR, ERR, ERR, ZRP, ZRP, ERR, IMP, IMM, ACC, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
    ERR, ERR, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, ERR, IMP, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ZPX, ZPX, ZPY, ERR, IMP, ABY, IMP, ERR, ERR, ABX, ERR, ERR,
    IMM, ERR, IMM, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ZPX, ZPX, ZPY, ERR, IMP, ABY, IMP, ERR, ABX, ABX, ABY, ERR,
    IMM, ERR, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
    IMM, ERR, ERR, ERR, ZRP, ZRP, ZRP, ERR, IMP, IMM, IMP, ERR, ABS, ABS, ABS, ERR,
    REL, INY, ERR, ERR, ERR, ZPX, ZPX, ERR, IMP, ABY, ERR, ERR, ERR, ABX, ABX, ERR,
};

static uint8_t (*instructions[])(cpu_registers*) = {
    &implied_BRK, &indirectX_ORA, NULL, NULL, NULL, &zeropage_ORA, &zeropage_ASL, NULL, &implied_PHP, &immediate_ORA, &accumulator_ASL, NULL, NULL, &absolute_ORA, &absolute_ASL, NULL, 
    &relative_BPL, &indirectY_ORA, NULL, NULL, NULL, &zeropageX_ORA, &zeropageX_ASL, NULL, &implied_CLC, &absoluteY_ORA, NULL, NULL, NULL, &absoluteX_ORA, &absoluteX_ASL, NULL, 
    &absolute_JSR, &indirectX_AND, NULL, NULL, &zeropage_BIT, &zeropage_AND, &zeropage_ROL, NULL, &implied_PLP, &immediate_AND, &accumulator_ROL, NULL, &absolute_BIT, &absolute_AND, &absolute_ROL, NULL, 
    &relative_BMI, &indirectY_AND, NULL, NULL, NULL, &zeropageX_AND, &zeropageX_ROL, NULL, &implied_CLC, &absoluteY_AND, NULL, NULL, NULL, &absoluteX_AND, &absoluteX_ROL, NULL, 
    &implied_RTI, &indirectX_EOR, NULL, NULL, NULL, &zeropage_EOR, &zeropage_LSR, NULL, &implied_PHA, &immediate_EOR, &accumulator_LSR, NULL, &absolute_JMP, &absolute_EOR, &absolute_LSR, NULL, 
    &relative_BVC, &indirectY_EOR, NULL, NULL, NULL, &zeropageX_EOR, &zeropageX_LSR, NULL, &implied_CLI, &absoluteY_EOR, NULL, NULL, NULL, &absoluteX_EOR, &absoluteX_LSR, NULL, 
    &implied_RTS, &indirectX_ADC, NULL, NULL, NULL, &zeropage_ADC, &zeropage_ROR, NULL, &implied_PLA, &immediate_ADC, &accumulator_ROR, NULL, &absolute_JMP, &absolute_ADC, &absolute_ROR, NULL, 
    &relative_BVS, &indirectY_ADC, NULL, NULL, NULL, &zeropageX_ADC, &zeropageX_ROR, NULL, &implied_SEI, &absoluteY_ADC, NULL, NULL, NULL, &absoluteX_ADC, &absoluteX_ROR, NULL, 
    NULL, &indirectX_STA, NULL, NULL, &zeropage_STY, &zeropage_STA, &zeropage_STX, NULL, &implied_DEY, NULL, &implied_TXA, NULL, &absolute_STY, &absolute_STA, &absolute_STX, NULL, 
    &relative_BCC, &indirectY_STA, NULL, NULL, &zeropageX_STY, &zeropageX_STA, &zeropageY_STX, NULL, &implied_TYA, &absoluteY_STA, &implied_TXS, NULL, NULL, &absoluteX_STA, NULL, NULL, 
    &immediate_LDY, &indirectX_LDA, &immediate_LDX, NULL, &zeropage_LDY, &zeropage_LDA, &zeropage_LDX, NULL, &implied_TAY, &immediate_LDA, &implied_TAX, NULL, &absolute_LDY, &absolute_LDA, &absolute_LDX, NULL, 
    &relative_BCS, &indirectY_LDA, NULL, NULL, &zeropageX_LDY, &zeropageX_LDA, &zeropageY_LDX, NULL, &implied_CLV, &absoluteY_LDA, &implied_TSX, NULL, &absoluteX_LDY, &absoluteX_LDA, &absoluteY_LDX, NULL, 
    &immediate_CPY, &indirectX_CMP, NULL, NULL, &zeropage_CPY, &zeropage_CMP, &zeropage_DEC, NULL, &implied_INY, &immediate_CMP, &implied_DEX, NULL, &absolute_CPY, &absolute_CMP, &absolute_DEC, NULL, 
    &relative_BNE, &indirectY_CMP, NULL, NULL, NULL, &zeropageX_CMP, &zeropageX_DEC, NULL, &implied_CLD, &absoluteY_CMP, NULL, NULL, NULL, &absoluteX_CMP, &absoluteX_DEC, NULL, 
    &immediate_CPX, &indirectX_SBC, NULL, NULL, &zeropage_CPX, &zeropage_SBC, &zeropage_INC, NULL, &implied_INX, &immediate_SBC, &implied_NOP, NULL, &absolute_CPX, &absolute_SBC, &absolute_INC, NULL, 
    &relative_BEQ, &indirectY_SBC, NULL, NULL, NULL, &zeropageX_SBC, &zeropageX_INC, NULL, &implied_SED, &absoluteY_SBC, NULL, NULL, NULL, &absoluteX_SBC, &absoluteX_INC, NULL, 
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

static void increment_PC_instruction_length(cpu_registers* registers, uint8_t opcode) {
    // Have to subtract 1 to account for the increment done after the
    // opcode fetch
    registers-> PC += instruction_byte_length[opcode] - 1;
}

static uint8_t execute_instruction(cpu_registers* registers, uint8_t opcode) {
    if(instructions[opcode] == NULL) {
        printf("ERROR: Illegal opcode 0x%X attempted to execute. Exiting\n", opcode);
        exit(-1);
    }
    uint8_t extra_cycles = (*instructions[opcode])(registers);
    return extra_cycles;
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

// TODO: REMOVE after PPU is coded
static uint16_t dot = 0;
static int16_t scanline = 241;
static void ppu_bfm(uint8_t extra_cycles, uint8_t opcode) {
    uint8_t cpu_cycles = instruction_cycle_length[opcode] + extra_cycles;

    dot += cpu_cycles * 3;
    if(dot > 340) {
        dot -= 341;
        scanline++;

        if(scanline > 260) {
            scanline = -1;
        }
    }
    
    const uint16_t reg2002_address = 0x2002;
    const uint8_t vblank_value = 0x80;
    if((scanline == 241) && (dot > 0)) {
        write_RAM(reg2002_address, vblank_value);
    }
    if((scanline == -1) && (dot > 0)) {
        write_RAM(reg2002_address, 0x00);
    }
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
    uint8_t data_read = fetch_immediate(registers);
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
    uint8_t zeropage_address  = fetch_immediate(registers);
    uint8_t data_read = fetch_zeropage(registers);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X = %02X", zeropage_address, data_read);
    fprintf(cpu_logfile, "%20s", " ");
}

static void print_zeropageX_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address  = fetch_immediate(registers);
    uint8_t zeropageX_address  = calc_zeropageX_address(registers);
    uint8_t data_read = fetch_zeropageX(registers);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X,X @ %02X = %02X", zeropage_address, zeropageX_address, 
            data_read);
    fprintf(cpu_logfile, "%13s", " ");
}

static void print_zeropageY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address = fetch_immediate(registers);
    uint8_t zeropageY_address = calc_zeropageY_address(registers);
    uint8_t data_read = fetch_zeropageY(registers);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X,Y @ %02X = %02X", zeropage_address, zeropageY_address, 
            data_read);
    fprintf(cpu_logfile, "%13s", " ");
}

static void print_absolute_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = fetch_immediate(registers);
    uint8_t high_address_byte = read_RAM(registers->PC + 1);
    uint16_t absolute_address = calc_absolute_address(registers);
    uint8_t data_read = fetch_absolute(registers);
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
    uint8_t low_address_byte = fetch_immediate(registers);
    uint8_t high_address_byte = read_RAM(registers->PC + 1);
    uint16_t absoluteX_address = calc_absoluteX_address(registers);
    uint8_t data_read = fetch_absoluteX(registers, NULL);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X%02X,X @ %04X = %02X", high_address_byte, low_address_byte,
            absoluteX_address, data_read);
    fprintf(cpu_logfile, "%9s", " ");
}

static void print_absoluteY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = fetch_immediate(registers);
    uint8_t high_address_byte = read_RAM(registers->PC + 1);
    uint16_t absoluteY_address = calc_absoluteY_address(registers);
    uint8_t data_read = fetch_absoluteY(registers, NULL);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%02X%02X,Y @ %04X = %02X", high_address_byte, low_address_byte,
            absoluteY_address, data_read);
    fprintf(cpu_logfile, "%9s", " ");
}

static void print_indirectY_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t zeropage_address = fetch_immediate(registers);
    uint16_t indirect_address = calc_indirectY_address(registers);
    uint8_t data_read = fetch_indirectY(registers, NULL);
    fprintf(cpu_logfile, " %02X", zeropage_address);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "($%02X),Y = %04X @ %04X = %02X", zeropage_address, indirect_address - registers->Y,
            indirect_address, data_read);
    fprintf(cpu_logfile, "%2s", " ");
}

static void print_relative_debug_info(cpu_registers* registers, uint8_t opcode) {
    char offset = fetch_immediate(registers) & BYTE_MASK;
    uint16_t target_address = registers->PC + offset + instruction_byte_length[opcode] - 1;
    fprintf(cpu_logfile, " %02X", offset & BYTE_MASK);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "$%04X", target_address);
    fprintf(cpu_logfile, "%23s", " ");
}

static void print_indirect_debug_info(cpu_registers* registers, uint8_t opcode) {
    uint8_t low_address_byte = fetch_immediate(registers);
    uint8_t high_address_byte = read_RAM(registers->PC + 1);
    uint16_t indirect_address = calc_absolute_address(registers);
    fprintf(cpu_logfile, " %02X %02X", low_address_byte, high_address_byte);
    fprintf(cpu_logfile, "%2s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "($%02X%02X) = %04X", high_address_byte, low_address_byte,
            indirect_address);
    fprintf(cpu_logfile, "%14s", " ");
}

static void print_common_debug_info(cpu_registers* registers) {
    fprintf(cpu_logfile, "A:%02X ", registers->A);
    fprintf(cpu_logfile, "X:%02X ", registers->X);
    fprintf(cpu_logfile, "Y:%02X ", registers->Y);
    fprintf(cpu_logfile, "P:%02X ", registers->flags);
    fprintf(cpu_logfile, "SP:%02X ", registers->S);
    fprintf(cpu_logfile, "CYC:%3d ", dot);
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
    else if(instruction_addressing_mode[opcode] == INY) print_indirectY_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == REL) print_relative_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == IND) print_indirect_debug_info(registers, opcode);
    else printf("ERROR: Addressing mode %d not recogonized for opcode 0x%02X\n",
                instruction_addressing_mode[opcode], opcode);

    print_common_debug_info(registers);
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

    uint16_t initial_PC = fetch_reset_vector();
    init_cpu_registers(registers, initial_A, initial_X, initial_Y, initial_PC,
                       initial_S, initial_flags);
}

void execute_interpreter_cycle(cpu_registers* registers) {
    uint8_t opcode = fetch_opcode(registers);
    increment_PC(registers);

    #ifdef DEBUG
        print_debug_info(registers, opcode);
    #endif

    uint8_t extra_cycles = execute_instruction(registers, opcode);
    // Do not increment PC if we jump because it will skip the first instruction at least
    if(!is_JSR_or_JMP(opcode)) increment_PC_instruction_length(registers, opcode);
    ppu_bfm(extra_cycles, opcode);
}

    
