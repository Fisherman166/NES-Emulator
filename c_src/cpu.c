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
#define ZEP 2
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
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 0
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 1
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 2
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 3
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 4
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 5
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ADC", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 6
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 7
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 8
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // 9
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // A
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // B
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // C
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // D
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // E
    "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR",  // F
};

static const uint8_t instruction_addressing_mode[] = {
  //0  , 1  , 2  , 3  , 4  , 5  , 6  , 7  , 8  , 9  , A  , B  , C  , D  , E  , F  , 
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 0
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 1
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 2
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 3
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 4
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 5
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, IMM, ERR, ERR, ERR, ERR, ERR, ERR,  // 6
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 7
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 8
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // 9
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // A
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // B
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // C
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // D
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // E
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,  // F
};

static uint8_t (*instructions[]) (cpu_registers*) = {
  //0   , 1   , 2   , 3   , 4   , 5   , 6   , 7   , 8   , 9   , A   , B   , C   , D   , E   , F
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 1
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 2
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 3
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 4
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 5
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &immediate_ADC, NULL, NULL, NULL, NULL, NULL, NULL, // 6
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 8
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 9
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // A
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // B
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // C
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // D
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // E
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // F
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
    fprintf(cpu_logfile, " %2X", data_read);
    fprintf(cpu_logfile, "%5s", " ");
    fprintf(cpu_logfile, "%3s ", instruction_text[opcode]);
    fprintf(cpu_logfile, "#$%2X", data_read);
    fprintf(cpu_logfile, "%24s", " ");
}

static void print_common_debug_info(cpu_registers* registers) {
    fprintf(cpu_logfile, "A:%2X ", registers->A);
    fprintf(cpu_logfile, "X:%2X ", registers->X);
    fprintf(cpu_logfile, "Y:%2X ", registers->Y);
    fprintf(cpu_logfile, "P:%2X ", registers->flags);
    fprintf(cpu_logfile, "SP:%2X ", registers->S);
    fprintf(cpu_logfile, "CYC:%3d ", 0);
    fprintf(cpu_logfile, "SL:%3d ", 0);
    fprintf(cpu_logfile, "\n");
}

static void print_debug_info(cpu_registers* registers, uint8_t opcode) {
    if(cpu_logfile == NULL) printf("ERROR: cpu_logfile not open when attempting to write to file\n");

    fprintf(cpu_logfile, "%04X  ", registers->PC - 1); // Decrement to get original PC
    fprintf(cpu_logfile, "%02X", opcode);

    if(instruction_addressing_mode[opcode] == ACC) print_accumulator_debug_info(registers, opcode);
    else if(instruction_addressing_mode[opcode] == IMM) print_immediate_debug_info(registers, opcode);
    else printf("ERROR: Addressing mode %d not recogonized.\n", instruction_addressing_mode[opcode]);
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
    const uint8_t initial_flags = 0x34;

    uint16_t initial_PC = fetch_reset_vector();
    init_cpu_registers(registers, initial_A, initial_X, initial_Y, initial_PC,
                       initial_S, initial_flags);
}

void execute_interpreter_cycle(cpu_registers* registers) {
    uint8_t opcode = fetch_opcode(registers);
    increment_PC(registers);
    execute_instruction(registers, opcode);

    #ifdef DEBUG
        print_debug_info(registers, opcode);
    #endif

    increment_PC_instruction_length(registers, opcode);
}

