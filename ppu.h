#ifndef PPU_H
#define PPU_H

#include <iostream>
#include <fstream>
#include "memory.h"

class ppu
{
public:
	typedef unsigned char byte;
	typedef unsigned short word;
	ppu();
	~ppu();

	//Functions
	void emulateCycle();
	void printDebug();						//Outputs addresses and values.
	bool setPointer(memory*);

	int screenData[256][224];					//Holds RGB data for the screen
	word ppuAddress, ppuTempAddress;				//Holds the addresses
	byte fineXScroll;						//For scrolling
	word dotNumber;							//What the current dot the PPU is running
	word scanline;							//Current scanline

	bool writeToggle;						//For $2005 and $2006
	bool vblank,bufferVblank;					//Buffervblank is for rendering a frame
	bool NMI;							//Used to signal a NMI

	//Pallete colors
	static int const RGB[];
	
private:
	memory* VRAM;							//Pointer to memory object

	//PPU status
	bool oddFrame;							//Used to keep track of even and odd frame
	bool ntFetch;							//Next operation will be nametable fetch
	bool atFetch;							//Next operation will be attribute fetch
	bool bgLowFetch;						//Next operation will be low or high tile fetch
	word idleCounter;						//Current idle cycles to wait
	word reloadDot;							//Allows PPU to keep track of when registers need to be reloaded
	word horizontalDot;						//Allows PPU to check when coarse X needs to be incremented
	word coarseY;

	//For background rendering
	word 	highBGShift, lowBGShift;			//Hold the higher and lower background tiles
	byte	highBGFetch,lowBGFetch;				//Holds the two fetches for the shifter
	word 	highAttShift, lowAttShift;			//Shift registers for attribute bytes that apply to the BG tiles
	byte	attFetch, nameFetch;				//Used in attrtibute fetch and nametable fetch
	word 	tileAddress, attAddress;			//Addresses for these two things
	word 	nameAddress;	
	
	//For Pixel Rendering
	word palleteAddress;						//Holds the pallete address for the pixel
	byte palleteData;						//The pallete number to be looked up in pallete arrays
	

	//I/O register data 
	byte reg2000;						//$2000
	byte reg2001;						//$2001
	word reg2002 = 0x2002;					//Holds the address for $2002

	//$2002
	bool spriteOverflow;					//Set if > 8 sprites on scanline
	bool zeroFlag;						//Sprite zero hit flag
	byte vblankValue = 0x80;				//Holds the vblank value


	//Functions
	const void checkDotNumber();				//Does different things on different dot numbers
	const void shiftRegisters();				//Shifts the shifting registers
	const void renderPixel();				//Puts pixel data into 
	const void checkVblank();				//Checks if vblank is occuring
	
	//Used for different scanlines
	const void backgroundFetch();
	//const void preBGFetch();
	
	//Muxes
	const void fourToOneMux();				//Used when loading attribute shift register
	const bool eightToOneMux(word&);			//Used when picking four bits for a pixel.
	const bool eightToOneMux(byte&);

	//Debug stuff
	std::ofstream ppuDebug;

};

#endif
