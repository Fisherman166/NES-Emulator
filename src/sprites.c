//*****************************************************************************
// Filename: sprites.c
// Author: Fisherman166
//
// Functions that deal with sprites
//
// ****************************************************************************

#include "sprites.h"
#include "VRAM.h"

#define PRIMARY_OAM_SIZE 256

static uint8_t primary_OAM[PRIMARY_OAM_SIZE];
static uint8_t primary_OAM_addr;

#define SECONDARY_OAM_SIZE 32
static uint8_t secondary_OAM[SECONDARY_OAM_SIZE];
static uint8_t secondary_OAM_addr;

static uint8_t oam_data; //r2004

static bool sprite0_on_next_scanline = 0;
static bool sprite0_on_current_scanline = 0;
static bool overflow_detection = 0;
static bool primary_overflow = 0;
static bool secondary_overflow = 0;
static bool sprite_overflow = 0;
static bool sprite_zero_hit = 0;

#define NUM_SPRITES 8
static uint8_t sprite_attributes[NUM_SPRITES];
static uint8_t sprite_x[NUM_SPRITES];
static uint8_t sprite_low_pattern[NUM_SPRITES];
static uint8_t sprite_high_pattern[NUM_SPRITES];

static bool is_odd_cycle(uint16_t dot) {
    return dot & 1;
}

static void set_secondary_OAM_address(uint8_t address) {
    secondary_OAM_addr = address;
}

static void increment_secondary_OAM_address() {
    secondary_OAM_addr = (secondary_OAM_addr + 1) & 0x1F;
}

static uint8_t read_secondary_OAM() {
    return secondary_OAM[secondary_OAM_addr];
}

void init_primary_OAM() {
    for(uint16_t addr = 0; addr < PRIMARY_OAM_SIZE + 1; addr++)
        primary_OAM[addr] = 0xFF;
}

void set_OAM_address(uint8_t address) {
    primary_OAM_addr = address;
}

uint8_t get_OAM_data() {
    return oam_data;
}

void clear_sprite0() {
    sprite0_on_next_scanline = false;
}

bool get_sprite_overflow() {
    return sprite_overflow;
}

void clear_sprite_overflow() {
    sprite_overflow = false;
}

void set_sprite_zero_hit() {
    sprite_zero_hit = true;
}

void clear_sprite_zero_hit() {
    sprite_zero_hit = false;
}

void write_primary_OAM(uint8_t data) {
    oam_data = data;
    primary_OAM[primary_OAM_addr++] = data;
}

void secondary_OAM_clear(uint16_t dot) {
    const uint8_t OAM_value = 0xFF;
    if(dot == 1)
        set_secondary_OAM_address(0);
    if(!is_odd_cycle(dot))
        secondary_OAM[secondary_OAM_addr++] = OAM_value;
}

static void increment_OAM_addresses() {
    primary_OAM_addr++;
    secondary_OAM_addr++;
    primary_overflow = (primary_OAM_addr == 0) ? true : false;
    secondary_overflow = (secondary_OAM_addr == 0) ? true : false;
    overflow_detection = (secondary_OAM_addr == 0) ? true : false;
}

static bool is_OAM_overflow() {
    return primary_overflow || secondary_overflow;
}

static bool sprite_in_range(
    uint16_t scanline,
    uint8_t y_cord,
    uint8_t sprite_size
) {
    uint16_t delta = scanline - y_cord;
    return delta < sprite_size;
}

// REFACTOR AFTER IT IS WORKING
void sprite_evaluation(uint16_t scanline, uint16_t dot, uint8_t sprite_size) {
    static uint8_t bytes_left_to_copy = 0;

    if(dot == 65) {
        primary_overflow = secondary_overflow = sprite_overflow = false;
        set_secondary_OAM_address(0);
    }
    if(is_odd_cycle(dot)) {
        oam_data = primary_OAM[primary_OAM_addr];
        return;
    }

    uint8_t const orig_oam_data = oam_data;

    if(!is_OAM_overflow())
        secondary_OAM[secondary_OAM_addr] = oam_data;
    else
        oam_data = secondary_OAM[secondary_OAM_addr];

    if(bytes_left_to_copy > 0) {
        bytes_left_to_copy--;
        increment_OAM_addresses();
        return;
    }

    bool const in_range = sprite_in_range(scanline, orig_oam_data, sprite_size);

    if(dot == 66)
        sprite0_on_next_scanline = in_range;

    if(in_range && !is_OAM_overflow()) {
        bytes_left_to_copy = 3;
        increment_OAM_addresses();
        return;
    }

    if(!overflow_detection) {
        primary_OAM_addr = (primary_OAM_addr + 4) & 0xFC;
        if(primary_OAM_addr == 0)
            primary_overflow = true;
    }
    else {
        if(in_range && !primary_overflow) {
            sprite_overflow = true;
            overflow_detection = false;
        }
        else {
            primary_OAM_addr = ((primary_OAM_addr + 4) & 0xFC) | ((primary_OAM_addr + 1) & 0x3);
            if((primary_OAM_addr & 0xFC) == 0)
                primary_overflow = true;
        }
    }
}

static uint16_t calc_8x8_low_sprite_tile_address(
    uint16_t scanline,
    uint16_t base_sprite_address,
    uint8_t sprite_y,
    uint8_t index,
    uint8_t attribute,
    bool* in_range
) {
    bool y_is_flipped = attribute & 0x80;
    uint16_t y_diff = scanline - sprite_y;
    uint16_t flipped_y_diff = ~y_diff;
    uint16_t address = base_sprite_address | (index << 4);
    if(y_is_flipped) {
        address |= flipped_y_diff & 0x7;
    }
    else {
        address |= y_diff & 0x7;
    }
    *in_range = y_diff < 8;
    return address;
}

static uint16_t calc_8x16_low_sprite_tile_address(
    uint16_t scanline,
    uint8_t sprite_y,
    uint8_t index,
    uint8_t attribute
) {
    bool y_is_flipped = attribute & 0x80;
    uint8_t y_diff = scanline - sprite_y;
    uint8_t flipped_y_diff = ~y_diff;

    uint16_t address = 0x1000 * (index & 1);
    address |= (index & 0xFE) << 4;
    if(y_is_flipped) {
        address |= (flipped_y_diff & 0x8) << 1;
        address |= flipped_y_diff & 0x7;
    }
    else {
        address |= (y_diff & 0x8) << 1;
        address |= y_diff & 0x7;
    }
    return address;
}

static uint16_t calc_high_sprite_tile_address(uint16_t low_sprite_address) {
    return low_sprite_address | 0x8;
}

static bool should_flip_sprite_horizontal(uint8_t attribute) {
    return attribute & 0x40;
}

static uint8_t rev_byte(uint8_t sprite_byte) {
    uint8_t bit_count = 8;
    bool bits[bit_count];
    uint8_t reversed = 0;

    uint8_t mask = 0x1;
    for(uint8_t bit_num = 0; bit_num < bit_count; bit_num++) {
        bits[bit_num] = sprite_byte & mask;
        mask <<= 1;
    }

    uint8_t shift_amount = 0;
    for(int bits_left = 7; bits_left >= 0; bits_left--) {
        reversed |= bits[bits_left] << shift_amount;
        shift_amount++;
    }
    return reversed;
}

void do_sprite_load(
    uint16_t scanline,
    uint16_t dot,
    uint16_t base_sprite_address,
    uint8_t sprite_size
) {
    static uint8_t cycle_count = 0;
    uint16_t const sprite_number = (dot - 257) / 8;
    static uint8_t sprite_y;
    static uint8_t sprite_index;
    static uint16_t low_sprite_address;
    static uint16_t high_sprite_address;
    static bool in_range;

    if(dot == 257)
        set_secondary_OAM_address(0);

    sprite0_on_current_scanline = sprite0_on_next_scanline;
    switch(cycle_count) {
        case 0:
            sprite_y = read_secondary_OAM();
            increment_secondary_OAM_address();
            break;
        case 1:
            sprite_index = read_secondary_OAM();
            increment_secondary_OAM_address();
            break;
        case 2:
            sprite_attributes[sprite_number] = read_secondary_OAM();
            increment_secondary_OAM_address();
            break;
        case 3:
            sprite_x[sprite_number] = read_secondary_OAM();
            increment_secondary_OAM_address();
            break;
        case 4:
            if(sprite_size == 8) {
                low_sprite_address = calc_8x8_low_sprite_tile_address(
                    scanline, 
                    base_sprite_address,
                    sprite_y,
                    sprite_index,
                    sprite_attributes[sprite_number],
                    &in_range
                );
            }
            else {
                low_sprite_address = calc_8x16_low_sprite_tile_address(
                    scanline, 
                    sprite_y,
                    sprite_index,
                    sprite_attributes[sprite_number]
                );
            }
            break;
        case 5:
            sprite_low_pattern[sprite_number] = in_range ? read_VRAM(low_sprite_address) : 0x00;
            if(should_flip_sprite_horizontal(sprite_attributes[sprite_number]))
                sprite_low_pattern[sprite_number] = rev_byte(sprite_low_pattern[sprite_number]);
            break;
        case 6:
            high_sprite_address = calc_high_sprite_tile_address(low_sprite_address);
            break;
        case 7:
            sprite_high_pattern[sprite_number] = in_range ? read_VRAM(high_sprite_address) : 0x00;
            if(should_flip_sprite_horizontal(sprite_attributes[sprite_number]))
                sprite_high_pattern[sprite_number] = rev_byte(sprite_high_pattern[sprite_number]);
            break;
    }

    cycle_count = (cycle_count + 1) & 0x7;
}

// REFACTOR THIS WHEN WORKING
#define NTH_BIT(x, n) (((x) >> (n)) & 1)
uint8_t get_sprite_pixel(uint8_t* spr_pal, bool* spr_behind_bg, bool* spr_is_s0, uint16_t dot) {
    uint16_t const pixel = dot - 1;

    for (uint8_t i = 0; i < 8; ++i) {
        uint16_t const offset = pixel - sprite_x[i];
        if (offset < 8) {
            uint8_t pat_res = (NTH_BIT(sprite_high_pattern[i], 7 - offset) << 1) |
                               NTH_BIT(sprite_low_pattern[i], 7 - offset);
            if (pat_res) {
                *spr_pal       = sprite_attributes[i] & 3;
                *spr_behind_bg = sprite_attributes[i] & 0x20;
                *spr_is_s0     = sprite0_on_current_scanline && (i == 0);
                return pat_res;
            }
        }
    }
    return 0;
}
