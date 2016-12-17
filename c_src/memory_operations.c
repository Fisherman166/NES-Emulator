//*****************************************************************************
// Filename: memory_operations.c
// Author: Fisherman166
//
// Implements the memory operations for the CPU and PPU
//
//*****************************************************************************

#include <stdlib.h>
#include "memory_operations.h"
#include "cpu_decode_logic.h"
#include "mappers.h"
#include "sdl_interface.h"

#define RAM_locations 0x10000

#define NROM 0

uint8_t RAM[RAM_locations];

//*****************************************************************************
// Functions 
//*****************************************************************************
void init_RAM() {
    for(int i = 0; i < 0x4000; i++) RAM[i] = 0xFF;
    for(int i = 0x4000; i < 0x8000; i++) RAM[i] = 0xFF;
}

static uint8_t* read_rom(FILE* game_filehandle) {
    if(fseek(game_filehandle, 0, SEEK_END) != 0) {
        printf("ERROR: Failed to move to END of file\n");
        return NULL;
    }

    long game_size = ftell(game_filehandle);
    if(fseek(game_filehandle, 0, SEEK_SET) != 0) {
        printf("ERROR: Failed to move to START of file\n");
        return NULL;
    }

    uint8_t* memory_block = malloc(game_size * sizeof(uint8_t));
    if(memory_block == NULL) {
        printf("ERROR: Failed to malloc memory block\n");
        return NULL;
    }
    size_t bytes_read = fread(memory_block, sizeof(uint8_t), game_size, game_filehandle);
    fclose(game_filehandle);

    if(bytes_read != game_size) {
        printf("ERROR: Only read %u bytes when expecting %u bytes.\n",
               (uint32_t)bytes_read, (uint32_t)game_size);
        return NULL;
    }
    return memory_block;
}

static uint8_t extract_mapper_from_header(uint8_t* memory_block) {
    // Upper nibble of header in byte 8 while lower nibble in byte
    // 7
    return (memory_block[7] & 0xF0) | (memory_block[6] & 0x0F);
}

bool load_game() {
    FILE* game_filehandle = NULL;
    char* game_filename = "Donkey_Kong.nes";
    game_filehandle = fopen(game_filename, "rb");
    if(game_filename == NULL) {
        printf("ERROR: Failed to open game file %s\n", game_filename);
        return false;
    }
    uint8_t* game_data = read_rom(game_filehandle);
    if(game_data == NULL) {
        return false;
    }
    uint8_t mapper = extract_mapper_from_header(game_data);

    if(mapper == NROM) load_NROM(game_data);
    else {
        printf("ERROR: Mapper %u does not match any supported mappers\n", mapper);
        return false;
    }

    free(game_data);
    return true;
}
    
uint8_t read_RAM(uint16_t address_to_read) {
    // FIXME - Revisit controller input when PPU is working and allowing the game to boot
    if(address_to_read == JOYPAD1_ADDRESS) {
        printf("JOYPAD1 RAM VALUE = %X", RAM[address_to_read]);
    }
    return RAM[address_to_read];
}

void write_RAM(uint16_t address_to_write, uint8_t value_to_write) {
    if(address_to_write == JOYPAD1_ADDRESS) {
        if(value_to_write & 1) enable_controller_strobe(JOYPAD1);
        else disable_controller_strobe(JOYPAD1);
    }
    else if(address_to_write == JOYPAD2_ADDRESS) {
        if(value_to_write & 1) enable_controller_strobe(JOYPAD2);
        else disable_controller_strobe(JOYPAD2);
    }
    RAM[address_to_write] = value_to_write;
}

//*****************************************************************************
// Specific address mode writes
//*****************************************************************************
void write_zeropage(cpu_registers* registers, uint8_t data) {
    uint8_t zeropage_address = calc_zeropage_address(registers);
    write_RAM(zeropage_address, data);
}


void write_zeropageX(cpu_registers* registers, uint8_t data) {
    uint8_t zeropageX_address = calc_zeropageX_address(registers);
    write_RAM(zeropageX_address, data);
}


void write_absolute(cpu_registers* registers, uint8_t data) {
    uint16_t absolute_address = calc_absolute_address(registers);
    write_RAM(absolute_address, data);
}


void write_absoluteX(cpu_registers* registers, uint8_t data) {
    uint16_t absoluteX_address = calc_absoluteX_address(registers);
    write_RAM(absoluteX_address, data);
}

