//*****************************************************************************
// Filename: RAM.c
// Author: Fisherman166
//
// Implements the RAM memory operations
//
//*****************************************************************************

#include <stdlib.h>
#include "RAM.h"
#include "cpu_decode_logic.h"
#include "sdl_interface.h"
#include "VRAM.h"
#include "cpu.h"

#define RAM_locations 0x10000

#define INTERNAL_RAM_BASE_ADDR 0x0000
#define INTERNAL_RAM_MIRROR_BASE_ADDR 0x0800
#define PPU_REGISTERS_BASE_ADDR 0x2000
#define PPU_REGISTERS_MIRROR_BASE_ADDR 0x2008
#define APU_IO_REGISTERS_BASE_ADDR 0x4000
#define APU_DISABLED_REGISTERS_BASE_ADDR 0x4018
#define CART_ROM_BASE_ADDR 0x4020

#define DMA_REG_ADDR 0x4014
#define VBLANK_BIT 0x80
#define NMI_BIT 0x80

uint8_t RAM[RAM_locations];

//*****************************************************************************
// Private Functions 
//*****************************************************************************
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

//*****************************************************************************
// Public Functions 
//*****************************************************************************
void init_RAM() {
    for(int i = 0; i < 0x8000; i++) RAM[i] = 0x00;
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

    if(address == PPUCTRL_ADDRESS) PPUCTRL_update_temp_VRAM_address(value);
    else if(address == OAMDATA_ADDRESS) RAM[OAMADDR_ADDRESS] += 1;
    else if(address == PPUSCROLL_ADDRESS) PPUSCROLL_update_temp_VRAM_address(value);
    else if(address == PPUADDR_ADDRESS) PPUADDR_update_temp_VRAM_address(value);
    else if(address == PPUDATA_ADDRESS) PPUDATA_update_temp_VRAM_address(value, RAM[PPUCTRL_ADDRESS]);
    else if(address == DMA_REG_ADDR) start_DMA(value);
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

bool is_NMI_set_in_RAM() {
    return (RAM[PPUCTRL_ADDRESS] & NMI_BIT); 
}

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

