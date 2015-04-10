#include "ppu.h"

ppu::ppu() : ppuAddress(0), dotNumber(0), scanline(241), writeToggle(false), vblank(false),
		bufferVblank(false), NMI(false), VRAM(NULL), oddFrame(false), ntFetch(true),
		idleCounter(0), reloadDot(9), horizontalDot(8), pOAMAddress(0), sOAMAddress(0), spriteWrite(true), sprite_number(0),
		reg2000(0x00), reg2001(0x00), reg2002(0x2002), vblankValue(0x80)
{	
}

ppu::~ppu()
{
}

/******************************************************************************
** RUNS A PPU CYCLE
******************************************************************************/
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
			spriteEval();

			if(idleCounter > 0) idleCounter--;
			else
			{
				//Reload the registers
				if(dotNumber == reloadDot)
				{
					//Fills the upper 8 bits with next tiles
					highBGShift |= highBGFetch; 
					lowBGShift |= lowBGFetch; 
					fourToOneMux();		//Fills the attribute registers
	
					reloadDot += 8;				//Reload up to 257
				}

				//For getting the background data.
				if(scanline == 240) 
					idleCounter += 340;		//Idles for a scanline + 1 dot
				else
					backgroundFetch();		//Fetch name, att, tiles
			}

			
			//The registers only shift between dots 2-257 and 322-337 (inclusive)
			if((dotNumber > 1 && dotNumber < 258) || (dotNumber > 321 && dotNumber < 338)) shiftRegisters();

			checkDotNumber();				//Does stuff depending on the dot number
		}
	}

	dotNumber++;
	
	if(dotNumber == 341) 
	{
		scanline++;		//262 scanlines in a frame
		dotNumber = 0;
		sOAMAddress = 0;	//Reset OAM address

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


/******************************************************************************
** SETS THE PPU POINTER
******************************************************************************/
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
	{	
		ppuAddress &= ~0x41F;						//Clears the bits for horizontal position
		ppuAddress |= ppuTempAddress & 0x41F;				//Keeps the bits that were cleared above
		horizontalDot = 328;						//Next time this is needed
		reloadDot = 329;						//Next time the registers are reloaded
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
		ppuAddress &= ~0x7BE0;				//Clears the vertical bits
		ppuAddress |= ppuTempAddress & 0x7BE0;		//Puts the vertical bits in
	}
}

/******************************************************************************
** SHIFTS ALL OF THE SHIFT REGISTERS
******************************************************************************/
const void ppu::shiftRegisters()
{
	//The registers only shift between dots 2-257 and 322-337 (inclusive)
	lowBGShift <<= 1;
	highBGShift <<= 1;
	lowAttShift <<= 1;
	lowAttShift |= (highAttShift & 0x80) >> 8;	//Move the high byte into the low byte;
	highAttShift <<= 1;
}

/******************************************************************************
** RENDERS A PIXEL TO THE SCREEN
******************************************************************************/
const void ppu::renderPixel()
{	
	using namespace std;
	bool spriteActive;

	//Decrement sprite X position counters
	for(int i = 0; i < 8; i++)
	{
		spriteXPos[i]--;
		if(spriteXPos[i] == 0) 
			spriteCounter[i] = 7;
	}

	if(reg2001 & 0x18) 
	{	
      if((reg2001 & 0x08) && (reg2001 & 0x10))	//Both background and sprite enabled
		{
			int i = 0;	//Keeps track of the first sprite that is active

			//If one counter is great than 0, at least one sprite is active.
			for(;i < 8; i++) {
				if(spriteCounter[i] > 0) 
				{
					spriteActive = true;
					break;
				}
			}

			if(!spriteActive)	//Only have to worry about background pixel
			{
				palleteAddress = 0x3F00 | eightToOneMux(lowBGShift) | (eightToOneMux(highBGShift) << 1)
                                                        | (eightToOneMux(lowAttShift) << 2) | (eightToOneMux(highAttShift) << 3);
			}
			else	//A sprite is active
			{
				byte backgroundBits = 0;	//First two background bits for pallete address
				byte spriteBits = 0;		//First two sprite bits for pallete address
			
				//Get first two pallete bits to compare priority
				backgroundBits = eightToOneMux(lowBGShift) | (eightToOneMux(highBGShift) << 1);
				spriteBits = (spritesLow[i] & 1) | ((spritesHigh[i] & 1) << 1);
				
				if(backgroundBits == 0)
				{
					if(spriteBits == 0)	//Both equal 0
						palleteAddress = 0x3F00;
					else
						palleteAddress = 0x3F10 | spriteBits | ((spriteAtt[i] & 1) << 2) | ((spriteAtt[i] & 2) << 3);
				}
				else	//Backgrounds bits aren't 0
				{
					bool priority = spriteAtt[i] & 0x20;	//Grab the sprite priority bit

					if(spriteBits == 0 || priority == true)
						palleteAddress = 0x3F00 | backgroundBits | (eightToOneMux(lowAttShift) << 2) 
						| (eightToOneMux(highAttShift) << 3);
					else
						palleteAddress = 0x3F10 | spriteBits | ((spriteAtt[i] & 1) << 2) | ((spriteAtt[i] & 2) << 3);
				}
			}

			//Shift all registers of the active sprite
			spritesLow[i] >>= 1;
			spritesHigh[i] >>= 1;
			//spritesAtt[i] >>= 1;
		}
		else if(reg2001 & 0x08)	//Only background enabled
		{	
			palleteAddress = 0x3F00 | eightToOneMux(lowBGShift) | (eightToOneMux(highBGShift) << 1)
									| (eightToOneMux(lowAttShift) << 2) | (eightToOneMux(highAttShift) << 3);
		}
	//else if(reg2001 & 0x10)	//Only sprites enabled.
	}
	else	//No rendering enabled
  	{	
		if(ppuAddress >= 0x3F00) palleteAddress = ppuAddress;
		else palleteAddress = 0x3F00;
  	}

	//Always do this.  Renders
	palleteData = VRAM->readVRAM(palleteAddress);
   screenData[scanline][dotNumber] = RGB[palleteData];	//RGB data
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


const void ppu::spriteEval()
{
	static word spriteLowLoad;
	static word spriteHighLoad;
	static byte readCounter;
	static byte sprite_state;
	static byte OAM_data;

	if(dotNumber < 64)	//Clear secondary OAM
	{
		VRAM->write_secondary_OAM(sOAMAddress,0xFF);
		sOAMAddress = (sOAMAddress + 1) & 31;	//Cutoff at 32 byte mark
	}
	else if(dotNumber < 257)
	{
		if(dotNumber == 64) 
		{
			pOAMAddress = VRAM->readRAM(0x2003);	//Get the OAMAddress
			sOAMAddress = 0;
			spriteLowLoad = 261;
			spriteHighLoad = 263;
			readCounter = 0;
			spriteWrite = true;		//Allow writing to secondary OAM again
			sprite_state = 0;
		}
		
		switch(sprite_state) {
			case 0: //See if Y-cood is in range
				if(dotNumber & 1) {
					OAM_data = VRAM->read_primary_OAM(pOAMAddress);
					if(scanline <= OAM_data && OAM_data < (scanline + 8)) {
						if(spriteWrite) VRAM->write_secondary_OAM(sOAMAddress++, OAM_data);
						sprite_number++;
						sprite_state = 1;
						}
					}
			case 1: //Copy remaining bits
				if(dotNumber & 1) {
					if(readCounter++ == 3) {
					}
				}
				break;
		}
	}
	else if(dotNumber < 321)	//Fetch stuff
	{
		if(dotNumber == 257) sOAMAddress = 1;	//Reset the address to first address

		if(dotNumber == spriteLowLoad)
		{
			//Find the tile it's in
			spriteAddress = (reg2002 & 0x8) | (VRAM->read_secondary_OAM(sOAMAddress) * 0x10);
			//TODO: FIX
			//spritesLow[spriteNum] = VRAM->readVRAM(spriteAddress);
			//spriteAtt[spriteNum] = VRAM->read_secondary_OAM(sOAMAddress + 2);
			//spriteXPos[spriteNum] = VRAM->read_secondary_OAM(sOAMAddress + 3);
			spriteLowLoad += 4;		//Wait 4 cycles to do this again
		}
		else if(dotNumber == spriteHighLoad)
		{
			spriteAddress += 8;
			//spritesHigh[spriteNum++] = VRAM->readVRAM(spriteAddress);
			sOAMAddress += 4;
			spriteHighLoad += 4;
		}
	}
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


//------------------------------------------------------------------------------------------------------------------------------------------
//Muxes
const void ppu::fourToOneMux()				//Used to refill attribute shift registers
{
	//Select first bit of coarse X and Y for MUX select
	const byte coarseX = 0x1;
	const byte coarseY = 0x20;
	bool xBit, yBit;				//Holds the coarse X and Y bits
	bool attBit1, attBit2;				//Holds the two attrbute bits. 1 = low bit, 2 = high bit
	xBit = ppuAddress & coarseX;
	yBit = ppuAddress & coarseY;
	
	
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
	if(attBit2) highAttShift |= 0xFF;		//Low byte 1s
	else highAttShift &= ~0xFF;			//Low byte 0s
	
	if(attBit1) lowAttShift |= 0xFF;		//Low byte 1s
	else lowAttShift &= ~0xFF;			//Low byte 0s
}


const bool ppu::eightToOneMux(unsigned short &data)		//16 bit numbers
{
	bool retval;
		
	if(fineXScroll == 0) retval = data & 0x8000;
	else if(fineXScroll == 1) retval = data & 0x4000;
	else if(fineXScroll == 2) retval = data & 0x2000;
	else if(fineXScroll == 3) retval = data & 0x1000;
	else if(fineXScroll == 4) retval = data & 0x0800;
	else if(fineXScroll == 5) retval = data & 0x0400;
	else if(fineXScroll == 6) retval = data & 0x0200;
	else retval = data & 0x0100;

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


