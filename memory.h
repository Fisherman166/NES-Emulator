#ifndef MEMORY_H
#define MEMORY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

class ppu;						//Foward declaration

class memory
{
public:
	typedef unsigned char byte;
	typedef unsigned short word;			//Two bytes

	memory();
	~memory();	
	
	//CPU
	byte RAM[0x10000]; 				//Memory for the system
	byte *RAMPTR[0x10000];				//Pointers for RAM

	//PPU
	byte chrRom[0x2001];				//$0000-$1FFF in VRAM. Holds CHR-ROM
	byte nameTable1[0x401];				//$2000-$23FF in VRAM. Holds first nametable
	byte nameTable2[0x401];				//$2400-$27FF in VRAM. Holds second nametable
	byte pallete[0x21];				//$3F00-$3F1F in VRAM. Holds pallete for sprite and BG
	byte *VRAMPTR[0x4001];				//Pointers for VRAM
	byte OAM[0x100];				//256 byte area for sprites
	char header[16];				//Holds the iNES header

	//Functions
	void writeRAM(word&, byte&, ppu*); 	//Writes to RAM
	byte readRAM(word, ppu*);		//Reads from RAM
	void writeVRAM(word, byte&);		//Writes to VRAM
	byte readVRAM(word&);

	void clearDMA();				//Clears the DMA flag
	bool loadMemory();
	void dumpRAM();					//Dumps $6000 to $7FFF
	void dumpVRAM();					
	void setMirror(bool);				//True = horizontal, false = vertical

	//Data peices
	bool DMAFlag;					//Flag for if a DMA has occured

private:
	byte mapper;					//Holds the mapper number
	char*	memBlock;				//Holds the temporary cartridge data
	std::ifstream game;				//Loads the game
	std::ofstream debug;
	int gameSize;
	bool	horizontalMirror;			//0 = vertical mirroring, 1 = horizontal mirroring
	byte	readBuffer;				//The intertal buffer for $2007 reads

	//Functions
	const void DMA(byte&);				//Used in a DMA

	//Mapper functions
	const void NROM();				//Used for mapper 0x00
};

#endif
