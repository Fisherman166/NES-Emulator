//*****************************************************************************
// Filename: controller.c
// Author: Fisherman166
//
// Functions that get keyboard input and translate it into NES controller
// input
//
// ****************************************************************************

#include "SDL2/SDL.h"
#include "controller.h"

typedef struct {
    bool up_pressed;
    bool down_pressed;
    bool left_pressed;
    bool right_pressed;
    bool A_pressed;
    bool B_pressed;
    bool start_pressed;
    bool select_pressed;
} controller_buttons;

static bool quit = false;
static controller_buttons controller1_state = {0, 0, 0, 0, 0, 0, 0, 0};
static bool strobe_latch = false;
static uint8_t controller_bits;

static uint8_t pack_button_state(controller_buttons* buttons) {
    const uint8_t A_shift = 0;
    const uint8_t B_shift = 1;
    const uint8_t select_shift = 2;
    const uint8_t start_shift = 3;
    const uint8_t up_shift = 4;
    const uint8_t down_shift = 5;
    const uint8_t left_shift = 6;
    const uint8_t right_shift = 7;

    uint8_t packed_buttons = 0;
    packed_buttons |= buttons->A_pressed << A_shift;
    packed_buttons |= buttons->B_pressed << B_shift;
    packed_buttons |= buttons->select_pressed << select_shift;
    packed_buttons |= buttons->start_pressed << start_shift;
    packed_buttons |= buttons->up_pressed << up_shift;
    packed_buttons |= buttons->down_pressed << down_shift;
    packed_buttons |= buttons->left_pressed << left_shift;
    packed_buttons |= buttons->right_pressed << right_shift;

    return packed_buttons;
} 

void update_controller_states() {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();

    controller1_state.up_pressed = keys[SDL_SCANCODE_W];
    controller1_state.down_pressed = keys[SDL_SCANCODE_S];
    controller1_state.left_pressed = keys[SDL_SCANCODE_A];
    controller1_state.right_pressed = keys[SDL_SCANCODE_D];
    controller1_state.A_pressed = keys[SDL_SCANCODE_UP];
    controller1_state.B_pressed = keys[SDL_SCANCODE_LEFT];
    controller1_state.start_pressed = keys[SDL_SCANCODE_RETURN];
    controller1_state.select_pressed = keys[SDL_SCANCODE_L];
    quit = keys[SDL_SCANCODE_ESCAPE];
}

bool should_quit() {
    return quit;
}

void write_controller_strobe(bool strobe) {
    // Update continously while strobe is high. Or just cheat it when
    // strobe goes low
    if(strobe_latch && !strobe)
        controller_bits = pack_button_state(&controller1_state);
    strobe_latch = strobe;
}

uint8_t read_controller() {
    uint8_t result;
    if(strobe_latch)
        result = controller1_state.A_pressed;
    else {
        result = controller_bits & 1;
        // The NES shifts 1s into the MSB
        controller_bits = 0x80 | (controller_bits >> 1);
    }
    return result;
}
