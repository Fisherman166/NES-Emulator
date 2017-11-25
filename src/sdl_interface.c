//*****************************************************************************
// Filename: sdl_interface.c
// Author: Fisherman166
//
// Functions that interact with SDL
//
// ****************************************************************************

#include "sdl_interface.h"

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

// Window stuff
static SDL_Window *screen = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

//*****************************************************************************
// Private functions
//*****************************************************************************
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

static bool clear_renderer(SDL_Renderer* renderer) {
    const uint8_t red = 0;
    const uint8_t green = 0;
    const uint8_t blue = 0;
    if(SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE)) {
        printf("ERROR: Renderer: %s\n", SDL_GetError());
        return true;
    }
    if(SDL_RenderClear(renderer)) {
        printf("ERROR: Renderer: %s\n", SDL_GetError());
        return true;
    }
    SDL_RenderPresent(renderer);
    return false;
}

//*****************************************************************************
// Public functions
//*****************************************************************************
bool init_SDL() {
    if( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        printf("ERROR: SDL failed to init: %s\n", SDL_GetError());
        return true;
    }

    screen = SDL_CreateWindow("NES EMU by Fisherman166",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640, 480,
                              SDL_WINDOW_OPENGL);
    if(screen == NULL) {
        printf("ERROR: Failed to create SDL WINDOW %s\n", SDL_GetError());
        return true;
    }

    // Renderer stuff
    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        printf("ERROR: %s\n", SDL_GetError());
        return true;
    }
    if(clear_renderer(renderer)) return true;

    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear") == SDL_FALSE) {
        printf("ERROR: HINT: %s\n", SDL_GetError());
        return true;
    }
    if(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) {
        printf("ERROR: Set Logical Size: %s\n", SDL_GetError());
        return true;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);
    if(texture == NULL) {
        printf("ERROR: Texture creation: %s\n", SDL_GetError());
        return true;
    }

    return false;
}

void render_frame(uint32_t* pixel_data) {
    SDL_UpdateTexture(texture, NULL, (void*)pixel_data, SCREEN_WIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void exit_SDL() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();
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
