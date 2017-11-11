#include "sprites.h"

#define PRIMARY_OAM_SIZE 256

static uint8_t primary_OAM[PRIMARY_OAM_SIZE];
static uint8_t primary_OAM_addr;


void set_OAM_address(uint8_t address) {
    primary_OAM_addr = address;
}

// TODO - This doesn't increment if in vblank
uint8_t get_OAM_address() {
    primary_OAM_addr++;
    return primary_OAM_addr - 1;
}

void write_primary_OAM(uint8_t data) {
    primary_OAM[primary_OAM_addr++] = data;
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

