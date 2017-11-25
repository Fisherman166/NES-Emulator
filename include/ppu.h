#ifndef PPU_H
#define PPU_H

void init_ppu();

bool run_PPU_cycle();
uint32_t* get_pixel_data_ptr();

uint16_t get_scanline(); 
uint16_t get_dot();

bool get_NMI_flag();
void clear_nmi_flag();

#endif

