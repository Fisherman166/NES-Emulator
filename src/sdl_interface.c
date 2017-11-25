//*****************************************************************************
// Filename: sdl_interface.c
// Author: Fisherman166
//
// Functions that interact with SDL
//
// ****************************************************************************

#include "sdl_interface.h"

// Window stuff
static SDL_Window *screen = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

//*****************************************************************************
// Private functions
//*****************************************************************************
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
