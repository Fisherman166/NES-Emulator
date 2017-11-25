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
void render_frame(uint32_t*);
void exit_SDL();

// Input functions
void update_controller_states();
bool should_quit();
void write_controller_strobe(bool);
uint8_t read_controller();

#endif // SDL_INTERFACE_H

