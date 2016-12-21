//*****************************************************************************
// Filename: sdl_interface.c
// Author: Fisherman166
//
// Functions that interact with SDL
//
// ****************************************************************************

#include "sdl_interface.h"
#include "memory_operations.h"

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

static controller_buttons controller1_state = {0, 0, 0, 0, 0, 0, 0, 0};
static bool controller_strobe[2] = {0, 0};

// Window stuff
static SDL_Window *screen = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static SDL_Event keyboard_event;

//*****************************************************************************
// Private functions
//*****************************************************************************
static bool get_controller_strobe_state(uint8_t controller) {
    return controller_strobe[controller];
}

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

void exit_SDL() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();
}

bool check_input(uint8_t controller) {
    if( (controller < 0) || (controller > NUM_OF_JOYPADS) ) printf("ERROR: bad controller number in check_input: %u\n", controller);
    bool quit = false;

    while(SDL_PollEvent(&keyboard_event)) {
        if(keyboard_event.key.repeat != 0) continue;

        bool key_pressed = 0;
        if(keyboard_event.type == SDL_KEYDOWN) key_pressed = 1;

        switch(keyboard_event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if(key_pressed) quit = true;
                break;
            case SDLK_w:
                controller1_state.up_pressed = key_pressed;
                break;
            case SDLK_s:
                controller1_state.down_pressed = key_pressed;
                break;
            case SDLK_a:
                controller1_state.left_pressed = key_pressed;
                break;
            case SDLK_d:
                controller1_state.right_pressed = key_pressed;
                break;
            case SDLK_UP:
                controller1_state.A_pressed = key_pressed;
                break;
            case SDLK_LEFT:
                controller1_state.B_pressed = key_pressed;
                break;
            case SDLK_RETURN:
                controller1_state.start_pressed = key_pressed;
                break;
            case SDLK_l:
                controller1_state.select_pressed = key_pressed;
                break;
        }
    }

    // FIXME - Revist when PPU is working
    /*static bool last_stobe = false;
    bool strobe = get_controller_strobe_state(controller);
    if( (last_stobe == true) && (strobe == false) ) {
        uint8_t packed_buttons = pack_button_state(&controller1_state);
        write_RAM(JOYPAD1_ADDRESS, packed_buttons);
    }*/

    return quit;
}

void enable_controller_strobe(uint8_t controller) {
    printf("HERE\n");
    if( (controller < 0) || (controller > NUM_OF_JOYPADS) ) return;
    controller_strobe[controller] = true;
}

void disable_controller_strobe(uint8_t controller) {
    if( (controller < 0) || (controller > NUM_OF_JOYPADS) ) return;
    controller_strobe[controller] = false;
}

