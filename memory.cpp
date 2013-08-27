#include "memory.h"
#include "ppu.h"

memory::memory()
{
	for(int i = 0; i < 0x0800; i++)	//Clears ram
		RAM[i] = 0x00;
	RAM[0x0008] = 0xF7;
	RAM[0x0009] = 0xEF;
	RAM[0x000A] = 0xDF;
	RAM[0x000F] = 0xBF;
	
	for(int i = 0x2000; i < 0x3000; i++)	//Clears nametables
		VRAM[i] = 0xFF;
	DMAFlag = false;
	readBuffer = 0x00;
}

memory::~memory()
{

}

bool memory::loadMemory()
{
	using namespace std;
	bool retval;
	
	game.open("11-stack.nes", ios::in | ios::binary | ios::ate);
	if(game.is_open())
	{
		retval = true;
		game.seekg(0, ios::beg);
		game.read(header, 16);		//Reads the header bytes
		//The uppernibble of mapper number is in 7 and lower is in 6
		mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);

		
		switch(mapper)
		{
			case 0:	//NROM.  No mapper
				NROM();
				break;
		}
		
		if(header[6] & 1) horizontalMirror = true;		//Checks the first bit for the mirror mode
		else horizontalMirror = false;
	}
	else retval = false;
	
	return retval;
}

void memory::writeRAM(unsigned short &address, unsigned char &data, ppu* ppu)
{
	if(address <= 0x07FF)
	{
		//The data is mirrored at 0x0800 intervals 
		for(int i = address; i <= 0x1FFF; i += 0x0800)
		{
			RAM[i] = data;
		}
	}
	else if(address >= 0x2000 && address <= 0x3FFF)
	{
		if(address == 0x2002 && data == 0x80) 
		{
			RAM[0x2002] |= 0x80;				//Wants to set vblank only
			for(unsigned short i = 0x200A; i < 0x3FFF; i += 8)
				RAM[i] = RAM[0x2002];
		}
		else if(address == 0x2002 && data == 0x9F)
		{
			RAM[0x2002] &= 0x9F;				//Clears sprite 0 and overflow flags
			for(unsigned short i = 0x200A; i < 0x3FFF; i += 8)
				RAM[i] = RAM[0x2002];
		}
		else
		{
			//Mirrors every 8 bytes in both directions.
			for(unsigned short i = address; i < 0x3FFF; i += 8)
				RAM[i] = data;
			for(unsigned short i = address; i > 0x2000; i -= 8)
				RAM[i] = data;
		}
	}
	else if(address == 0x4014)
	{
		RAM[address] = data;
		DMA(data);
	}	
	else
	{
		//std::cout << "Somewhere else" << std::endl;
		RAM[address] = data;
	}

	//Checks for PPU register conditions
	switch(address)
	{
		case 0x2000:
			ppu->ppuTempAddress |= ((data & 0x03) << 10);	//Shifts the nametable select bits to bit 10 and 11 in the temp address
			break;
		case 0x2005:
			if(ppu->writeToggle)
			{
				ppu->ppuTempAddress &= 0x8C1F;				//Makes bits 5-9 and 12-14 zero
				ppu->ppuTempAddress |= (data & 0xF8) << 5;		//Shifts the data to fill bits 5-9
				ppu->ppuTempAddress |= (data & 0x07) << 12;		//Shifts the data to fill bits 12-14
				ppu->writeToggle = false;
				break;
			}
			else
			{
				ppu->ppuTempAddress &= ~0x001F;				//Makes the first 5 bits zero
				ppu->ppuTempAddress |= data & 0xF8;			//Gets the last 5 bits for the address
				ppu->fineXScroll = data & 0x07;				//Gets the first three bits
				ppu->writeToggle = true;
			}
			break;
		case 0x2006:
			if(ppu->writeToggle) 
			{
				ppu->ppuTempAddress &= 0xFF00;				//Clears the lower 8 bits
				ppu->ppuTempAddress |= data;				//Lower byte of address
				ppu->ppuAddress = ppu->ppuTempAddress;			//Set after temp address is filled
				ppu->writeToggle = false;
			}
			else 
			{
				ppu->ppuTempAddress &= 0x00FF;				//Clears upper 8 bits
				ppu->ppuTempAddress = (data & 0x3F) << 8;		//Upper piece of address
				ppu->writeToggle = true;
			}
			break;
		case 0x2007:
			writeVRAM(ppu->ppuAddress, data);
			if(RAM[0x2000] & 0x04) ppu->ppuAddress += 32;			//Checks increment bit
			else ppu->ppuAddress++;
			break;
	}
}

unsigned char memory::readRAM(unsigned short address, ppu* ppu)
{
	unsigned char retval;
	
	if(address == 0x2002)
	{
		retval = RAM[0x2002];
		//Have to reset things for the PPU
		RAM[0x2002] &= 0x7F;			//Turns off the 7th bit only
		for(unsigned short i = 0x200A; i < 0x3FFF; i += 8)
				RAM[i] = RAM[0x2002];
		ppu->writeToggle = false;
	}
	else if(address == 0x2007)
	{
		retval = readBuffer;
		readBuffer = VRAM[ppu->ppuAddress];
		if(RAM[0x2000] & 0x04) ppu->ppuAddress += 32;		//Checks increment bit
		else ppu->ppuAddress++;
	}	
	else	retval = RAM[address];

	return retval;
}

void memory::writeVRAM(unsigned short address, unsigned char &data)
{
	//Address above 0x3FFF wrap around between the 0x0000 and 0x3FFF range.
	address %= 0x4000;

	if(address >= 0x2000 && address <= 0x2FFF)
	{
		if(horizontalMirror) 
		{
			//$2000 = $2400 and $2800 = $2C00
			VRAM[address] = data;
			VRAM[address + 0x400] = data;
			VRAM[address + 0x1000] = data;
			if(address < 0x2B00) VRAM[address + 0x1400] = data;	//No mirror above 0x2EFF
		}
		else 
		{
			//$2000 = $2800 and $2400 = $2C00
			VRAM[address] = data;
			VRAM[address + 0x800] = data;
			VRAM[address + 0x1000] = data;				//Mirrored up one number
			if(address < 0x2300) VRAM[address + 0x1800] = data;	//0x2AFF = 0x2EFF.  No mirror above that
		}	
	}
	else
	{
		//Palletes are mirrored every 0x20 bytes
		VRAM[address] = data;
		VRAM[address + 0x20] = data;
	}
}

unsigned char memory::readVRAM(unsigned short &address)
{
	return VRAM[address];
}

void memory::clearDMA()
{
	DMAFlag = false;
}


//Private functions

const void memory::DMA(unsigned char &data)
{
	DMAFlag = true;
	unsigned char counter = 0;

	//The 256 bytes after the address defined by data * 0x100 is copied into the sprite ram
	for(int i = data * 0x100; i < (data * 0x100 + 0xFF); i++)
		OAM[counter++] = RAM[i];
}

void memory::dump()	//Dumps memory addresses $6000-$7FFF for CPU testing
{
	bool stringEnd = false, stringStart = false;
	unsigned char temp[0x3FF];
	int tempCounter = 0;
	char* text;


	for(int i = 0x6000; i < 0x8000; i++)
	{
		if(RAM[i] != 0)
		{
			temp[tempCounter++] = RAM[i];
			stringStart = true;
		}

		if(stringStart)
		{
			if(RAM[i] == 0)
			{
				temp[tempCounter++] = RAM[i];
				stringEnd = true;
			}
		}

		if(stringEnd)
		{
			std::stringstream s;
			s << temp;
			std::string text = s.str();
			std::cout << text << std::endl;
			stringEnd = stringStart = false;
			tempCounter = 0;
		}
	}
}


//Mapper functions

const void memory::NROM()
{
	int size = (0x4000 * header[4]) + (0x2000 * header[5]);		//Finds the total size of the game
	int j = 0x8000;							//Address of lower PRG bank
	memBlock = new char[size];
	game.seekg(16, std::ios::beg);
	game.read(memBlock, size);					//Reads in the data
	game.close();

	if(header[4] == 2)
	{
		for(int i = 0; i < (0x4000 * 2); i++)
			RAM[j++] = memBlock[i];
		j = 0x0000;						//Start of the pattern table
		for(int i = (0x4000 * 2); i < size; i++)
			VRAM[j++] = memBlock[i];
	}
	else
	{
		//This has to run twice since both banks need to be filled with the same PRG data
		//For 0x8000 bank
		for(int i = 0; i < 0x4000; i++)
			RAM[j++] = memBlock[i];
		//For 0xC000 bank
		for(int i = 0; i < 0x4000; i++)
			RAM[j++] = memBlock[i];
		
		j = 0x0000;						//Start of the pattern table
		
		for(int i = 0x4000; i < size; i++)
			VRAM[j++] = memBlock[i];
	}

	delete [] memBlock;
}

	

	
