#include <iostream>
#include <ctime>
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "SDL2/SDL.h"	//These are for graphics
#include "GL/gl.h"
#include "GL/glu.h"

using namespace std;
bool run = true;
bool flag = false;
int screenWidth = 256;
int screenHeight = 224;

//Objects
memory* systemMemory = new memory();
cpu* core = new cpu();
ppu* video = new ppu();
unsigned char ppuCycles;					//Number of ppu cycles to run

//For rendering
SDL_Window *screen = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

//Video functions
void initSDL();

//Functions
bool checkInput();						//Checks for input
void quitEmu();							//Quits the emulator
bool setPointers();						//Sets object pointers
const Uint8* keyboard;						//Holds keyboars state

//Debugging stuff
time_t startTime, endTime;
double seconds;
bool startStep = false;
bool spaceHit = false;
bool printDebug = true;

int main(int argc, char *args[])
{	
	//Initalize objects
	if( !setPointers() ) cout << "Error setting pointers" << endl;
	systemMemory->loadMemory();
	core->setPCStart();

	//Setup everything for SDL
	initSDL();

	int *pixels = &video->screenData[0][0];

	for(;;)
	{
		//Run the cpu for one instruction
		ppuCycles = core->emulateCycle();

		//3 PPU cycles per CPU cycle
		for(int i = (ppuCycles * 3); i > 0; i--)
			video->emulateCycle();

		//Checks for keyboard input and stops the loop if esc hit
		if(checkInput()) break;

		
		if(video->bufferVblank)	//Time to render a frame!
		{
			//Update the texture from screen data
			SDL_UpdateTexture(texture, NULL, (void*)pixels, 256 * 4);

			//Render updated texture
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			video->bufferVblank = false;
		}
	}

	//Cleanup
	quitEmu();
	
	return 0;
}

//Sets everything up for SDL
void initSDL()
{
	if( SDL_Init(SDL_INIT_EVERYTHING) < 0) cout << "Video init failed" << endl;

	//Creates our screen
	screen = SDL_CreateWindow("NES Emulator by Dartht33bagger",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				0,
				0,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	//Creates our renderer
	renderer = SDL_CreateRenderer(screen, -1, 0);
	
	//Error checking
	if(screen == NULL || renderer == NULL)
		cout << "Opening window failed." << SDL_GetError() << endl;

	//Clear the window
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");	//Scale linearly
	SDL_RenderSetLogicalSize(renderer, 256, 224);

	//Create a texture
	texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			256, 224);

	if(texture == NULL) cout << "Error creating texture" << endl;

	//Get pointer to keyboard state
	keyboard = SDL_GetKeyboardState(NULL);
}

bool checkInput()
{
	unsigned char reload = 0;		//Temp value that is updated every function call
	bool quit = false;

	SDL_PumpEvents();			//Update keyboard states
	
	if(keyboard[SDL_SCANCODE_ESCAPE]) quit = true;	//Closes the emulator

	if(keyboard[SDLK_p]) startStep = true;	//Starts stepping

	while(startStep == true && spaceHit == false)
	{
		if(printDebug)
		{
			printDebug = false;
			core->printDebug();
			video->printDebug();	
		}	

		if(keyboard[SDLK_p]) startStep = false;	//Starts stepping
		if(keyboard[SDLK_SPACE]) spaceHit = true;	//Starts stepping
	}

	spaceHit = false;
	printDebug = true;

	//Right
	if(keyboard[SDL_SCANCODE_D]) reload |= 0x80;

	//Left
	if(keyboard[SDL_SCANCODE_A]) reload |= 0x40;
	
	//Down
	if(keyboard[SDL_SCANCODE_S]) reload |= 0x20;

	//Up
	if(keyboard[SDL_SCANCODE_W]) reload |= 0x10;

	//Start
	if(keyboard[SDL_SCANCODE_RETURN]) reload |= 0x8;

	//Select
	if(keyboard[SDL_SCANCODE_L]) reload |= 0x4;

	//B
	if(keyboard[SDL_SCANCODE_LEFT]) reload |= 0x2;

	//A
	if(keyboard[SDL_SCANCODE_UP]) reload |= 0x1;

	//If strobe is high, reload the controller1 shift register
	if(systemMemory->RAM[0x4016] & 1) systemMemory->controller1 = reload;

	return quit;
}

//Used to exit the emulate when done running
void quitEmu()
{
	systemMemory->dumpVRAM();
	delete video, systemMemory, core;
	SDL_Quit();
}

//Sets the pointers inside of the classes
bool setPointers()
{
	bool retval = true;

	//Setting the pointers
	if( !video->setPointer(systemMemory) ) retval = false;
	if( !systemMemory->setPointer(video) ) retval = false;
	if( !core->setPointers(systemMemory, video) ) retval = false;
	
	return retval;
}
