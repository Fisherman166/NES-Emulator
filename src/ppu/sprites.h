#ifndef SPRITES_H
#define SPRITES_H

#include "common.h"

void set_OAM_address(uint8_t);
uint8_t get_OAM_address();
void write_primary_OAM(uint8_t);

void secondary_OAM_clear(uint16_t);

#ifdef DEBUG
void print_primary_OAM();
#endif

#endif

