#ifndef SPRITES_H
#define SPRITES_H

#include "common.h"

void set_OAM_address(uint8_t);
void write_primary_OAM(uint8_t);
uint8_t get_OAM_data();
void clear_sprite0();

void secondary_OAM_clear(uint16_t);
void sprite_evaluation(uint16_t, uint16_t, uint8_t);
void do_sprite_load(uint16_t, uint16_t, uint16_t, uint8_t);

#ifdef DEBUG
void print_primary_OAM();
#endif

#endif

