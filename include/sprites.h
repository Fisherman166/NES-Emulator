//*****************************************************************************
// Filename: sprites.h
// Author: Fisherman166
//
// Functions that deal with sprites
//
// ****************************************************************************

#ifndef SPRITES_H
#define SPRITES_H

#include "common.h"

void init_primary_OAM();

void set_OAM_address(uint8_t);
void write_primary_OAM(uint8_t);
uint8_t get_OAM_data();

void clear_sprite0();
void clear_sprite_overflow();
bool get_sprite_overflow();
void set_sprite_zero_hit();
void clear_sprite_zero_hit();

void secondary_OAM_clear(uint16_t);
void sprite_evaluation(uint16_t, uint16_t, uint8_t);
void do_sprite_load(uint16_t, uint16_t, uint16_t, uint8_t);
uint8_t get_sprite_pixel(uint8_t*, bool*, bool*, uint16_t);

#endif //SPRITES_H
