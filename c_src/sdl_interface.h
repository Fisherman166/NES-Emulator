//*****************************************************************************
// Filename: sdl_interface.h
// Author: Fisherman166
//
// Functions that interact with SDL
//
// ****************************************************************************

#ifndef SDL_INTERFACE_H
#define SDL_INTERFACE_H

#include "SDL2/SDL.h"
#include "common.h"

bool init_SDL();
void exit_SDL();

// Input functions
bool check_input(uint8_t);
void enable_controller_strobe(uint8_t);
void disable_controller_strobe(uint8_t);

#endif // SDL_INTERFACE_H

