#ifndef MEMORY_H
#define MEMORY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>

class ppu;							//Foward declaration

class memory
{
public:
	memory();
	~memory();	
	
	//CPU
	unsigned char RAM[0xFFFF]; 				//Memory for the system

	//PPU
	unsigned char VRAM[0x4000];				//Memory for the PPU. 16384 bytes large
	unsigned char OAM[0xFF];				//256 byte area for sprites
	char header[16];					//Holds the iNES header

	//Functions
	void writeRAM(unsigned short&, unsigned char&, ppu*); 	//Writes to RAM
	unsigned char readRAM(unsigned short, ppu*);		//Reads from RAM
	void writeVRAM(unsigned short, unsigned char&);		//Writes to VRAM
	unsigned char readVRAM(unsigned short&);
	void clearDMA();					//Clears the DMA flag
	bool loadMemory();
	void dump();						//Dumps $6000 to $7FFF

	//Data peices
	bool DMAFlag;						//Flag for if a DMA has occured

private:
	unsigned char mapper;					//Holds the mapper number
	char*	memBlock;					//Holds the temporary cartridge data
	std::ifstream game;					//Loads the game
	std::ofstream debug;
	bool	horizontalMirror;				//0 = vertical mirroring, 1 = horizontal mirroring
	unsigned char	readBuffer;				//The intertal buffer for $2007 reads

	//Functions
	const void DMA(unsigned char&);				//Used in a DMA

	//Mapper functions
	const void NROM();					//Used for mapper 0x00
};

#endif
