#ifndef PPU_H
#define PPU_H

#include <iostream>
#include <fstream>
#include "memory.h"

class ppu
{
public:
	ppu();
	~ppu();

	void emulateCycle(memory*);
	void setIdleCounter(unsigned short);

	unsigned char screenData[224][256][3];					//Holds the RGB values of the screen
	unsigned short ppuAddress, ppuTempAddress;				//Holds the addresses
	unsigned char fineXScroll;						//For scrolling
	unsigned short dotNumber;						//What the current dot the PPU is running
	unsigned short scanline;						//Current scanline

	bool writeToggle;							//For $2005 and $2006
	bool vblank,bufferVblank;						//Buffervblank is for rendering a frame
	bool NMI;
	
private:
	//PPU status
	bool oddFrame;								//Used to keep track of even and odd frame
	bool ntFetch;								//Next operation will be nametable fetch
	bool atFetch;								//Next operation will be attribute fetch
	bool bgLowFetch;							//Next operation will be low or high tile fetch
	unsigned short idleCounter;						//Current idle cycles to wait
	unsigned short reloadDot;						//Allows PPU to keep track of when registers need to be 										reloaded
	unsigned short horizontalDot;						//Allows PPU to check when coarse X needs to be 										incremented
	unsigned short coarseY;
	

	//For background rendering
	unsigned short 	highBGShift, lowBGShift;		//Hold the higher and lower background tiles
	unsigned char	highBGFetch,lowBGFetch;			//Holds the two fetches for the shifter
	unsigned char 	highAttShift, lowAttShift;		//Shift registers for attribute bytes that apply to the BG tiles
	unsigned char	attFetch, nameFetch;			//Used in attrtibute fetch and nametable fetch
	unsigned short 	tileAddress, attAddress;		//Addresses for these two things
	unsigned short 	nameAddress;
	
	//For Pixel Rendering
	unsigned short palleteAddress;					//Holds the pallete address for the pixel
	unsigned char palleteData;						//The pallete number to be looked up in pallete arrays
	

	//I/O register data 
	unsigned char reg2000;						//$2000
	unsigned char reg2001;						//$2001
	unsigned short reg2002 = 0x2002;				//Holds the address for $2002

	//$2002
	bool spriteOverflow;						//Set if > 8 sprites on scanline
	bool zeroFlag;								//Sprite zero hit flag
	unsigned char vblankValue = 0x80;			//Holds the vblank value


	//Functions
	const void checkDotNumber();				//Does different things on different dot numbers
	const void shiftRegisters();				//Shifts the shifting registers
	const void renderPixel(memory*);			//Puts pixel data into 
	const void checkVblank(memory*);
	
	//Used for different scanlines
	const void visableScanline(memory*);
	const void visableBGFetch(memory*);
	const void preBGFetch(memory*);
	
	//Muxes
	const void fourToOneMux();					//Used when loading attribute shift register
	const bool eightToOneMux1(unsigned short&);			//Used when picking four bits for a pixel.
	const bool eightToOneMux2(unsigned char&);

	//Debug stuff
	std::ofstream ppuDebug;


	//Pallete colors
	unsigned char red[0x40] =	{
	0x75, 0x27, 0x00, 0x47, 0x8F, 0xAB, 0xA7, 0x7F, 0x43, 0x00, 0x00,	//0x0A
	0x00, 0x1B, 0x00, 0x00, 0x00, 0xBC, 0x00, 0x23, 0x83, 0xBF, 0xE7,	//0x15
	0xDB, 0xCB, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,	//0x20 
	0x3F, 0x5F, 0xA7, 0xF7, 0xFF, 0xFF, 0xFF, 0xF3, 0x83, 0x4F, 0x58, 	//0x2B
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xAB, 0xC7, 0xD7, 0xFF, 0xFF, 0xFF, 	//0x36
	0xFF, 0xFF, 0xE3, 0xAB, 0xB3, 0x9F, 0x00, 0x00, 0x00};

	unsigned char green[0x40] =	{
	0x75, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x2F, 0x47, 0x51,	//0x0A
	0x3F, 0x3F, 0x00, 0x00, 0x00, 0xBC, 0x73, 0x3B, 0x00, 0x00, 0x00,	//0x15
	0x2B, 0x4F, 0x73, 0x97, 0xAB, 0x93, 0x83, 0x00, 0x00, 0x00, 0xFF,	//0x20 
	0xBF, 0x97, 0x8B, 0x7B, 0x77, 0x77, 0x9B, 0xBF, 0xD3, 0xDF, 0xF8, 	//0x2B
	0xEB, 0x00, 0x00, 0x00, 0xFF, 0xE7, 0xD7, 0xCB, 0xC7, 0xC7, 0xBF, 	//0x36
	0xDB, 0xE7, 0xFF, 0xF3, 0xFF, 0xFF, 0x00, 0x00, 0x00};

	unsigned char blue[0x40] =	{
	0x75, 0x8F, 0xAB, 0x9F, 0x77, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,	//0x0A
	0x17, 0x5F, 0x00, 0x00, 0x00, 0xBC, 0xEF, 0xEF, 0xF3, 0xBF, 0x5B,	//0x15
	0x00, 0x0F, 0x00, 0x00, 0x00, 0x3B, 0x8B, 0x00, 0x00, 0x00, 0xFF,	//0x20 
	0xFF, 0xFF, 0xFD, 0xFF, 0xB7, 0x63, 0x3B, 0x3F, 0x13, 0x4B, 0x98, 	//0x2B
	0xDB, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xB3, 	//0x36
	0xAB, 0xA3, 0xA3, 0xBF, 0xCF, 0xF3, 0x00, 0x00, 0x00};
};

#endif
