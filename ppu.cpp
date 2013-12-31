#include "ppu.h"

ppu::ppu() : writeToggle(false), oddFrame(false), reg2000(0x00), reg2001(0x00), NMI(false), vblank(false), ntFetch(true)
		,ppuAddress(0), scanline(241), dotNumber(0), bufferVblank(false), horizontalDot(8), reloadDot(9), idleCounter(1)
{	
	ppuDebug.open("ppuDebug.txt");
}

ppu::~ppu()
{
	ppuDebug.close();
}

void ppu::emulateCycle()
{
	using namespace std;
	//Update the registers
	reg2000 = VRAM->RAM[0x2000];
	reg2001 = VRAM->RAM[0x2001];

	if(!vblank)					//If vblank is going on, none of this happens
	{
		//These are the only pixels that make it onto the screen
		//The top and bottom 8 scanlines are cut off
		if(scanline > 7 && scanline < 232)
			if(dotNumber < 256) renderPixel();
		
		if(reg2001 & 0x18)				//Checks if rendering is enabled
		{
			if(scanline < 240 || scanline == 261)
			{
				//Reload the registers
				if(dotNumber == reloadDot)
				{
					//Fills the upper 8 bits with next tiles
					highBGShift |= highBGFetch << 8; 
					lowBGShift |= lowBGFetch << 8; 
					fourToOneMux();							//Fills the attribute registers
					reloadDot += 8;							//Reload up to 257
				}
			}
			
			if(idleCounter != 0) idleCounter--;
			else
			{
				//For getting the background data.
				if(scanline < 240) 
				{
					visableBGFetch();						//For visable scanlines
				}
				else if(scanline == 240) 
				{
					idleCounter += 340;						//Idles for a scanline + 1 dot
				}
				else 
				{
					preBGFetch();							//For the prerender scanline
				}
			}

			if(scanline < 240 || scanline == 261)
			{
				//The registers only shift between dots 2-257 and 322-337 (inclusive)
				if((dotNumber > 1 && dotNumber < 258) || (dotNumber > 321 && dotNumber < 338)) shiftRegisters();
			}

			checkDotNumber();						//Does stuff depending on the dot number
		}
	}

	dotNumber++;
	
	if(dotNumber == 341) 
	{
		scanline++;		//262 scanlines in a frame
		dotNumber = 0;
		if(scanline == 262)
		{
			scanline = 0;

			if(oddFrame) oddFrame = false;				//Is already an oddframe, so starts at 0,0
			else
			{
				oddFrame = true;
				dotNumber = 1;				//Dot 0 is skipped on odd frame
			}
		}
	}

	checkVblank();					//Checks if vblank is happening
}

//Sets the VRAM pointer
bool ppu::setPointer(memory* memory)
{
	bool retval = true;

	VRAM = memory;

	if(VRAM == NULL) retval = false;

	return retval;
}


//----------------------------------------------------------------------------------------------------------------------------------------
//Private functions
const void ppu::checkDotNumber()
{
	if(dotNumber == 256)
	{
		if((ppuAddress & 0x7000) != 0x7000)	ppuAddress += 0x1000;	//If fine Y < 7 increment it
		else
		{
			ppuAddress &= ~0x7000;		//Fine Y = 0
			coarseY = (ppuAddress & 0x03E0) >> 5;	//Let y = coarse Y
			if(coarseY == 29)
			{
				coarseY = 0;			//Coarse Y = 0
				ppuAddress = (ppuAddress & ~0x03E0) | (coarseY << 5);		//Put coarse Y back into address
				ppuAddress ^= 0x8000;		//Switch vertical nametable
			}
			else if(coarseY == 31) 
			{
				coarseY = 0;			//Coarse Y = 0, nametable not switched
				ppuAddress = (ppuAddress & ~0x03E0) | (coarseY << 5);		//Put coarse Y back into address
			}
			else
			{
				coarseY++;							//Increment coarse Y
				ppuAddress = (ppuAddress & ~0x03E0) | (coarseY << 5);		//Put coarse Y back into address
			}
		}
	}


	if(dotNumber == 257)
	{
		ppuAddress &= 0x7DF0;						//Clears the bits for horizontal position
		ppuAddress |= ppuTempAddress & ~0x7DF0;				//Keeps the bits that were cleared above
		horizontalDot = 328;						//Next time this is needed
		reloadDot = 329;						//Next time the registers are reloaded
	}
	else if(dotNumber > 279 && dotNumber < 305)
	{
		ppuAddress &= ~0xFBE0;				//Clears the vertical bits
		ppuAddress |= ppuTempAddress & 0xFBE0;		//Puts the vertical bits in
	}
	else if(dotNumber < 257 || dotNumber > 327)
	{
		if(dotNumber == horizontalDot)
		{
			if((ppuAddress & 0x001F) == 31) 	// if coarse X == 31
			{
				ppuAddress &= ~0x001F;         	// coarse X = 0
				ppuAddress ^= 0x0400;           // switch horizontal nametable
			}
			else ppuAddress++;                	// increment coarse X

			horizontalDot += 8;
		}
	
		if(dotNumber == 336) horizontalDot = 8;				//Next dot to increment horizontal position
		else if(dotNumber == 337) reloadDot = 9;			//Next dot to reload registers
	}
}

const void ppu::shiftRegisters()
{
	//The registers only shift between dots 2-257 and 322-337 (inclusive)
	lowBGShift >>= 1;
	highBGShift >>= 1;
	lowAttShift >>= 1;
	highAttShift >>= 1;

}

const void ppu::renderPixel()
{
	if(reg2001 & 0x18)
        {
                if(reg2001 & 0x08)                //Checks if background rendering is enabled
                {
                        palleteAddress = 0x3F00 | eightToOneMux(lowBGShift) | (eightToOneMux(highBGShift) << 1)
                                                        | (eightToOneMux(lowAttShift) << 2) | (eightToOneMux(highAttShift) << 3);
                        palleteData = VRAM->readVRAM(palleteAddress);

                	screenData[dotNumber][scanline - 8] = RGB[palleteData];	//RGB data
                }
                else
                {
                        if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;        //If ppuAddress in pallete address range, use that address.
                        else palleteAddress = 0x3F00;
                        palleteData = VRAM->readVRAM(palleteAddress);

			screenData[dotNumber][scanline - 8] = RGB[palleteData];
                }
        }
        else
        {
                if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;
                else palleteAddress = 0x3F00;
                palleteData = VRAM->readVRAM(palleteAddress);

		screenData[dotNumber][scanline - 8] = RGB[palleteData];
        }
}

const void ppu::checkVblank()
{
	if(scanline == 241 && dotNumber == 1) 
	{
		VRAM->writeRAM(reg2002, vblankValue);
		vblank = true;
		bufferVblank = true;
		if(reg2000 & 0x80) NMI = true;				//NMI bit set
	}
	else if(scanline == 261 && dotNumber == 1)
	{
		unsigned char clearData = 0x9F;				//Value that clears the flags
		zeroFlag = false;
		spriteOverflow = false;
		vblank = false;
		VRAM->writeRAM(reg2002, clearData);		//Clears sprite 0 hit and overflow bit
	}
}



//----------------------------------------------------------------------------------------------------------------------------------------
//Scanline functions	

const void ppu::visableBGFetch()
{	
	if(dotNumber != 257)
	{
		if((dotNumber > 0 && dotNumber < 257) || (dotNumber > 320 && dotNumber < 337))
		{
			if(ntFetch)	//First in the order
			{
				nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
				nameFetch = VRAM->readVRAM(nameAddress);
				ppuDebug << std::hex << std::setfill('0') << "Name: 0x" << std::setw(4) << nameAddress;
				ntFetch = false;
				atFetch = true;
				idleCounter++;
			}
			else if(atFetch)
			{
				attAddress = 0x23C0 | (ppuAddress & 0x0C00) | ((ppuAddress >> 4) & 0x38) | ((ppuAddress >> 2) & 0x07);
				attFetch = VRAM->readVRAM(attAddress);
				ppuDebug << std::hex << std::setfill('0') << " Att: 0x" << std::setw(4) << attAddress;
				atFetch = false;
				bgLowFetch = true;
				idleCounter++;
			}
			else if(bgLowFetch)
			{
				//Finds the bg tile address
				if(reg2000 & 0x10) tileAddress = 0x1000 | (nameFetch << 4) | ((ppuAddress & 0x7000) >> 12);
				else tileAddress = 0x0000 | (nameFetch << 4) | ((ppuAddress & 0x7000) >> 12);
				ppuDebug << std::hex << std::setfill('0') << " Low tile: 0x" << std::setw(4) << tileAddress;
				lowBGFetch = VRAM->readVRAM(tileAddress);
				bgLowFetch = false;
				idleCounter++;
			}
			else		//End of cycle.  Do nametable fetch next.
			{
				tileAddress += 8;				//8 bytes ahead
				ppuDebug << std::hex << std::setfill('0') << " High tile: 0x" << std::setw(4) << tileAddress;
				ppuDebug << std::hex << std::setfill('0') << " PPU: 0x" << std::setw(4) << ppuAddress << std::endl;
				highBGFetch = VRAM->readVRAM(tileAddress);
				ntFetch = true;
				idleCounter++;
			}	
		}
		else
		{
			nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
			nameFetch = VRAM->readVRAM(nameAddress);
			idleCounter++;
		}
	}
	else
		idleCounter += 63;		//Idle between cycles 258-320
}


const void ppu::preBGFetch()
{
	if((dotNumber > 0 && dotNumber < 257) || (dotNumber > 320 && dotNumber < 337))
	{
		if(ntFetch)
		{
			nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
			nameFetch = VRAM->readVRAM(nameAddress);
			ntFetch = false;
			atFetch = true;
			idleCounter++;
		}
		else if(atFetch)
		{
			attAddress = 0x23C0 | (ppuAddress & 0x0C00) | ((ppuAddress >> 4) & 0x38) | ((ppuAddress >> 2) & 0x07);
			attFetch = VRAM->readVRAM(attAddress);
			atFetch = false;
			bgLowFetch = true;
			idleCounter++;
		}
		else if(bgLowFetch)
		{
			//Finds the bg tile address
			if(reg2000 & 0x10) tileAddress = 0x1000 | (nameFetch << 4) | ((ppuAddress & 0x7000) >> 12);
			else tileAddress = 0x0000 | (nameFetch << 4) | ((ppuAddress & 0x7000) >> 12);
			lowBGFetch = VRAM->readVRAM(tileAddress);
			bgLowFetch = false;
			idleCounter++;
		}
		else
		{
			tileAddress += 8;
			highBGFetch = VRAM->readVRAM(tileAddress);	//8 bytes ahead of lower address
			ntFetch = true;
			idleCounter++;
		}	
	}
	else
	{
		nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
		nameFetch = VRAM->readVRAM(nameAddress);
		idleCounter++;
	}
}



//------------------------------------------------------------------------------------------------------------------------------------------
//Muxes
const void ppu::fourToOneMux()				//Used to refill attribute shift registers
{
	bool xBit, yBit;				//Holds the coarse X and Y bits
	bool attBit1, attBit2;				//Holds the two attrbute bits. 1 = low bit, 2 = high bit
	xBit = ppuAddress & 0x1;
	yBit = ppuAddress & 0x20;
	
	
	//Bit 0 = xBit, Bit 1 = yBit
	if(xBit == true && yBit == true)		//Choose bits 6 and 7
	{
		attBit1 = attFetch & 0x40;
		attBit2 = attFetch & 0x80;
	}
	else if(xBit)					//Choose bits 4 and 5
	{
		attBit1 = attFetch & 0x10;
		attBit2 = attFetch & 0x20;
	}
	else if(yBit)					//Choose bits 2 and 3
	{
		attBit1 = attFetch & 0x04;
		attBit2 = attFetch & 0x08;
	}
	else						//Choose bits 0 and 1
	{
		attBit1 = attFetch & 0x01;
		attBit2 = attFetch & 0x02;
	}
	
	//Sets all the bits the same
	if(attBit2) highAttShift = 0xFFFF;		//All 1s
	else highAttShift = 0;				//All 0s
	
	if(attBit1) lowAttShift = 0xFFFF;		//All 1s
	else lowAttShift = 0;				//All 0s
}


const bool ppu::eightToOneMux(unsigned short &data)		//16 bit numbers
{
	bool retval;
		
	if(fineXScroll == 0) retval = data & 0x01;
	else if(fineXScroll == 1) retval = data & 0x02;
	else if(fineXScroll == 2) retval = data & 0x04;
	else if(fineXScroll == 3) retval = data & 0x08;
	else if(fineXScroll == 4) retval = data & 0x10;
	else if(fineXScroll == 5) retval = data & 0x20;
	else if(fineXScroll == 6) retval = data & 0x40;
	else retval = data & 0x80;

	return retval;
}

const bool ppu::eightToOneMux(unsigned char &data)		//8 bit numbers
{
	bool retval;
		
	if(fineXScroll == 0) retval = data & 0x01;
	else if(fineXScroll == 1) retval = data & 0x02;
	else if(fineXScroll == 2) retval = data & 0x04;
	else if(fineXScroll == 3) retval = data & 0x08;
	else if(fineXScroll == 4) retval = data & 0x10;
	else if(fineXScroll == 5) retval = data & 0x20;
	else if(fineXScroll == 6) retval = data & 0x40;
	else retval = data & 0x80;

	return retval;
}


//Pallete colors
int const ppu::RGB[] =
{
   0x00808080, 0x00003DA6, 0x000012B0, 0x00440096, 0x00A1005E, 0x00C70028, 0x00BA0600, 
   0x008C1700, 0x005C2F00, 0x00104500, 0x00054A00, 0x0000472E, 0x00004166, 0x00000000, 
   0x00050505, 0x00050505, 0x00C7C7C7, 0x000077FF, 0x002155FF, 0x008237FA, 0x00EB2FB5, 
   0x00FF2950, 0x00FF2200, 0x00D63200, 0x00C46200, 0x00358000, 0x00058F00, 0x00008A55, 
   0x000099CC, 0x00212121, 0x00090909, 0x00090909, 0x00FFFFFF, 0x000FD7FF, 0x0069A2FF, 
   0x00D480FF, 0x00FF45F3, 0x00FF618B, 0x00FF8833, 0x00FF9C12, 0x00FABC20, 0x009FE30E, 
   0x002BF035, 0x000CF0A4, 0x0005FBFF, 0x005E5E5E, 0x000D0D0D, 0x000D0D0D, 0x00FFFFFF, 
   0x00A6FCFF, 0x00B3ECFF, 0x00DAABEB, 0x00FFA8F9, 0x00FFABB3, 0x00FFD2B0, 0x00FFEFA6, 
   0x00FFF79C, 0x00D7E895, 0x00A6EDAF, 0x00A2F2DA, 0x0099FFFC, 0x00DDDDDD, 0x00111111, 
   0x00111111
};


