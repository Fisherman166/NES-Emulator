//*****************************************************************************
// Filename: controller.h
// Author: Fisherman166
//
// Functions that get keyboard input and translate it into NES controller
// input
//
// ****************************************************************************

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

void update_controller_states();
bool should_quit();
void write_controller_strobe(bool);
uint8_t read_controller();

#endif //CONTROLLER_H
