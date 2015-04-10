#ifndef MEMORY_H
#define MEMORY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

class ppu;						//Foward declaration
class cpu;

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
	byte chrRom[0x2000];				//$0000-$1FFF in VRAM. Holds CHR-ROM
	byte nameTable1[0x400];				//$2000-$23FF in VRAM. Holds first nametable
	byte nameTable2[0x400];				//$2400-$27FF in VRAM. Holds second nametable
	byte pallete[0x20];				//$3F00-$3F1F in VRAM. Holds pallete for sprite and BG
	byte *VRAMPTR[0x4000];				//Pointers for VRAM
	char header[16];				//Holds the iNES header

	//Functions
	void writeRAM(word&, byte&); 			//Writes to RAM
	byte readRAM(word);				//Reads from RAM
	void writeVRAM(word, byte&);			//Writes to VRAM
	byte readVRAM(word&);				//Reads from VRAM
	void write_primary_OAM(byte, byte);
	byte read_primary_OAM(byte);
	void write_secondary_OAM(byte, byte);
	byte read_secondary_OAM(byte);
	bool setPointers(ppu*, cpu*);			

	bool loadMemory();				//Loads the game
	void dumpRAM();				//Dumps $6000 to $7FFF
	void dumpVRAM();				//Dumps all of VRAM
	void dumpOAM();				//Dumps primary OAM	
	void setMirror(bool);				//True = horizontal, false = vertical

	//Varaibles
	byte controller1;				//Shift register for controller1

private:
	//Variables
	byte mapper;					//Holds the mapper number
	char* memBlock;					//Holds the temporary cartridge data
	std::ifstream game;				//Loads the game
	std::ofstream debug;				//For debug output
	int gameSize;					//Size of the game
	bool horizontalMirror;				//0 = vertical mirroring, 1 = horizontal mirroring
	ppu* video;					//Pointer to PPU
	cpu* core;

	//Memory items
	byte primaryOAM[0x100];				//256 byte area for sprites
	byte secondaryOAM[0x20];			//32 byte area for 8 current sprites

	//Mapper functions
	const void NROM();				//Used for mapper 0x00
};

#endif
