#include "memory.h"
#include "ppu.h"

memory::memory(): DMAFlag(false), readBuffer(0x00)
{
	for(int i = 0; i < 0x4000; i++)	//Clears ram
		RAM[i] = 0x00;
	for(int i = 0x4000; i < 0x8000; i++)
		RAM[i] = 0xFF;
	
	for(int i = 0; i < 0x400; i++)	//Clears nametables
		nameTable1[i] = nameTable2[i] = 0xFF;

	//Setting up RAM pointers
	for(int i = 0; i < 0x0800; i++)
		RAMPTR[i] = RAMPTR[i + 0x800] = RAMPTR[i + 0x1000] = RAMPTR[i + 0x1800] = &RAM[i];

	for(int i = 0x2000; i < 0x4000; i += 8)
	{
		RAMPTR[i] = &RAM[0x2000];
		RAMPTR[i + 1] = &RAM[0x2001];
		RAMPTR[i + 2] = &RAM[0x2002];
		RAMPTR[i + 3] = &RAM[0x2003];
		RAMPTR[i + 4] = &RAM[0x2004];
		RAMPTR[i + 5] = &RAM[0x2005];
		RAMPTR[i + 6] = &RAM[0x2006];
		RAMPTR[i + 7] = &RAM[0x2007];
	}

	for(int i = 0x4000; i < 0x8000; i++)
		RAMPTR[i] = &RAM[i];

	//Setup VRAM pallete pointers
	for(int i = 0; i < 0x20; i++)
	{
		VRAMPTR[i + 0x3F00] = VRAMPTR[i + 0x3F20] = VRAMPTR[i + 0x3F40] = VRAMPTR[i + 0x3F60] 
		= VRAMPTR[i + 0x3F80] = VRAMPTR[i + 0x3FA0] = VRAMPTR[i + 0x3FC0] = &pallete[i];
		pallete[i] = 0x0D;	//Sets the background to black
	}

	debug.open("VRAM_DUMP.bin");
}

memory::~memory()
{
	debug.close();
}

bool memory::loadMemory()
{
	using namespace std;
	bool retval;
	
	game.open("16-special.nes", ios::binary);
	if(game.is_open())
	{
		retval = true;
		game.seekg(0, game.end);
		gameSize = game.tellg();
		game.seekg(0, game.beg);
		memBlock = new char[gameSize];
		game.read(memBlock, gameSize);		//Reads the header bytes
		game.close();

		for(int i = 0; i < 16; i++)
			header[i] = memBlock[i];
		//The uppernibble of mapper number is in 7 and lower is in 6
		mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);

		if(header[6] & 1) horizontalMirror = true;		//Checks the first bit for the mirror mode
		else horizontalMirror = false;
		
		switch(mapper)
		{
			case 0:	//NROM.  No mapper
				NROM();
				break;
		}
	}
	else retval = false;
	
	return retval;
}

void memory::writeRAM(word &address, byte &data, ppu* ppu)
{
	if(address >= 0x2000 && address <= 0x3FFF)
	{
		word tempAddress = address & 0x7;			//Only gets the first three bits

		if(tempAddress == 0)
		{
			RAM[0x2000] = data;
			ppu->ppuTempAddress &= ~0x0C00;			//Clears bits 10 and 11
			ppu->ppuTempAddress |= ((data & 0x03) << 10);	//Shifts the nametable select bits to bit 10 and 11 in the temp address
		}
		else if(tempAddress == 2 && data == 0x80) 
		{
			RAM[0x2002] |= 0x80;				//Wants to set vblank only
		}
		else if(tempAddress == 2 && data == 0x9F)
		{
			RAM[0x2002] &= 0x9F;				//Clears sprite 0 and overflow flags
		}
		else if(tempAddress == 5)
		{
			if(ppu->writeToggle)
			{
				RAM[0x2005] = data;
				ppu->ppuTempAddress &= 0x8C1F;				//Makes bits 5-9 and 12-14 zero
				ppu->ppuTempAddress |= (data & 0xF8) << 2;		//Shifts the data to fill bits 5-9
				ppu->ppuTempAddress |= (data & 0x07) << 12;		//Shifts the data to fill bits 12-14
				ppu->writeToggle = false;
			}
			else
			{
				RAM[0x2005] = data;
				ppu->ppuTempAddress &= ~0x001F;				//Makes the first 5 bits zero
				ppu->ppuTempAddress |= (data & 0xF8) >> 3;		//Gets the last 5 bits for the address
				ppu->fineXScroll = data & 0x07;				//Gets the first three bits
				ppu->writeToggle = true;
			}
		}
		else if(tempAddress == 6)
		{
			if(ppu->writeToggle) 
			{
				RAM[0x2006] = data;
				ppu->ppuTempAddress &= 0xFF00;				//Clears the lower 8 bits
				ppu->ppuTempAddress |= data;				//Lower byte of address
				ppu->ppuAddress = ppu->ppuTempAddress;			//Set after temp address is filled
				ppu->writeToggle = false;
			}
			else 
			{
				RAM[0x2006] = data;
				ppu->ppuTempAddress &= 0x00FF;				//Clears upper 8 bits
				ppu->ppuTempAddress = (data & 0x3F) << 8;		//Upper piece of address
				ppu->writeToggle = true;
			}
		}
		else if(tempAddress == 7)
		{
			RAM[0x2007] = data;
			writeVRAM(ppu->ppuAddress, data);
			if(RAM[0x2000] & 0x04) ppu->ppuAddress += 32;			//Checks increment bit
			else ppu->ppuAddress++;
		}
		else	RAM[tempAddress + 0x2000] = data;
	}
	/*else if(address == 0x4014)
	{
		RAM[address] = data;
		DMA(data);
	}	*/
	else	*RAMPTR[address] = data;
}

memory::byte memory::readRAM(word address, ppu* ppu)
{
	byte retval;
	
	if(address >= 0x2000 && address <= 0x3FFF)
	{
		address &= 0x7;			//Only the lower 3 bits
		
		if(address == 2)
		{
			retval = RAM[0x2002];
			//Have to reset things for the PPU
			RAM[0x2002] &= 0x7F;			//Turns off the 7th bit only
			ppu->writeToggle = false;
		}
		else if(address == 7)
		{
			retval = readBuffer;
			readBuffer = *VRAMPTR[ppu->ppuAddress];
			if(RAM[0x2000] & 0x04) ppu->ppuAddress += 32;		//Checks increment bit
			else ppu->ppuAddress++;
		}
		else retval = RAM[0x2000 + address];
	}
	else	retval = *RAMPTR[address];

	return retval;
}

void memory::writeVRAM(word address, byte &data)
{
	//Address above 0x3FFF wrap around between the 0x0000 and 0x3FFF range.
	address &= 0x3FFF;

	*VRAMPTR[address] = data;
}

memory::byte memory::readVRAM(word &address)
{
	return *VRAMPTR[address];
}

void memory::clearDMA()
{
	DMAFlag = false;
}


//Private functions

const void memory::DMA(byte &data)
{
	DMAFlag = true;
	byte counter = 0;

	//The 256 bytes after the address defined by data * 0x100 is copied into the sprite ram
	for(int i = data * 0x100; i < (data * 0x100 + 0xFF); i++)
		OAM[counter++] = RAM[i];
}

void memory::dumpRAM()	//Dumps memory addresses $6000-$7FFF for CPU testing
{
	bool stringEnd = false, stringStart = false;
	byte temp[0x2000];
	int tempCounter = 0;

	for(int i = 0x6000; i < 0x8000; i++)
	{
		debug << std::hex << (int)RAM[i] << std::endl;
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

void memory::dumpVRAM()
{
	using namespace std;

	for(int i = 0; i < 0x4000; i++)
	{
		debug << hex << (int)*VRAMPTR[i];
		if( ((i & 0xFF) == 0x00) || ((i & 0xFF) == 0x30) || ((i & 0xFF) == 0x60) || ((i & 0xFF) == 0x90) || ((i & 0xFF) == 0xC0) )
			debug << endl;
	}
}

void memory::setMirror(bool horizontal)
{
	int j;
	
	//$2000-$27FF contains the data in VRAM (nameTable1 and nameTable2
	if(horizontal) //$2400 = $2000 and $2C00 = $2800
	{
		j = 0x2000;
		
		for(int i = 0; i < 0x400; i++)
		{
			VRAMPTR[j] = VRAMPTR[j + 0x400] = VRAMPTR[j + 0x1000] = VRAMPTR[j + 0x1400] = &nameTable1[i];
			j++;
		}
		
		j = 0x2800;

		for(int i = 0; i < 0x400; i++)
		{
			VRAMPTR[j] = VRAMPTR[j + 0x400] = VRAMPTR[j + 0x1000] = &nameTable2[i];
			if( (j + 0x1400) < 0x3F00) VRAMPTR[j + 0x1400] = &nameTable2[i];
			j++;
		}
	}
	else	//Vertical.  $2800 = $2000 and $2C00 = $2400
	{
		j = 0x2000;
		
		for(int i = 0; i < 0x400; i++) {
			VRAMPTR[j] = VRAMPTR[j + 0x800] = VRAMPTR[j + 0x1000] = VRAMPTR[j + 0x1800] = &nameTable1[i];
			j++;
		}

		j = 0x2400;
		
		for(int i = 0; i < 0x400; i++)
		{
			VRAMPTR[j] = VRAMPTR[j + 0x800] = VRAMPTR[j + 0x1000] = &nameTable2[i];
			if( (j + 0x1800) < 0x3F00) VRAMPTR[i + 0x1800] = &nameTable2[i];
			j++;
		}
	}
}

//Mapper functions

const void memory::NROM()
{
	int j = 0x8000;							//Address of lower PRG bank

	if(header[4] > 1)
	{
		for(int i = 0x10; i < 0x8010; i++)
		{
			RAM[j] = memBlock[i];
			RAMPTR[j] = &RAM[j];
			j++;
		}	
			
		j = 0x0000;						//Start of the pattern table

		for(int i = 0x8010; i < gameSize; i++)
		{
			chrRom[j] = memBlock[i];
			VRAMPTR[j] = &chrRom[j];
			j++;
		}
		
		setMirror(horizontalMirror);
	}
	else
	{
		//For 0x8000 bank
		for(int i = 0x10; i < 0x4010; i++)
			RAM[j++] = memBlock[i];
		
		//Setting the pointers up
		for(int i = 0x8000; i < 0xC000; i++)
			RAMPTR[i] = RAMPTR[i + 0x4000] = &RAM[i];
		
		j = 0x0000;						//Start of the pattern table
		
		for(int i = 0x4010; i < gameSize; i++)
		{
			chrRom[j] = memBlock[i];
			VRAMPTR[j] = &chrRom[j];
			j++;
		}

		setMirror(horizontalMirror);
	}

	delete [] memBlock;
}

	

	
