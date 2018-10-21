#include "VRAM.h"
#include "ppu.h"
#include "stdlib.h"

#define VRAM_SIZE 0x4000
#define VRAM_WRAP_MASK 0x3FFF

#define PATTERN_TABLE1_BASE_ADDR 0x0000
#define PATTERN_TABLE2_BASE_ADDR 0x1000
#define NAMETABLE1_BASE_ADDR 0x2000
#define NAMETABLE2_BASE_ADDR 0x2400
#define NAMETABLE3_BASE_ADDR 0x2800
#define NAMETABLE4_BASE_ADDR 0x2C00
#define NAMETABLE_MIRROR_BASE_ADDR 0x3000
#define PALLATE_TABLE_BASE_ADDR 0x3F00
#define PALLATE_TABLE_MIRROR_BASE_ADDR 0x3F20

static uint8_t VRAM[VRAM_SIZE];
static bool write_toggle;
static enum mirror_modes {horizontal, vertical, single_screen_low, single_screen_high, four_sceen} mirroring;

static struct VRAM_state_s {
    uint16_t VRAM_address;
    uint16_t temp_VRAM_address;
    uint8_t fineX_scroll;
} VRAM_state;

//*****************************************************************************
// Private Functions
//*****************************************************************************
static bool is_nametable_mirror(uint16_t address) {
    return( (address >= NAMETABLE_MIRROR_BASE_ADDR) && (address < PALLATE_TABLE_BASE_ADDR) );
}

static bool is_pallete_mirror(uint16_t address) {
    return( (address >= PALLATE_TABLE_MIRROR_BASE_ADDR) && (address < VRAM_SIZE) );
}

// These addrsses are speficially mirrored down 0x10 for some reason.
// See PPU palletes page on NESDEV wiki for more information.
static bool is_special_pallete_mirror(uint16_t address) {
    return( (address == 0x3F10) || (address == 0x3F14) ||
            (address == 0x3F18) || (address == 0x3F1C) );
}

static bool is_horizontal_mirror(uint16_t address) {
    bool nametable1_mirror = (address >= NAMETABLE2_BASE_ADDR) &&
                             (address < NAMETABLE3_BASE_ADDR);
    bool nametable2_mirror = (address >= NAMETABLE4_BASE_ADDR) &&
                             (address < NAMETABLE_MIRROR_BASE_ADDR);
    return (mirroring == horizontal) && (nametable1_mirror | nametable2_mirror);
}

static bool is_vertical_mirror(uint16_t address) {
    bool is_mirror = (address >= NAMETABLE3_BASE_ADDR) &&
                     (address < NAMETABLE_MIRROR_BASE_ADDR);
    return (mirroring == vertical) && is_mirror;
}

static uint16_t sanitize_VRAM_address(uint16_t address) {
    address &= VRAM_WRAP_MASK;
    if(is_nametable_mirror(address)) address -= 0x1000;
    if(is_pallete_mirror(address)) address = PALLATE_TABLE_BASE_ADDR | (address & 0x1F);
    if(is_special_pallete_mirror(address)) address = PALLATE_TABLE_BASE_ADDR | (address & 0xF);

    // Four screen does not mirror
    if(is_horizontal_mirror(address)) address &= ~0x0400;
    else if(is_vertical_mirror(address)) address &= ~0x0800;
    else if(mirroring == single_screen_low) address = NAMETABLE1_BASE_ADDR | (address & 0x03FF);
    else if(mirroring == single_screen_high) address = NAMETABLE2_BASE_ADDR | (address & 0x03FF);
    return address;
}

//*****************************************************************************
// Public Functions
//*****************************************************************************
void init_VRAM() {
    // Set nametable
    for(uint16_t address = NAMETABLE1_BASE_ADDR; address < NAMETABLE_MIRROR_BASE_ADDR; address++) {
        write_VRAM(address, 0xFF);
    }
    // set pallete
    for(uint16_t address = PALLATE_TABLE_BASE_ADDR; address < PALLATE_TABLE_MIRROR_BASE_ADDR; address++) {
        write_VRAM(address, 0x3F);
    }
}

void set_horizontal_mirroring() {
    mirroring = horizontal;
}

void set_vertical_mirroring() {
    mirroring = vertical;
}

void set_single_screen_low_mirroring() {
    mirroring = single_screen_low;
}

void set_single_screen_high_mirroring() {
    mirroring = single_screen_high;
}

void set_four_screen_mirroring() {
    mirroring = four_sceen;
}

uint8_t read_VRAM(uint16_t address) {
    return VRAM[sanitize_VRAM_address(address)];
}

void write_VRAM(uint16_t address, uint8_t data) {
    VRAM[sanitize_VRAM_address(address)] = data;
}

// Functions called from the RAM
void clear_write_toggle() {
    write_toggle = false;
}

uint8_t PPUDATA_VRAM_read() {
    uint16_t VRAM_address = sanitize_VRAM_address(VRAM_state.VRAM_address);
    static uint8_t read_buffer = 0;
    uint8_t retval;

    if(VRAM_address >= PALLATE_TABLE_BASE_ADDR)
        retval = read_VRAM(VRAM_address);
    else {
        retval = read_buffer;
        read_buffer = read_VRAM(VRAM_address);
    }
    return retval;
}

void increment_ppu_address(uint8_t PPUCTRL_value) {
    const uint8_t ppu_increment_mask = 0x04;
    if(PPUCTRL_value & ppu_increment_mask)
        VRAM_state.VRAM_address = (VRAM_state.VRAM_address + 32) & VRAM_WRAP_MASK;
    else
        VRAM_state.VRAM_address = (VRAM_state.VRAM_address + 1) & VRAM_WRAP_MASK;
}

void PPUCTRL_update_temp_VRAM_address(uint8_t value) {
    const uint8_t nametable_select_input_mask = 0x03;
    const uint16_t nametable_select_clear_mask = ~0x0C00;
    VRAM_state.temp_VRAM_address &= nametable_select_clear_mask;
    VRAM_state.temp_VRAM_address |= ((value & nametable_select_input_mask) << 10);
}

void PPUSCROLL_update_temp_VRAM_address(uint8_t value) {
    const uint16_t clear_Y_mask = 0x8C1F;
    const uint16_t clear_X_mask = ~0x001F;

    if(write_toggle) {
        VRAM_state.temp_VRAM_address &= clear_Y_mask;
        VRAM_state.temp_VRAM_address |= (value & 0xF8) << 2;
        VRAM_state.temp_VRAM_address |= (value & 0x07) << 12;
    }
    else {
        VRAM_state.temp_VRAM_address &= clear_X_mask;
        VRAM_state.temp_VRAM_address |= (value & 0xF8) >> 3;
        VRAM_state.fineX_scroll = value & 0x07;
    }
    write_toggle ^= 1;
}

void PPUADDR_update_temp_VRAM_address(uint8_t value) {
    const uint16_t clear_low_byte = 0xFF00;
    const uint16_t clear_high_byte = 0x00FF;

    if(write_toggle) {
        VRAM_state.temp_VRAM_address &= clear_low_byte;
        VRAM_state.temp_VRAM_address |= value;
        VRAM_state.VRAM_address = VRAM_state.temp_VRAM_address;
    }
    else {
        VRAM_state.temp_VRAM_address &= clear_high_byte;
        VRAM_state.temp_VRAM_address |= (value & 0x3F) << 8;
    }
    write_toggle ^= 1;
}

void PPUDATA_update_temp_VRAM_address(uint8_t value, uint8_t PPUCTRL_value) {
    write_VRAM(VRAM_state.VRAM_address, value);
    increment_ppu_address(PPUCTRL_value);
}

//PPU functions
void incrementY() {
    if( (VRAM_state.VRAM_address & 0x7000) != 0x7000 ) VRAM_state.VRAM_address += 0x1000;    // Increment if fine Y < 7
    else {
        VRAM_state.VRAM_address &= ~0x7000;   //Fine Y = 0
        uint16_t coarseY = (VRAM_state.VRAM_address & 0x03E0) >> 5;

        if(coarseY == 29) {
            coarseY = 0;
            VRAM_state.VRAM_address ^= 0x0800;    //Switch vertical nametable
        }
        else if(coarseY == 31) {
            coarseY = 0;
        }
        else coarseY++;
        VRAM_state.VRAM_address = (VRAM_state.VRAM_address & ~0x03E0) | (coarseY << 5); //Coarse Y back into address
    }
}

void incrementX() {
    const uint16_t coarseX_bitmask = 0x001F;
    const uint16_t switch_nametable = 0x0400;

    if( (VRAM_state.VRAM_address & coarseX_bitmask) == 31 ) {
        VRAM_state.VRAM_address &= ~coarseX_bitmask;  // CoarseX = 0
        VRAM_state.VRAM_address ^= switch_nametable;  // Switch horizontal nametable
    }
    else VRAM_state.VRAM_address++;
}

void copyX() {
    const uint16_t clear_horizontal_position_bitmask = 0x041F;
    VRAM_state.VRAM_address &= ~clear_horizontal_position_bitmask;
    VRAM_state.VRAM_address |= VRAM_state.temp_VRAM_address & clear_horizontal_position_bitmask;
}

void copyY() {
    const uint16_t clear_vertical_position_bitmask = 0x7BE0;
    VRAM_state.VRAM_address &= ~clear_vertical_position_bitmask;
    VRAM_state.VRAM_address |= VRAM_state.temp_VRAM_address & clear_vertical_position_bitmask;
}

uint8_t get_fineX_scroll() {
    return VRAM_state.fineX_scroll;
}

uint16_t get_VRAM_address() {
    return VRAM_state.VRAM_address;
}

uint16_t get_temp_VRAM_address() {
    return VRAM_state.temp_VRAM_address;
}

bool get_write_toggle() {
    return write_toggle;
}

void debug_dump_VRAM() {
    FILE* VRAM_dump = fopen("VRAM_dump.bin", "w");
    if(VRAM_dump == NULL) {
        printf("ERROR: Could not open VRAM_dump.bin\n");
        exit(0);
    }
    for(uint32_t addr = 0; addr < VRAM_SIZE; addr += 0x10) {
        fprintf(VRAM_dump, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                addr, read_VRAM(addr), read_VRAM(addr + 0x1), read_VRAM(addr + 0x2), read_VRAM(addr + 0x3), read_VRAM(addr + 0x4),
                read_VRAM(addr + 0x5), read_VRAM(addr + 0x6), read_VRAM(addr + 0x7), read_VRAM(addr + 0x8), read_VRAM(addr + 0x9),
                read_VRAM(addr + 0xA), read_VRAM(addr + 0xB), read_VRAM(addr + 0xC), read_VRAM(addr + 0xD), read_VRAM(addr + 0xE),
                read_VRAM(addr + 0xF));
    }
    fclose(VRAM_dump);
}

