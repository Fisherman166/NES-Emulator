#ifndef PPU_H
#define PPU_H

#include <iostream>
#include <fstream>
#include "memory.h"

using namespace std;

class ppu
{
public:
	typedef unsigned char byte;
	typedef unsigned short word;
	ppu();
	~ppu();

	//Functions
	void emulateCycle();			//Render one pixel per cycle
	void printDebug();			    //Outputs addresses and values.
	bool setPointer(memory*);

	int screenData[240][256];		//Holds RGB data for the screen
	word ppuAddress, ppuTempAddress;	//Holds the addresses
	byte fineXScroll;			    //For scrolling
	word dotNumber;				    //What the current dot the PPU is running
	word scanline;				    //Current scanline

	bool writeToggle;			//For $2005 and $2006
	bool vblank,bufferVblank;	//Buffervblank is for rendering a frame
	bool NMI;				    //Used to signal a NMI

	//Pallete colors
	static int const RGB[];
	
private:
	memory* VRAM;				//Pointer to memory object

	//PPU status
	bool oddFrame;				//Used to keep track of even and odd frame
	bool bgHighFetch;			//Next operation will be low or high tile fetch
	word idleCounter;			//Current idle cycles to wait

	//For background rendering
	word 	highBGShift, lowBGShift;	//Hold the higher and lower background tiles
	byte	high_tile_byte,low_tile_byte;		//Holds the two fetches for the shifter
	word 	highAttShift, lowAttShift;	//Shift registers for attribute bytes that apply to the BG tiles
	byte	attribute_byte, nametable_byte;		//Used in attrtibute fetch and nametable fetch
	word 	tileAddress, attAddress;	//Addresses for these two things
	word 	nameAddress;	
	
	//For Pixel Rendering
	word palleteAddress;			//Holds the pallete address for the pixel
	byte palleteData;			//The pallete number to be looked up in pallete arrays

	//Sprites
	word pOAMAddress;			//Primary OAM adress
	byte sOAMAddress;			//Secondary OAM address
	byte sprite_number;
	bool spriteWrite;			//Keeps track if secondary OAM full
	bool spriteActive;			//Signals if a sprite is active
	byte spritesLow[8];			//Holds lower sprite data
	byte spritesHigh[8];			//Holds upper sprite data
	byte spriteAtt[8];			//Holds attribute bytes for the sprites
	word spriteXPos[8];			//X position of the sprite
	byte spriteCounter[8];			//How many pixels the sprite has left to render
	word spriteAddress;			//Pattern table address of the tile
	

	//I/O register data 
	byte reg2000;				//$2000
	byte reg2001;				//$2001
	word reg2002;				//Holds the address for $2002

	//$2002
	bool spriteOverflow;			//Set if > 8 sprites on scanline
	bool zeroFlag;				//Sprite zero hit flag
	byte vblankValue;			//Holds the vblank value

	//Functions
	const void shiftRegisters();	//Shifts the shifting registers
    const void reload_registers();  //Reloads the shift registers
	const void renderPixel();		//Puts pixel data into 
	const void checkVblank();		//Checks if vblank is occuring
	const void spriteEval();		//Used for sprite evaluation
    const void incrementY();        //Used to increment the y cord
    const void incrementX();        //Used to increment the x cord
    const void copyX();             //Copies temp X to ppuAddress X
    const void copyY();             //Copies temp Y to ppuAddress Y
	
    //Functions that fetch data
	const void backgroundFetch();
    const void nametable_fetch();
    const void attribute_fetch();
    const void low_background_fetch();
	
	//Muxes
	const void fourToOneMux();		//Used when loading attribute shift register
	const bool eightToOneMux(word&);	//Used when picking four bits for a pixel.

};

#endif

