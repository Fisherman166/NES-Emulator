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
#include "VRAM.h"

#define RAM_locations 0x10000

#define INTERNAL_RAM_BASE_ADDR 0x0000
#define INTERNAL_RAM_MIRROR_BASE_ADDR 0x0800
#define PPU_REGISTERS_BASE_ADDR 0x2000
#define PPU_REGISTERS_MIRROR_BASE_ADDR 0x2008
#define APU_IO_REGISTERS_BASE_ADDR 0x4000
#define APU_DISABLED_REGISTERS_BASE_ADDR 0x4018
#define CART_ROM_BASE_ADDR 0x4020

#define NROM 0

#define VBLANK_BIT 0x80
#define NMI_BIT 0x80

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

static void print_rom_data() {
    const uint16_t rom_start = 0x8000;
    FILE* rom_data = fopen("rom_data.log", "w");

    if(rom_data == NULL) {
        printf("ERROR: rom_data.log failed to open\n");
        exit(1);
    }
    for(uint32_t address = rom_start; address < 0x10000; address += 0x10)
        fprintf(rom_data, "0x%04X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                address, RAM[address], RAM[address + 1], RAM[address + 2], RAM[address + 3],
                RAM[address + 4], RAM[address + 5], RAM[address + 6], RAM[address + 7],
                RAM[address + 8], RAM[address + 9], RAM[address + 10], RAM[address + 11],
                RAM[address + 12], RAM[address + 13], RAM[address + 14], RAM[address + 15]);
    fclose(rom_data);
}

bool load_game() {
    FILE* game_filehandle = NULL;
    char* game_filename = "Donkey_Kong.nes";
    game_filehandle = fopen(game_filename, "rb");
    if(game_filename == NULL) {
        printf("ERROR: Failed to open game file %s\n", game_filename);
        return true;
    }
    uint8_t* game_data = read_rom(game_filehandle);
    if(game_data == NULL) {
        return true;
    }
    uint8_t mapper = extract_mapper_from_header(game_data);

    if(mapper == NROM) load_NROM(game_data);
    else {
        printf("ERROR: Mapper %u does not match any supported mappers\n", mapper);
        return true;
    }

    #ifdef DEBUG
        print_rom_data();
    #endif

    free(game_data);
    return false;
}

static bool is_ppu_mirror_address(uint16_t address) {
    return( (address >= PPU_REGISTERS_MIRROR_BASE_ADDR) &&
            (address < APU_IO_REGISTERS_BASE_ADDR) );
}

static bool is_internal_RAM_mirror_address(uint16_t address) {
    return( (address >= INTERNAL_RAM_MIRROR_BASE_ADDR) &&
            (address < PPU_REGISTERS_BASE_ADDR) );
}

static uint16_t sanitize_RAM_address(uint16_t address) {
    if( is_ppu_mirror_address(address) )
        address = PPU_REGISTERS_BASE_ADDR | (address & 0x7);
    else if( is_internal_RAM_mirror_address(address) )
        address &= 0x7FF;
    return address;
}
    
uint8_t read_RAM(uint16_t address) {
    address = sanitize_RAM_address(address);
    uint8_t retval = RAM[address];

    if(address == PPUSTATUS_ADDRESS) {
        clear_vblank_bit();
        clear_write_toggle();
    }
    else if(address == PPUDATA_ADDRESS) {
        retval = PPUDATA_VRAM_read();
        increment_ppu_address(RAM[PPUCTRL_ADDRESS]);
    }
    return retval;
}

// Does not change values for debug
uint8_t debug_read_RAM(uint16_t address) {
    address = sanitize_RAM_address(address);
    return RAM[address];
}

void write_RAM(uint16_t address, uint8_t value) {
    address = sanitize_RAM_address(address);
    RAM[address] = value;

    if(address == PPUSTATUS_ADDRESS) PPUSTATUS_update_temp_VRAM_address(value);
    else if(address == OAMDATA_ADDRESS) RAM[OAMADDR_ADDRESS] += 1;
    else if(address == PPUSCROLL_ADDRESS) PPUSCROLL_update_temp_VRAM_address(value);
    else if(address == PPUADDR_ADDRESS) PPUADDR_update_temp_VRAM_address(value);
    else if(address == PPUDATA_ADDRESS) PPUDATA_update_temp_VRAM_address(value, RAM[PPUCTRL_ADDRESS]);
    else if(address == JOYPAD1_ADDRESS) {
        if(value & 1) enable_controller_strobe(JOYPAD1);
        else disable_controller_strobe(JOYPAD1);
    }
    else if(address == JOYPAD2_ADDRESS) {
        if(value & 1) enable_controller_strobe(JOYPAD2);
        else disable_controller_strobe(JOYPAD2);
    }
}

void set_vblank_bit() {
    RAM[PPUSTATUS_ADDRESS] |= VBLANK_BIT;
}

void clear_vblank_bit() {
    RAM[PPUSTATUS_ADDRESS] &= ~VBLANK_BIT;
}

bool NMI_and_vblank_set() {
    return( (RAM[PPUCTRL_ADDRESS] & NMI_BIT) && (RAM[PPUSTATUS_ADDRESS] & VBLANK_BIT) );
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

