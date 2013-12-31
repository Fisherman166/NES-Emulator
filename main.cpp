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
SDL_Surface *next = NULL;

//Functions
bool checkInput();						//Checks for input
void quitEmu();							//Quits the emulator
bool setPointers();						//Sets object pointers
Uint8* keyboard;						//Holds keyboars state

time_t startTime, endTime;					//Only used for debugging
double seconds;

int main(int argc, char *args[])
{	
	//Initalize objects
	if( !setPointers() ) cout << "Error setting pointers" << endl;
	systemMemory->loadMemory();
	core->setPCStart();

	if( SDL_Init(SDL_INIT_EVERYTHING) < 0) cout << "Video init failed" << endl;
	screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE);

	if(screen == NULL)
		cout << "Get mode set failed: " << SDL_GetError() << endl;

	SDL_WM_SetCaption("NES Emulator by Dartht33bagger", NULL);

	//Create initial surface for next
	next = SDL_CreateRGBSurface(SDL_HWSURFACE, screen->w, screen->h, screen->format->BitsPerPixel, screen->format->Rmask,
					screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	
	//Create a pointer to the next pixels
	Uint32 *pixels = (Uint32*)next->pixels;

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
			//If the screen has to be locked, lock it.
			if( SDL_MUSTLOCK(next) )
				SDL_LockSurface(next);

			//Fill in next's pixel data from the ppu array
			for(int x = 0; x < 224; x++)                
                		for(int y = 0; y < 256; y++)
					pixels[y + x * next->w] = video->screenData[y][x];

			if(SDL_MUSTLOCK(next)) 
				SDL_UnlockSurface(next);

			//Set x and y offset to 0
			SDL_Rect offset;
			offset.x = 0;
			offset.y = 0;

			SDL_BlitSurface(next, NULL, screen, &offset);

			//Draw the new screen
			SDL_Flip(screen);

			video->bufferVblank = false;
		}
	}

	//Cleanup
	quitEmu();
	
	return 0;
}

bool checkInput()
{
	unsigned char reload = 0;		//Temp value that is updated every function call
	bool quit = false;
	SDL_PumpEvents();
	keyboard = SDL_GetKeyState(NULL);
	
	if(keyboard[SDLK_ESCAPE]) quit = true;	//Closes the emulator

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

	return quit;
}

//Used to exit the emulate when done running
void quitEmu()
{
	//systemMemory->dumpVRAM();
	delete video, systemMemory, core;
	SDL_FreeSurface(screen);
	SDL_FreeSurface(next);
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
