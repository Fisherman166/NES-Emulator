#include <iostream>
#include <ctime>
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "SDL/SDL.h"	//These are for graphics
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
SDL_Surface *screen = NULL;

//Functions
void checkInput();						//Checks for input
void quitEmu();							//Quits the emulator
bool setPointers();						//Sets object pointers
Uint8* keyboard;						//Holds keyboars state

time_t startTime, endTime;					//Only used for debugging
double seconds;

int main(int argc, char *args[])
{	
	if( !setPointers() ) cout << "Error setting pointers" << endl;
	systemMemory->loadMemory();
	core->setPCStart();

	if( SDL_Init(SDL_INIT_EVERYTHING) < 0) cout << "Video init failed" << endl;
	screen = SDL_SetVideoMode(screenWidth, screenHeight, 24, SDL_HWSURFACE);

	if(screen == NULL)
		cout << "Get mode set failed: " << SDL_GetError() << endl;

	SDL_WM_SetCaption("NES Emulator by Dartht33bagger", NULL);

	uint32_t *pixels = (uint32_t*)screen->pixels;

	for(;;)
	{
		//Run the cpu for one instruction
		ppuCycles = core->emulateCycle();

		//3 PPU cycles per CPU cycle
		for(int i = (ppuCycles * 3); i > 0; i--)
			video->emulateCycle();

		//Checks for keyboard input
		checkInput();

		if(video->bufferVblank)	//Time to render a frame!
		{
			/*//Create surface from the array values
			next = SDL_CreateRGBSurfaceFrom((void*)video->pixels,
					screenWidth,
					screenHeight,
					24,			//24 bpp because no alpha
					screenWidth * 3,	//Width * channels, where channels = 3
					0xFF,			//Red mask
					0xFF00,			//Green mask
					0xFF0000,		//Blue mask
					0);			//Alpha mask (none)

			SDL_Rect offset;
			offset.x = 0;
			offset.y = 0;
			SDL_BlitSurface(next, NULL, screen, &offset);

			if( SDL_Flip(screen) < 0) cout << "Flip error" << endl;*/
			
			for(int y = 0; y < 224; ++y)                
                		for(int x = 0; x < 256; ++x)
					pixels[x + y * screen->w] = video->screenData[x][y][0]
								| (video->screenData[x][y][1] << 8)
								| (video->screenData[x][y][2] << 16);

			SDL_UpdateRect(screen, 0, 0, 0, 0);

			video->bufferVblank = false;
		}
	}
	
	return 0;
}

void checkInput()
{
	unsigned char reload = 0;		//Temp value that is updated every function call
	SDL_PumpEvents();
	keyboard = SDL_GetKeyState(NULL);
	
	if(keyboard[SDLK_ESCAPE]) quitEmu();	//Closes the emulator

	//Right
	if(keyboard[SDLK_d]) reload |= 0x80;

	//Left
	if(keyboard[SDLK_a]) reload |= 0x40;
	
	//Down
	if(keyboard[SDLK_s]) reload |= 0x20;

	//Up
	if(keyboard[SDLK_w]) reload |= 0x10;

	//Start
	if(keyboard[SDLK_RETURN]) reload |= 0x8;

	//Select
	if(keyboard[SDLK_l]) reload |= 0x4;

	//B
	if(keyboard[SDLK_LEFT]) reload |= 0x2;

	//A
	if(keyboard[SDLK_UP]) reload |= 0x1;

	//If strobe is high, reload the controller1 shift register
	if(systemMemory->RAM[0x4016] & 1) systemMemory->controller1 = reload;
}

//Used to exit the emulate when done running
void quitEmu()
{
	systemMemory->dumpVRAM();
	delete video, systemMemory, core;
	SDL_FreeSurface(screen);
	SDL_Quit();
}


bool setPointers()
{
	bool retval = true;

	//Setting the pointers
	if( !video->setPointer(systemMemory) ) retval = false;
	if( !systemMemory->setPointer(video) ) retval = false;
	if( !core->setPointers(systemMemory, video) ) retval = false;
	
	return retval;
}
