//*****************************************************************************
// Filename: rom_loader.c
// Author: Fisherman166
//
// Implements functions that load the rom into memory
//
//*****************************************************************************

#include <stdlib.h>
#include "common.h"
#include "rom_loader.h"
#include "mappers.h"
#include "rom_parser.h"
#include "RAM.h"

#define NROM 0

static FILE* open_rom_file(const char* rom_file) {
    FILE* rom_filehandle = NULL;
    rom_filehandle = fopen(rom_file, "rb");
    printf("Opening rom %s for loading\n", rom_file);
    if(rom_file == NULL) {
        printf("ERROR: Failed to open rom file %s\n", rom_file);
    }
    return rom_filehandle;
}

static bool read_rom(const char* rom_filename) {
    FILE* rom_filehandle = open_rom_file(rom_filename);
    if(rom_filehandle == NULL) {
        return true;
    }

    if(fseek(rom_filehandle, 0, SEEK_END) != 0) {
        printf("ERROR: Failed to move to END of file\n");
        return true;
    }

    long rom_size_in_bytes = ftell(rom_filehandle);
    if(rom_size_in_bytes == -1L) {
        printf("ERROR: ftell failed to get the size of the rom image\n");
        return true;
    }

    if(fseek(rom_filehandle, 0, SEEK_SET) != 0) {
        printf("ERROR: Failed to move to START of file\n");
        return true;
    }

    uint8_t* rom_data = malloc(rom_size_in_bytes * sizeof(uint8_t));
    if(rom_data == NULL) {
        printf("ERROR: Failed to malloc memory block for rom data\n");
        return true;
    }
    size_t bytes_read = fread(rom_data, sizeof(uint8_t), rom_size_in_bytes, rom_filehandle);
    fclose(rom_filehandle);

    if(bytes_read != rom_size_in_bytes) {
        printf("ERROR: Only read %u bytes when expecting %u bytes.\n",
               (uint32_t)bytes_read, (uint32_t)rom_size_in_bytes);
        return true;
    }

    parse_iNes_1_0_header(rom_data);
    parse_PRG_rom(rom_data);
    parse_CHR_rom(rom_data);
    free(rom_data);

    return false;
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
bool load_rom(const char* rom_filename) {
    if(read_rom(rom_filename)) {
        return true;
    }

    iNES_1_0_header parsed_header = get_iNes_1_0_header();
    print_iNes_1_0_header(parsed_header);
    switch(parsed_header.mapper) {
        case NROM:
            load_NROM();
            break;
        default:
            printf("ERROR: Mapper %u does not match any supported mappers\n", parsed_header.mapper);
            return true;
    }

    #ifdef DEBUG
        print_rom_data();
    #endif
    return false;
}
