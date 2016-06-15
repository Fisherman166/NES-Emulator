#include "ppu.h"

ppu::ppu() : ppuAddress(0), dotNumber(0), scanline(241), writeToggle(false), 
		vblank(false), NMI(false), VRAM(NULL), oddFrame(false),
		pOAMAddress(0), sOAMAddress(0), sprite_number(0), spriteWrite(true), 
		reg2000(0x00), reg2001(0x00), reg2002(0x2002)
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
	reg2000 = VRAM->RAM[0x2000];
	reg2001 = VRAM->RAM[0x2001];

    byte rendering_enabled = reg2001 & 0x18;
    byte pre_render_line = (scanline == 261);
    byte visable_line = (scanline < 240);
    byte render_line = pre_render_line || visable_line;
    byte fetch_next_screen = (dotNumber >= 321) && (dotNumber <= 336);
    byte visable_cycle = (dotNumber >= 1) && (dotNumber <= 256);
    byte fetch_cycle = (fetch_next_screen || visable_cycle);
    byte shift_reload_dots = (dotNumber >= 9) && (dotNumber <= 257);

    if(rendering_enabled)				//Checks if rendering is enabled
    {	
        if(visable_line && visable_cycle) renderPixel();

        spriteEval();

        if(render_line && shift_reload_dots && ((dotNumber % 8) == 1)) reload_registers();

        if(render_line && visable_cycle) {
            switch( dotNumber % 8 ) {
                case 1:
                    nametable_fetch();
                    break;
                case 3:
                    attribute_fetch();
                    break;
                case 5:
                    low_background_fetch();
                    break;
                case 7:
                    high_background_fetch();
                    break;
            }
        }
        
        //The registers only shift between dots 2-257 and 322-337 (inclusive)
        if((dotNumber > 1 && dotNumber < 258) || (dotNumber > 321 && dotNumber < 338)) shiftRegisters();

        if(render_line) {
            if(dotNumber == 256) incrementY();
            if(fetch_cycle && ((dotNumber % 8) == 0)) incrementX();
            if(dotNumber == 257) copyX();
            if(pre_render_line && (dotNumber >= 280) && (dotNumber <= 304)) copyY();
        }
    }

    tick(rendering_enabled);
    
	checkVblank();
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
const void ppu::tick(ppu::byte& rendering_enabled) {
    if(rendering_enabled) {
        if(oddFrame && scanline == 261 && dotNumber == 339) {
            scanline = 0;
            dotNumber = 0;
            oddFrame ^= 1;
            return;
        }
    }       
    dotNumber++;
    if(dotNumber > 340) {
        dotNumber = 0;
        scanline++;

        if(scanline > 261) {
            scanline = 0;
            oddFrame ^= 1;
        }
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
	highAttShift <<= 1;
}

const void ppu::reload_registers() {
    highBGShift |= high_tile_byte; 
    lowBGShift |= low_tile_byte; 
    fourToOneMux();		//Fills the attribute registers 
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
    byte vblank_value = 0x80;

	if(scanline == 241 && dotNumber == 1) 
	{
		VRAM->writeRAM(reg2002, vblank_value);
		vblank = true;
		if(reg2000 & 0x80) NMI = true;				//NMI bit set
	}
	else if(scanline == 261 && dotNumber == 1)
	{
		unsigned char clearData = 0x9F;				//Value that clears the flags
		zeroFlag = false;
		spriteOverflow = false;
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
const void ppu::incrementY() {
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

const void ppu::incrementX() {
    if((ppuAddress & 0x001F) == 31) 	// if coarse X == 31
    {
        ppuAddress &= ~0x001F;         	// coarse X = 0
        ppuAddress ^= 0x0400;           // switch horizontal nametable
    }
    else ppuAddress++;                	// increment coarse X
}

const void ppu::copyX() {
   	ppuAddress &= ~0x41F;						//Clears the bits for horizontal position
	ppuAddress |= ppuTempAddress & 0x41F;				//Keeps the bits that were cleared above
 }

 const void ppu::copyY() {
    ppuAddress &= ~0x7BE0;				//Clears the vertical bits
	ppuAddress |= ppuTempAddress & 0x7BE0;		//Puts the vertical bits in
}

//----------------------------------------------------------------------------------------------------------------------------------------
//Scanline functions	
const void ppu::nametable_fetch() {
    word nameAddress;

    nameAddress = 0x2000 | (ppuAddress & 0x0FFF);
    nametable_byte = VRAM->readVRAM(nameAddress);
}

const void ppu::attribute_fetch() {
    word attAddress = 0x23C0 | (ppuAddress & 0x0C00) | ((ppuAddress >> 4) & 0x38) | ((ppuAddress >> 2) & 0x07);
    attribute_byte = VRAM->readVRAM(attAddress);
}

const void ppu::low_background_fetch() {
    word tileAddress;
    
    tileAddress = calc_tile_address();
    low_tile_byte = VRAM->readVRAM(tileAddress);
}

const void ppu::high_background_fetch() {
    const byte high_tile_offset = 8;
    word tileAddress;

    tileAddress = calc_tile_address();
    tileAddress += high_tile_offset;
    high_tile_byte = VRAM->readVRAM(tileAddress);
}

const ppu::word ppu::calc_tile_address() {
    word tileAddress; 

    if(reg2000 & 0x10) tileAddress = 0x1000 | (nametable_byte << 4) | ((ppuAddress & 0x7000) >> 12);
    else tileAddress = 0x0000 | (nametable_byte << 4) | ((ppuAddress & 0x7000) >> 12);

    return tileAddress;
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
		attBit1 = attribute_byte & 0x40;
		attBit2 = attribute_byte & 0x80;
	}
	else if(xBit)					//Choose bits 4 and 5
	{
		attBit1 = attribute_byte & 0x10;
		attBit2 = attribute_byte & 0x20;
	}
	else if(yBit)					//Choose bits 2 and 3
	{
		attBit1 = attribute_byte & 0x04;
		attBit2 = attribute_byte & 0x08;
	}
	else						//Choose bits 0 and 1
	{
		attBit1 = attribute_byte & 0x01;
		attBit2 = attribute_byte & 0x02;
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


