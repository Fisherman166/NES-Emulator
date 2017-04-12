#ifndef PPU_H
#define PPU_H

bool get_NMI_flag();
uint32_t (*get_pixel_data_ptr())[256];
bool run_PPU_cycle();

#endif

