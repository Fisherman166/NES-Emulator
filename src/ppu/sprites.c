#include "sprites.h"

#define PRIMARY_OAM_SIZE 256
#define SECONDARY_OAM_SIZE 32

static uint8_t primary_OAM[PRIMARY_OAM_SIZE];
static uint8_t primary_OAM_addr;

static uint8_t secondary_OAM[SECONDARY_OAM_SIZE];
static uint8_t secondary_OAM_addr;

static uint8_t oam_data; //r2004

static bool sprite0_on_next_scanline = 0;
static bool overflow_detection = 0;
static bool primary_overflow = 0;
static bool secondary_overflow = 0;
static bool sprite_overflow = 0;
static uint8_t sprite_number = 0;

// This can be removed after it is working
static void print_secondary_OAM() {
    for(uint16_t addr = 0; addr < SECONDARY_OAM_SIZE; addr += 0x10) {
        printf("%02X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n",
            addr, secondary_OAM[addr], secondary_OAM[addr + 1], secondary_OAM[addr + 2],
            secondary_OAM[addr + 3], secondary_OAM[addr + 4], secondary_OAM[addr + 5],
            secondary_OAM[addr + 6], secondary_OAM[addr + 7], secondary_OAM[addr + 8],
            secondary_OAM[addr + 9], secondary_OAM[addr + 10], secondary_OAM[addr + 11],
            secondary_OAM[addr + 12], secondary_OAM[addr + 13], secondary_OAM[addr + 14],
            secondary_OAM[addr + 15]);
    }
    printf("\n");
}




static bool is_odd_cycle(uint16_t dot) {
    return dot & 1;
}

static void set_secondary_OAM_address(uint8_t address) {
    secondary_OAM_addr = address;
}

void set_OAM_address(uint8_t address) {
    primary_OAM_addr = address;
}

uint8_t get_OAM_data() {
    return oam_data;
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
    return (scanline - y_cord) < sprite_size;
}

// REFACTOR AFTER IT IS WORKING
void sprite_evaluation(uint16_t scanline, uint16_t dot, uint8_t sprite_size) {
    static uint8_t bytes_left_to_copy = 0;
    if(dot == 256) {
        print_secondary_OAM();
    }

    if(dot == 65) {
        primary_overflow = secondary_overflow = sprite_overflow = false;
        set_secondary_OAM_address(0);
    }
    if(is_odd_cycle(dot)) {
        oam_data = primary_OAM[primary_OAM_addr];
        return;
    }

    if(!is_OAM_overflow())
        secondary_OAM[secondary_OAM_addr] = oam_data;
    else 
        oam_data = secondary_OAM[secondary_OAM_addr];

    if(bytes_left_to_copy > 0)
        bytes_left_to_copy--;
        increment_OAM_addresses();
        return;

    bool const in_range = sprite_in_range(scanline, oam_data, sprite_size);

    if(dot == 66)
        sprite0_on_next_scanline = in_range;

    if(in_range && !is_OAM_overflow()) {
        bytes_left_to_copy = 3;
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

#ifdef DEBUG
void print_primary_OAM() {
    for(uint16_t addr = 0; addr < 256; addr += 0x10) {
        printf("%02X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n",
            addr, primary_OAM[addr], primary_OAM[addr + 1], primary_OAM[addr + 2],
            primary_OAM[addr + 3], primary_OAM[addr + 4], primary_OAM[addr + 5],
            primary_OAM[addr + 6], primary_OAM[addr + 7], primary_OAM[addr + 8],
            primary_OAM[addr + 9], primary_OAM[addr + 10], primary_OAM[addr + 11],
            primary_OAM[addr + 12], primary_OAM[addr + 13], primary_OAM[addr + 14],
            primary_OAM[addr + 15]);
    }
    printf("\n");
}
#endif
