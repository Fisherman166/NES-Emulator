#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "memory.h"
#include "ppu.h"

using namespace std;

class cpu
{
public:
	cpu();
	~cpu();
	unsigned short PC, SP;	//Stack pointer can be between 0x0100 and 0x01FF in memory
	unsigned char A, X, Y;

	/* C = Carry flag Z = Zero flag  I = Interrupt disable D = decimal mode
	V = Overflow flag N = Negative flag*/
	bool C, Z, I, D, V, N;

	unsigned short emulateCycle(memory*, ppu*);
	void setPCStart(memory*);					//Sets the PC to the reset vector
	
private:
	//ofstream debugFile;					//Used for outputting debug information
	unsigned char initialA; //initialX, initialY, initialP;	//Used for debugging
	//unsigned short initialSP;				//Used for debugging
	unsigned short cycles; //initialCycles;			//Holds the current number of cycles


	//Functions
	const void pushStack(memory*, unsigned char&, ppu*);
	const unsigned char popStack(memory*, ppu*);
	const void compareFlags(unsigned char&, unsigned char);
	const unsigned char encodeBits(bool bools[8]);
	const void decodeBits(bool (&bools)[8], unsigned char input);
	const void pageBoundry(unsigned short, unsigned short&, unsigned char);
	const void pageBranch(char&);

	const void NMI(memory*, ppu*);

	//Debug functions
	//DataPC = data from the PC counter memory address.  dataAddress = data from the PC counters addresses given
	//in dataPC.
	/*const void debugAcc(unsigned char &opcode, string OPname);			//Accumulator
	
	const void debugImm(unsigned char &opcode, unsigned char data, string OPname); //Immidiate

	const void debugZero(unsigned char &opcode,  unsigned char dataPC, 
			     unsigned char dataAddress, string OPname); 		//Zeropage

	const void debugZeroX(unsigned char &opcode, unsigned char dataPC,
			unsigned short totalAddress, unsigned char dataAddress, string Opname); //Zeropage, X

	const void debugZeroY(unsigned char &opcode, unsigned char dataPC,
			unsigned short totalAddress, unsigned char dataAddress, string Opname); //Zeropage, Y

	const void debugAbs(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
				unsigned char data, string OPname);	   //Absolute

	const void debugAbsX(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
			unsigned short totalAddress, unsigned char data, string OPname);	//Absolute,X

	const void debugAbsY(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
			unsigned short totalAddress, unsigned char data, string OPname);	//Absolute,Y

	const void debugIndirectX(unsigned char &opcode, unsigned char pcAddress, unsigned short indirectAddress, 					unsigned char data, string OPname);	//Indirect, X

	const void debugIndirectY(unsigned char &opcode, unsigned char pcAddress,
			unsigned short indirectAddress, unsigned char data, string OPname);	//Indirect, Y

	const void debugRelative(unsigned char &opcode, char offset, unsigned char dataPC, string OPname);
	
	const void debugImplied(unsigned char &opcode, string OPname);

	const void debugIndirect(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress,
				unsigned short jumpAddress, string OPname);*/
};

#endif
