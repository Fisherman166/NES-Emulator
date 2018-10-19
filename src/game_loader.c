//*****************************************************************************
// Filename: game_loader.c
// Author: Fisherman166
//
// Implements functions that load the game into memory
//
//*****************************************************************************

#include <stdlib.h>
#include "common.h"
#include "game_loader.h"
#include "RAM.h"
#include "mappers.h"

#define NROM 0

static long game_size = 0;

static FILE* open_game_file(const char* game_file) {
    FILE* game_filehandle = NULL;
    game_filehandle = fopen(game_file, "rb");
    printf("Opening game %s for execution\n", game_file);
    if(game_file == NULL) {
        printf("ERROR: Failed to open game file %s\n", game_file);
    }
    return game_filehandle;
}

static uint8_t* read_rom(const char* game_filename) {
    FILE* game_filehandle = open_game_file(game_filename);
    if(game_filehandle == NULL) {
        return NULL;
    }

    if(fseek(game_filehandle, 0, SEEK_END) != 0) {
        printf("ERROR: Failed to move to END of file\n");
        return NULL;
    }

    game_size = ftell(game_filehandle);
    if(fseek(game_filehandle, 0, SEEK_SET) != 0) {
        printf("ERROR: Failed to move to START of file\n");
        return NULL;
    }

    uint8_t* memory_block = malloc(game_size * sizeof(uint8_t));
    if(memory_block == NULL) {
        printf("ERROR: Failed to malloc memory block for rom data\n");
        return NULL;
    }
    size_t bytes_read = fread(memory_block, sizeof(uint8_t), game_size, game_filehandle);

    if(bytes_read != game_size) {
        printf("ERROR: Only read %u bytes when expecting %u bytes.\n",
               (uint32_t)bytes_read, (uint32_t)game_size);
        return NULL;
    }

    fclose(game_filehandle);
    return memory_block;
}

static uint8_t extract_mapper_from_header(uint8_t* memory_block) {
    return (memory_block[7] & 0xF0) | ((memory_block[6] & 0xF0) >> 4);
}

#ifdef DEBUG
static void print_rom_data() {
    const uint16_t rom_start = 0x8000;
    FILE* rom_data = fopen("rom_data.log", "w");

    if(rom_data == NULL) {
        printf("ERROR: rom_data.log failed to open\n");
        exit(1);
    }
    for(uint32_t address = rom_start; address < 0x10000; address += 0x10)
        fprintf(rom_data, "0x%04X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                address, debug_read_RAM(address), debug_read_RAM(address + 1), debug_read_RAM(address + 2),
                debug_read_RAM(address + 3), debug_read_RAM(address + 4), debug_read_RAM(address + 5),
                debug_read_RAM(address + 6), debug_read_RAM(address + 7), debug_read_RAM(address + 8),
                debug_read_RAM(address + 9), debug_read_RAM(address + 10), debug_read_RAM(address + 11),
                debug_read_RAM(address + 12), debug_read_RAM(address + 13), debug_read_RAM(address + 14),
                debug_read_RAM(address + 15));
    fclose(rom_data);
}
#endif //DEBUG

// Returns true on loading errors, false for no loading error
bool load_game(const char* game_filename) {
    uint8_t* game_data = read_rom(game_filename);
    if(game_data == NULL) {
        return true;
    }

    uint8_t mapper = extract_mapper_from_header(game_data);
    printf("Game is using mapper %u\n", mapper);

    if(mapper == NROM) load_NROM(game_data, game_size);
    else {
        printf("ERROR: Mapper %u does not match any supported mappers\n", mapper);
        free(game_data);
        return true;
    }

    #ifdef DEBUG
        print_rom_data();
    #endif
    free(game_data);
    return false;
}

