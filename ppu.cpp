#include "ppu.h"

ppu::ppu() : writeToggle(false), oddFrame(false), reg2000(0x00), reg2001(0x00), NMI(false), vblank(false), ntFetch(true)
		,ppuAddress(0), scanline(241), dotNumber(0), bufferVblank(false), horizontalDot(8), reloadDot(9), idleCounter(0)
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
		if(scanline < 240)
			if(dotNumber < 256) renderPixel();
		
		if(reg2001 & 0x18)				//Checks if rendering is enabled
		{	
			if(idleCounter > 0) idleCounter--;
			else
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

				//For getting the background data.
				if(scanline == 240) 
					idleCounter += 340;						//Idles for a scanline + 1 dot
				else
					backgroundFetch();						//Fetch name, att, tiles
			}

			
			//The registers only shift between dots 2-257 and 322-337 (inclusive)
			if((dotNumber > 1 && dotNumber < 258) || (dotNumber > 321 && dotNumber < 338)) shiftRegisters();

			checkDotNumber();						//Does stuff depending on the dot number
		}
	}

	dotNumber++;
	
	if(dotNumber == 341) 
	{
		scanline++;		//262 scanlines in a frame
		dotNumber = 0;

		/*This may need to be changed.  On startup, the first run of scanline 261
		**starts at dot 2.*/
		if(scanline == 261) idleCounter = 0;	//No idle on 261 because it turns back on on the second dot
		else idleCounter = 1;			//Make sure idle counter is always set 1 to idle first dot
		
		if(scanline == 262)
		{
			scanline = 0;
			if(reg2001 & 0x18)			//Checks if rendering is enabled
			{
				if(oddFrame == false)
				{	
					oddFrame = true;
					dotNumber = 1;		//Dot 0 is skipped on odd frame
					idleCounter = 0;	//Odd frame, so no idle on first dot
				}
				else oddFrame = false;
			}
		}
	}

	checkVblank();					//Checks if vblank is happening
}


//Renders a nametable all at once when vblank is reached
void ppu::simpleRender()
{	
	word highTileAddress;
	int x = 0, y = 0, temp;

	if(reg2001 & 0x18)
	{ 
		for(nameAddress = 0x2000; nameAddress < 0x23C0; nameAddress++)
		{
			//Fetching data and creating address
			nameFetch = VRAM->readVRAM(nameAddress);
			tileAddress = 0x0000 | (nameFetch << 4);
			word highTileAddress = tileAddress + 8;

			//Get attribute data
			simRenAtt();

			for(int i = 0; i < 8; i++)
			{
				tileAddress += i;
				highTileAddress += i;
				highBGFetch = VRAM->readVRAM(highTileAddress);
				lowBGFetch = VRAM->readVRAM(tileAddress);
			
				for(int j = 0; j < 8; j++)
				{
					temp = (lowBGFetch & 1) | ((highBGFetch & 1) << 1) | ((attFetch & 3) << 2);
					palleteAddress = 0x3F00 | temp;
					palleteData = VRAM->readVRAM(palleteAddress);
					screenData[y + i][x + j] = RGB[palleteData];	
					lowBGFetch >>= 1;
					highBGFetch >>= 1;
				}
			}

			if(x == 248) {x = 0; y += 8;}		
			else x += 8;
		}
	}
	else
        {
                if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;
                else palleteAddress = 0x3F00;
                palleteData = VRAM->readVRAM(palleteAddress);

		for(int y = 0; y < 240; y++)
			for(int x = 0; x < 256; x++)
				screenData[y][x] = RGB[palleteData];		//RGB data
        }
}


//Prints out debug info.
void ppu::printDebug()
{
	using namespace std;

	/*ppuDebug << hex << setfill('0') << "Name: 0x" << setw(4) << nameAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)nameFetch;
	ppuDebug << hex << setfill('0') << " Att: 0x" << setw(4) << attAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)attFetch;
	ppuDebug << hex << setfill('0') << " Low tile: 0x" << setw(4) << tileAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)lowBGFetch;
	ppuDebug << hex << setfill('0') << " High tile: 0x" << setw(4) << (int)tileAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)highBGFetch;
	ppuDebug << hex << setfill('0') << "PPU: 0x" << setw(4) << ppuAddress;
	ppuDebug << hex << setfill('0') << " Palletteaddr: 0x" << setw(4) << palleteAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)palleteData << endl;
	ppuDebug << dec << "Scanline: " << setw(3) << scanline;
	ppuDebug << dec << " Dot: " << setw(3) << dotNumber;
	ppuDebug << dec << " Scanline: " << setw(3) << scanline << endl << endl;*/

	ppuDebug << hex << setfill('0') << "Name: 0x" << setw(4) << nameAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)nameFetch;
	ppuDebug << hex << setfill('0') << " Att: 0x" << setw(4) << attAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)attFetch;
	ppuDebug << hex << setfill('0') << " Low tile: 0x" << setw(4) << tileAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)lowBGFetch;
	ppuDebug << hex << setfill('0') << " High tile: 0x" << setw(4) << (int)tileAddress << endl;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)highBGFetch;
	ppuDebug << hex << setfill('0') << " PPU: 0x" << setw(4) << ppuAddress;
	ppuDebug << hex << setfill('0') << " Palletteaddr: 0x" << setw(4) << palleteAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)palleteData;
	ppuDebug << dec << " Dot: " << setw(3) << dotNumber;
	ppuDebug << dec << " Scanline: " << setw(3) << scanline << endl << endl;

	/*ppuDebug << hex << setfill('0') << "Low att: 0x" << setw(4) << lowAttShift;
	ppuDebug << hex << setfill('0') << " High Att: 0x" << setw(4) << highAttShift;
	ppuDebug << hex << setfill('0') << " Low tile: 0x" << setw(4) << lowBGShift;
	ppuDebug << hex << setfill('0') << " High tile: 0x" << setw(4) << highBGShift << endl;
	ppuDebug << hex << setfill('0') << "PPU: 0x" << setw(4) << ppuAddress;
	ppuDebug << hex << setfill('0') << " Palletteaddr: 0x" << setw(4) << palleteAddress;
	ppuDebug << hex << setfill('0') << " Value: 0x" << setw(2) << (int)palleteData;
	ppuDebug << dec << "Scanline: " << setw(3) << scanline;
	ppuDebug << dec << " Dot: " << setw(3) << dotNumber << endl << endl;*/

	/*for(int y = 0; y < 240; y++)
		for(int x = 0; x < 256; x++)
		{
			ppuDebug << setfill('0') << hex << uppercase << setw(8) << screenData[y][x] << " ";

			if( (x & 0xF) == 0xF ) ppuDebug << endl;
		}	*/
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

//Rendering has to be enabled for this function to be called
const void ppu::checkDotNumber()
{
	//Coarse Y increment
	if(dotNumber == 256)
	{
		if((ppuAddress & 0x7000) != 0x7000)	ppuAddress += 0x1000;	//If fine Y < 7 increment it
		else
		{
			ppuAddress &= ~0x7000;		//Fine Y = 0
			int coarseY = (ppuAddress & 0x03E0) >> 5;	//Let y = coarse Y

			if(coarseY == 29)
			{
				coarseY = 0;			//Coarse Y = 0
				ppuAddress ^= 0x0800;		//Switch vertical nametable
			}
			else if(coarseY == 31) 
				coarseY = 0;			//Coarse Y = 0, nametable not switched
			else
				coarseY++;							//Increment coarse 
			
			ppuAddress = (ppuAddress & ~0x03E0) | (coarseY << 5);		//Put coarse Y back into address		
		}
	}


	if(dotNumber == 257)
	{	ppuDebug << "Temp: " << hex << uppercase << ppuTempAddress;
		ppuDebug << " PPU: " << hex << uppercase << ppuAddress;
		ppuAddress &= ~0x41F;						//Clears the bits for horizontal position
		ppuAddress |= ppuTempAddress & 0x41F;				//Keeps the bits that were cleared above
		horizontalDot = 328;						//Next time this is needed
		reloadDot = 329;						//Next time the registers are reloaded
		ppuDebug << " New PPU: " << hex << uppercase << ppuAddress << endl;
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
	else if(scanline == 261 && (dotNumber > 279 && dotNumber < 305))
	{
		ppuDebug << "Temp: " << hex << uppercase << ppuTempAddress;
		ppuDebug << " PPU: " << hex << uppercase << ppuAddress << endl;
		ppuAddress &= ~0x7BE0;				//Clears the vertical bits
		ppuAddress |= ppuTempAddress & 0x7BE0;		//Puts the vertical bits in
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
	using namespace std;

	if(reg2001 & 0x18)
        {	
                if(reg2001 & 0x08)                //Checks if background rendering is enabled
                {	
                        palleteAddress = 0x3F00 | eightToOneMux(lowBGShift) | (eightToOneMux(highBGShift) << 1)
                                                        | (eightToOneMux(lowAttShift) << 2) | (eightToOneMux(highAttShift) << 3);
                        palleteData = VRAM->readVRAM(palleteAddress);
			
                	screenData[scanline][dotNumber] = RGB[palleteData];	//RGB data
                }
                else
                {
                        if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;        //If ppuAddress in pallete address range, use that address.
                        else palleteAddress = 0x3F00;
                        palleteData = VRAM->readVRAM(palleteAddress);

			screenData[scanline][dotNumber] = RGB[palleteData];	//RGB data
                }
        }
        else
        {	
                if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;
                else palleteAddress = 0x3F00;
                palleteData = VRAM->readVRAM(palleteAddress);

		screenData[scanline][dotNumber] = RGB[palleteData];	//RGB data
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


const void ppu::simRenAtt()
{
	if( (nameAddress & 0xF00) == 0 ) attAddress = 0x23C0;		//Base att = 0x23C0
	else if( (nameAddress & 0xF00) == 0x100 ) attAddress = 0x23D0;	//Base att = 0x23D0
	else if( (nameAddress & 0xF00) == 0x200 ) attAddress = 0x23E0;	//Base att = 0x23E0
	else attAddress = 0x23F0;					//Base att = 0x23F0

	//Create value to compare
	word temp = (nameAddress & 0xF0) >> 4;
			
	//Check to see if this nametable address is in the top or bottom 32x16 attribute sq
	if(temp < 4 || (temp > 7 && temp << 0xC)) top = true;
	else top = false;

	//If these values, att byte is in second set of att squares
	if(temp == 1 || temp == 3 || temp == 5 || temp == 7)
		attAddress += 4;
	else if(temp == 8 || temp == 0xA || temp == 0xC || temp == 0xE)
		attAddress += 8;
	else if(temp == 9 || temp == 0xB || temp == 0xD || temp == 0xF)
		attAddress += 0xC;

	//Add final offset to get to correct attribute byte
	temp = nameAddress & 0xF;
	if(temp < 0x4) ;//Do nothing
	else if(temp < 0x8) attAddress += 1;
	else if(temp < 0xC) attAddress += 2;
	else attAddress += 3;

	//Figure out if nametable in left or right 16x32 att sq
	if(temp == 0 || temp == 1 || temp == 4 || temp == 5 || temp == 8 || temp == 9 
		|| temp == 0xC || temp == 0xD) left = true;
	else left = false;

	attFetch = VRAM->readVRAM(attAddress);

	if(top == true && left == false) attFetch >>= 2;
	else if(top == false && left == true) attFetch >>= 4;
	else if(top == false && left == false) attFetch >>= 6;
}



//----------------------------------------------------------------------------------------------------------------------------------------
//Scanline functions	

const void ppu::backgroundFetch()
{	
	if(dotNumber != 257)
	{
		if((dotNumber > 0 && dotNumber < 257) || (dotNumber > 320 && dotNumber < 337))
		{
			if(ntFetch)	//First in the order
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
			else		//End of cycle.  Do nametable fetch next.
			{	
				tileAddress += 8;				//8 bytes ahead
				highBGFetch = VRAM->readVRAM(tileAddress);
				ntFetch = true;
				idleCounter++;
			}	
		}
		else
		{	
			ntFetch = true;			
			nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
			nameFetch = VRAM->readVRAM(nameAddress);
			idleCounter++;
			
		}
	}
	else
		idleCounter += 63;		//Idle between cycles 258-320
}


/*const void ppu::preBGFetch()
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
}*/



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
	//0/8	1/9		2/A	3/B		4/C	5/D		6/E	7/F
   0x00808080, 0x00003DA6, 0x000012B0, 0x00440096, 0x00A1005E, 0x00C70028, 0x00BA0600, 0x008C1700, //0
   0x005C2F00, 0x00104500, 0x00054A00, 0x0000472E, 0x00004166, 0x00000000, 0x00050505, 0x00050505,
   0x00C7C7C7, 0x000077FF, 0x002155FF, 0x008237FA, 0x00EB2FB5, 0x00FF2950, 0x00FF2200, 0x00D63200, //1
   0x00C46200, 0x00358000, 0x00058F00, 0x00008A55, 0x000099CC, 0x00212121, 0x00090909, 0x00090909,
   0x00FFFFFF, 0x000FD7FF, 0x0069A2FF, 0x00D480FF, 0x00FF45F3, 0x00FF618B, 0x00FF8833, 0x00FF9C12, //2
   0x00FABC20, 0x009FE30E, 0x002BF035, 0x000CF0A4, 0x0005FBFF, 0x005E5E5E, 0x000D0D0D, 0x000D0D0D,
   0x00FFFFFF, 0x00A6FCFF, 0x00B3ECFF, 0x00DAABEB, 0x00FFA8F9, 0x00FFABB3, 0x00FFD2B0, 0x00FFEFA6, //3
   0x00FFF79C, 0x00D7E895, 0x00A6EDAF, 0x00A2F2DA, 0x0099FFFC, 0x00DDDDDD, 0x00111111, 0x00111111
};


